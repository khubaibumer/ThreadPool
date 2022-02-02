#include "../include/ThreadPool.h"
#include <syscall.h>
#include <unistd.h>

#define THREAD_CAST(x) ((thread_info_t*)(x))
#define GET_THREAD(index) &pool->ctrl.threads[index]
#define BUG_ON(x) ({ \
    if (x) { \
        fprintf(pool->logfile, "Assertion Failed " #x "\n"); \
        return false;\
    } \
})

typedef struct job_data {
    user_job_t job;
    void *arg;
} job_data_t;

typedef struct thread_info {
    bool in_use;
    bool is_working;
    pthread_t handle;
    tid_t tid;
    int task_count;
    size_t total_jobs;
    sem_t sem;
    void *queue;
    pthread_spinlock_t lock_;
} thread_info_t;

typedef struct thread_pool {
    struct {
        size_t total;
        size_t available;
        size_t busy;
    } count;
    struct {
        thread_info_t *threads;
        pthread_spinlock_t lock;
    } ctrl;
    FILE *logfile;
} thread_pool_t;

static thread_pool_t *pool = NULL;

/* Forward Declaration */
int find_suitable_thread();
bool add_job(user_job_t, void*);
static void* event_loop(void *args);
bool initialize_thread_pool(size_t pool_size);
static bool tp_set_queue_funcs(queue_new_t new, queue_init_t init, queue_push_back_t push_back, queue_pop_head_t pop_head);

__always_inline static void tp_lock() { pthread_spin_lock(&pool->ctrl.lock); }
__always_inline static void tp_unlock() { pthread_spin_unlock(&pool->ctrl.lock); }
__always_inline static size_t tp_get_total() { return pool->count.total; }
__always_inline static size_t tp_get_available() { return pool->count.available; }
__always_inline static size_t tp_get_busy() { return pool->count.busy; }
__always_inline static void th_lock(void *thread) { pthread_spin_lock(&THREAD_CAST(thread)->lock_); }
__always_inline static void th_unlock(void *thread) { pthread_spin_unlock(&THREAD_CAST(thread)->lock_); }
__always_inline static void th_post_job(void *thread) { sem_post(&THREAD_CAST(thread)->sem); ++THREAD_CAST(thread)->task_count; }
__always_inline static void th_wait_on_job(void *thread) { sem_wait(&THREAD_CAST(thread)->sem); --THREAD_CAST(thread)->task_count; }
__always_inline static tid_t th_get_tid(void *thread) { return THREAD_CAST(thread)->tid; }
__always_inline static size_t th_get_total_jobs(void *thread) { return THREAD_CAST(thread)->total_jobs; }
__always_inline static void tp_destroy() {
    for (int i = 0; i < pool->count.total; i++) {
        thread_info_t *thread  = GET_THREAD(i);
        thread->is_working = false;
        ThreadPool->thread.post_job(thread);
        fprintf(pool->logfile, "Destroying Thread TID: %ld Total Jobs taken: %ld\n", thread->tid, thread->total_jobs);
        pthread_cancel(thread->handle);
    }
}

static thread_pool_funcs_t pool_funcs = {
        .lock = tp_lock,
        .unlock = tp_unlock,
        .post = add_job,
        .set_logfile = set_logger,
        .count.get_available = tp_get_available,
        .count.get_busy = tp_get_busy,
        .count.get_total = tp_get_total,
        .get_tid = th_get_tid,
        .pool.lock = tp_lock,
        .pool.unlock = tp_unlock,
        .thread.lock = th_lock,
        .thread.unlock = th_unlock,
        .thread.post_job = th_post_job,
        .thread.wait_job = th_wait_on_job,
        .thread.jobs_count = th_get_total_jobs,
        .init = initialize_thread_pool,
        .destroy = tp_destroy,
        .set_queue_functions = tp_set_queue_funcs
};
thread_pool_funcs_t *getThreadPoolInternal() { return &pool_funcs; }

