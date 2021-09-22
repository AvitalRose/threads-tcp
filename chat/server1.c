//
// Created by avitalrose on 9/19/21.
//


/*
 * layout idea:
 *      in main:
 *      1.refrence signal
 *      2. set up socket:
 *          a. socket
 *          b. bind
 *          c. listen
 *          d. set up thread function for accepting connections
 *          e.set up message function for handling messages
 *      3.Accepting connections function
 *          a. accept connection and increment
 *          b. pass to thread function the fd of new connection
 *      4. Accepting messages function-producer
 *          a. add valid messages to queue
 *      5. Broadcasting messages function- consumer
 *          a. if there are messages in queue, remove them and send to all others
 *      in signal function: if process killed- first end nicely
 *      in thread function
 *
 */

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

#define MAX_CONNECTIONS 20

typedef struct{
    int listenfd;
} socketVariables;

struct os_queue connection_queue;
pthread_mutex_t connection_mutex;

void *handle_connection(void* p_connfd);
void *new_connection_handler(void* mainSocketDate);

int main(){

    int listenfd;
    struct sockaddr_in serv_addr;

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


    //Thread for accepting new threads;
    pthread_t new_connection;
    int *p_listenfd = malloc(sizeof(int));
    *p_listenfd = listenfd;
    if((pthread_create(&new_connection, NULL, new_connection_handgiler, p_listenfd)) != 0){
        perror("new connection handler thread creation failed: ");
        return 1;
    }
    if((pthread_join(new_connection, NULL)) != 0){
        perror("new connection handler join fail: ");
        return 1;
    }
    printf("after join\n");
//    sleep(2);




//    while(1){
//        len = sizeof(struct sockaddr_in);
//        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t *) &len)) < 0) {
//            perror("Accept failed: ");
//        } else {
//            chat_size++;
//        }
//
//        inet_ntop(AF_INET, &(remote_addr.sin_addr), addressBuff, len);    //copy the address to string
//        printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));
//
//        int *pclient = malloc(sizeof(int));
//        *pclient = connfd;
//        pthread_t t;
//        if((pthread_create(&t, NULL, handle_connection, pclient)) != 0 ){
//            perror("thread creation fail: ");
//            return 1;
//        }
//
//    }
}

void *new_connection_handler(void* p_listenfd){
    printf("enterd fucntion\n");
//    printf("%d\n", &p_listenfd);
    int listenfd = *((int*)p_listenfd);
    free(p_listenfd);
    struct sockaddr_in remote_addr;
    int chat_size = 0, connfd, len;
    char addressBuff[300];

    while(1){
        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t *) &len)) < 0) {
            perror("Accept failed: ");
        } else {
            chat_size++;
        }
        inet_ntop(AF_INET, &(remote_addr.sin_addr), addressBuff, len);    //copy the address to string
        printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));

        int *pclient = malloc(sizeof(int));
        *pclient = connfd;
        pthread_t t;
        if((pthread_create(&t, NULL, handle_connection, pclient)) != 0 ){
            perror("new connection thread creation fail: ");
            return NULL;
        }
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