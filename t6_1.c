//
// Created by avitalrose on 8/11/21.
//

//To compile: gcc -l pthread T6_1.c
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
pthread_t ntid;

void printids(const char *s)
{
    pid_t      pid;
    pthread_t  tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %u tid %u\n", s, (unsigned int)pid,(unsigned int)tid);
}

void* thr_fn(void *arg)
{
    printids("new thread: ");
    return((void *)0);
}

int main(void)
{
    int     err;
    err = pthread_create(&ntid, NULL, thr_fn, NULL);
    if (err != 0)
        perror("can't create thread\n");
    printids("main thread:");
    sleep(1);
    exit(0);
}