__always_inline static bool tp_set_queue_funcs(queue_new_t new, queue_init_t init, queue_push_back_t push_back, queue_pop_head_t pop_head) {
    if (pool != NULL || ThreadPool->queue.are_funcs_set == true) {
        perror("ThreadPool is already initialized\n");
        return false;
    }
    pool_funcs.queue.new = new;
    pool_funcs.queue.init = init;
    pool_funcs.queue.push_back = push_back;
    pool_funcs.queue.pop_head = pop_head;
    return true;
}

__always_inline static void tp_set_default_queue_funcs() {
    pool_funcs.queue.new = (void *(*)(void)) g_queue_new;
    pool_funcs.queue.init = (void (*)(void *)) g_queue_init;
    pool_funcs.queue.push_back = (void (*)(void *, void *)) g_queue_push_tail;
    pool_funcs.queue.pop_head = (void *(*)(void *)) g_queue_pop_head;
    ThreadPool->queue.are_funcs_set = true;
}

bool add_job(user_job_t job, void *arg) {
    job_data_t *data = calloc(1, sizeof(job_data_t));
    BUG_ON(data == NULL);
    data->job = job;
    data->arg = arg;
    int index = find_suitable_thread();
    BUG_ON(index == -1);
    thread_info_t *thread = GET_THREAD(index);
    BUG_ON(thread == NULL);
    ThreadPool->lock();
    ThreadPool->queue.push_back(thread->queue, data);
    ThreadPool->thread.post_job(thread);
    ThreadPool->unlock();
    fprintf(pool->logfile, "New Job Posted to the ThreadPool. Requesting Tid: %ld\n", syscall(SYS_gettid));
    return true;
}

bool initialize_thread_pool(size_t pool_size) {
    if (ThreadPool->queue.are_funcs_set == false) {
        tp_set_default_queue_funcs();
    }
    pool = calloc(1, sizeof(thread_pool_t));
    pool->count.total = pool_size;
    pool->count.available = pool_size;
    pool->ctrl.threads = calloc(pool_size, sizeof(thread_info_t));
    BUG_ON(pool->ctrl.threads == NULL);
    pthread_spin_init(&pool->ctrl.lock, 0);
    for (int i = 0; i < pool->count.total; i++) {
        pool->ctrl.threads[i].queue = g_queue_new();
        pool->ctrl.threads[i].queue = ThreadPool->queue.new();
        BUG_ON(pool->ctrl.threads[i].queue == NULL);
        BUG_ON(sem_init(&pool->ctrl.threads[i].sem, 0, 0) != 0);
        ThreadPool->queue.init(pool->ctrl.threads[i].queue);
        BUG_ON(pthread_create(&pool->ctrl.threads[i].handle, NULL, event_loop, NULL) != 0);
    }
    return true;
}

int find_suitable_thread() {
    int index = 0;
    ThreadPool->lock();
    int min = pool->ctrl.threads[index].task_count;
    if (min != 0) {
        for (int i = 1; i < pool->count.total; ++i) {
            if (min > pool->ctrl.threads[i].task_count) {
                index = i;
                min = pool->ctrl.threads[i].task_count;
            }
        }
    }
    ++pool->ctrl.threads[index].task_count;
    ++pool->ctrl.threads[index].total_jobs;
    ThreadPool->unlock();
    return index;
}

void set_logger(FILE *logfile) {
    if (logfile == NULL) {
        pool->logfile = stdout;
    }
    pool->logfile = logfile;
}

static void* event_loop(void *args) {
    ThreadPool->lock();
    int index = 0;
    for (int i = 0; i < pool->count.total; i++) {
        if (pool->ctrl.threads[i].in_use == false) {
            index = i;
        }
    }
    thread_info_t *thread = GET_THREAD(index);
    thread->tid = syscall(SYS_gettid);
    thread->in_use = true;
    thread->is_working = true;
    ThreadPool->unlock();
    while(thread->is_working == true) {
        ThreadPool->thread.wait_job(thread);
        ThreadPool->thread.lock(thread);
        job_data_t *data = ThreadPool->queue.pop_head(thread->queue);
        ThreadPool->thread.unlock(thread);
        data->job(data->arg);
        free(data);
    }
    return NULL;
}
