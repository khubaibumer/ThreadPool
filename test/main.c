#include <stdio.h>
#include <errno.h>
#include <syscall.h>
#include <unistd.h>
#include "../include/ThreadPool.h"

void* func1(void* arg) {
    printf("Routine Called! Tid: %ld\n", syscall(SYS_gettid));
    printf("arg: %s\n", (char*)arg);
    usleep(500);
}

int main() {

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
