#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <glib-2.0/glib.h>

#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif

typedef long tid_t;
typedef void* (*user_job_t) (void*);
typedef void* (*queue_new_t) (void);
typedef void (*queue_init_t) (void *head);
typedef void (*queue_push_back_t) (void *head, void *data);
typedef void* (*queue_pop_head_t) (void *head);

typedef struct thread_pool_funcs {
    struct {
        size_t (*get_total)(void);
        size_t (*get_available)(void);
        size_t (*get_busy)(void);
    } count;
    struct {
        void (*lock)(void);
        void (*unlock)(void);
    } pool;
    struct {
        void (*lock) (void*);
        void (*unlock) (void*);
        void (*post_job) (void*);
        void (*wait_job) (void*);
        size_t (*jobs_count) (void*);
    } thread;
    tid_t (*get_tid) (void*);
    void (*set_logfile) (FILE*);
    bool (*post) (user_job_t job, void *arg);
    void (*lock) (void);
    void (*unlock) (void);
    bool (*init) (size_t thread_count);
    void (*destroy) (void);
    bool (*set_queue_functions) (queue_new_t new, queue_init_t init, queue_push_back_t push_back, queue_pop_head_t pop_head);
    struct {
        bool are_funcs_set;
        void *(*new)(void);
        void (*init)(void *);
        void (*push_back)(void *head, void *data);
        void *(*pop_head) (void *head);
    } queue;
} thread_pool_funcs_t;

void set_logger(FILE *logfile);

thread_pool_funcs_t* getThreadPoolInternal();

__always_inline static thread_pool_funcs_t* getThreadPool() {
    return getThreadPoolInternal();
}

#define ThreadPool getThreadPool()

#endif //THREADPOOL_H
