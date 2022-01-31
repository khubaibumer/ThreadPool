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
} thread_pool_funcs_t;

void set_logger(FILE *logfile);

thread_pool_funcs_t* getThreadPoolInternal();

__always_inline static thread_pool_funcs_t* getThreadPool() {
    return getThreadPoolInternal();
}

#define ThreadPool getThreadPool()

#endif //THREADPOOL_H
