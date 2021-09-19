//
// Created by avitalrose on 9/18/21.
//
// TCP - server

#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "osqueue.h"

#define THREAD_POOL_SIZE 20


//pthread_t th[THREAD_POOL_SIZE];
//OSQueue* thread_q;


void *handle_connection(void *p_connfd);
void *thread_func(void *args);

int main(int argc, char *argv[]) {

    int listenfd, connfd, len;
    struct sockaddr_in serv_addr, remote_addr;
    char addressBuff[300];

    //create threadpool
//    int i;
//    for(i=0; i<THREAD_POOL_SIZE; i++){
//        if((pthread_create(&th[i], NULL, thread_func, NULL)) != 0 ){
//            perror("thread creation fail: ");
//            return 1;
//        }
//    }



    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("Socket opening error: ");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(15470);

    if((bind(listenfd, (struct sockaddr*)&serv_addr, (socklen_t)sizeof(struct sockaddr_in))) < 0) {
        perror("Socket bind error: ");
        return 1;
    }

    if((listen(listenfd, 10)) < 0){
        perror("Listen error: ");
        return 1;
    }

    while(1){
        len = sizeof(struct sockaddr_in);
        if((connfd = accept(listenfd, (struct sockaddr*)&remote_addr, (socklen_t*)&len)) < 0){
            perror("Accept failed: ");
        }
        inet_ntop(AF_INET,&(remote_addr.sin_addr),addressBuff,len);	//copy the address to string
        printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));

        int *pclient = malloc(sizeof(int));
        *pclient = connfd;
        pthread_t t;
        pthread_create(&t, NULL, handle_connection, pclient);

    }
}




void *handle_connection(void* p_connfd){
    int connfd = *((int*)p_connfd);
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


