//
// Created by avitalrose on 8/11/21.
//

//The basic_server checks for mistakes on every different call
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include "osqueue.h"

#define THREAD_POOL_SIZE 20

pthread_t th[THREAD_POOL_SIZE];
OSQueue* thread_q;


void * thread_func(void * p);
void * new_connection(void * p);

void * thread_func(void  * p) {
    while(1){
        if(osIsQueueEmpty(thread_q) != 1) {
            int *p_connfd = osDequeue(thread_q);
            new_connection(p_connfd);
        }
    }
}

void * new_connection(void * p_connfd)
{
    int connfd = *(int*)p_connfd;
    free(p_connfd);
    char sendBuff[1025];
    time_t ticks;
    ticks = time(NULL);
    snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
    sleep(2);

    write(connfd, sendBuff, strlen(sendBuff));
    close(connfd);
    sleep(1);
}

int main(int argc, char *argv[])
{
    //first creating a bunch of threads for later use
    int i, retcode;
    for(i=0; i<THREAD_POOL_SIZE; i++){
        retcode = pthread_create(&th[i], NULL, thread_func, NULL);
        if(retcode != 0){
            printf("Create thread failed with error %d\n", retcode);
        }
    }

    //creating the queue
    thread_q = osCreateQueue();

    //server tcp
    int listenfd = 0, connfd = 0,len;
    struct sockaddr_in serv_addr,remote_addr;
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {// AF_INET - IPv4 Internet protocols
        perror("socket opening failed: ");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(15470);

    if(bind(listenfd, (struct sockaddr*)&serv_addr, (socklen_t)sizeof(struct sockaddr_in)) == -1){
        perror("bind failed: ");
    }

    if(listen(listenfd, 10) == -1){
        perror("listen failed");
    }

    while(1)
    {
        char addressBuff[300];
        len=sizeof(struct sockaddr_in);
        connfd = accept(listenfd, (struct sockaddr*)&remote_addr, (socklen_t*)&len);
        inet_ntop(AF_INET,&(remote_addr.sin_addr),addressBuff,len);	//copy the address to string
        printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));

        int retcode;
        int *p_connfd = malloc(sizeof(int ));
        *p_connfd = connfd;
        osEnqueue(thread_q, p_connfd);


    }

}