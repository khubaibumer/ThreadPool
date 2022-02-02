#include <stdio.h>
#include <errno.h>
#include <syscall.h>
#include <unistd.h>
#include "../include/ThreadPool.h"

void* func1(void* arg) {
    printf("Routine Called! Tid: %ld\n", syscall(SYS_gettid));
    printf("arg: %s\n", (char*)arg);
}

int main() {

    if(ThreadPool->set_queue_functions((queue_new_t) g_queue_new, (queue_init_t) g_queue_init,
                                       (queue_push_back_t) g_queue_push_tail, (queue_pop_head) g_queue_pop_head) == false) {
        perror("Unable to set queue functions\n");
        exit(0);
    }

    if (ThreadPool->init(10) == false) {
        perror("Unable to initialize ThreadPool");
        return errno;
    }
    ThreadPool->set_logfile(stdout);
    for (int i  = 0; i < 10000; i++) {
        ThreadPool->post(func1, "Hello from the other side");
    }

    ThreadPool->destroy();

    printf("Hello, World!\n");
    return 0;
}
