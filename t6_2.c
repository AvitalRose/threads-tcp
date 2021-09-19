//
// Created by avitalrose on 8/11/21.
//

//To compile: gcc -l pthread T6_2.c
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

int int_Val[5] = {0,0,0,0,0};
pthread_t th[5];

void printids(const char *s)
{
    pid_t      pid;
    pthread_t  tid;

    pid = getpid();
    tid = pthread_self();
    printf("%s pid %u tid %u\n", s, (unsigned int)pid,(unsigned int)tid);
}

void *add_to_value(void *arg) { // arg = 0, 1, 2, 3 ,4
    int inData = *((int*)arg);
    int i;
    for(i = 0; i < 10000; i++){
        int_Val[i % 5] += inData; /* ? */
    }
    printids("new thread");


    /* Expectation (under the incorrect assumption that threads will run one after another for sure):
      thread 0: [0,0,0,0,0] (+0 to each cell)
      thread 1: [2000,2000,2000,2000,2000] (+1*2000=2000 to each cell)
      thread 2: [6000,6000,6000,6000,6000] (+2*2000=4000 to each cell)
      thread 3: [12000,12000,12000,12000,12000] (+3*2000=6000 to each cell)
      thread 4: [20000,20000,20000,20000,20000] (+4*2000=8000 to each cell)
    */

    return (NULL);
}

int main(void) {
    int i, retcode;

    /* Create the threads */
    for(i = 0; i < 5; i++) {
        retcode = pthread_create(&th[i], NULL, add_to_value, (void *)&i);
        if (retcode != 0)
            printf("Create thread failed with error %d\n", retcode);
    }

    /* Wait until all threads have finished */
    void* retVal[5];
    for(i = 0; i < 5; i++){
        pthread_join(th[i], &retVal[i]);
    }

    /* Print the results */
    printf("Final values:\n");
    for(i = 0; i < 5; i++)
        printf("Integer value[%d] = \t%d\n", i, int_Val[i]);
    return 0;
}
