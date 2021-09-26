//
// Created by avitalrose on 9/26/21.
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
 *          e. set up function to handle connections with specific client
 *          f. set up message function for handling messages
 *      3.Accepting connections function
 *          a. accept connection and increment chat size. conectionfd goes into array.
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
#define recvBuffSize 1024

typedef struct{
    int listenfd;
    int client_id;
} socketVariables;

OSQueue *connection_queue;
OSQueue *message_queue;
pthread_mutex_t connection_mutex;
pthread_mutex_t message_mutex;

socketVariables *clientIdArr[MAX_CONNECTIONS];
int chat_size;


void *handle_connection(void* p_connfd);
void *new_connection_handler(void* mainSocketDate);
void print_queue(OSQueue *queue);
void add_to_queue(socketVariables *clientId);
void print_arr_queue(void *data);
void *broadcast_message(void *data);
void remove_from_queue(void *data);



int main(){

    int listenfd;
    struct sockaddr_in serv_addr;

    //Creating message queue mutex
    pthread_mutex_init(&message_mutex, NULL);
    message_queue = osCreateQueue();

    //Setting up main socket
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
    if((pthread_create(&new_connection, NULL, new_connection_handler, p_listenfd)) != 0){
        perror("new connection handler thread creation failed: ");
        return 1;
    }
    if((pthread_join(new_connection, NULL)) != 0){
        perror("new connection handler join fail: ");
        return 1;
    }
    printf("after join\n");
}

void *new_connection_handler(void* p_listenfd){
    int listenfd = *((int*)p_listenfd);
    free(p_listenfd);
    struct sockaddr_in remote_addr;
    int  connfd, len, client_id = 1;
    chat_size = 1;
    char addressBuff[300];
    char max_message[1024];
    strcpy(max_message, "Server reached max connections, try later");
    connection_queue = osCreateQueue();
    pthread_mutex_init(&connection_mutex, NULL);

    while(1){
        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t *) &len)) < 0) {
            perror("Accept failed: ");
        } else {
            if(chat_size <= MAX_CONNECTIONS) {
                socketVariables *client = (socketVariables *) malloc(sizeof(socketVariables));
                client->listenfd = connfd;
                client->client_id = client_id++;
                add_to_queue(client);
                print_arr_queue(client);
                chat_size++;
                printf("chat size is %d\n", chat_size);
                inet_ntop(AF_INET, &(remote_addr.sin_addr), addressBuff, len);    //copy the address to string
                printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));
                int *pclient = malloc(sizeof(int));
                *pclient = connfd;
                pthread_t t;
                if ((pthread_create(&t, NULL, handle_connection, pclient)) != 0) {
                    perror("new connection thread creation fail: ");
                    return NULL;
                }
            } else {
                printf("Server reached max connections limit\n");
                fflush(stdout);
                if(send(connfd, max_message, 1024, 0) < 0) {
                    perror("Send to client that connections maxed failed: ");
                }
                if(close(connfd) != 0){
                    perror("closing extra connfd failed: ");
                }
            }

        }
    }


}

void *handle_connection(void *p_connfd){
    int connfd = *((int*)p_connfd);
    free(p_connfd);
    char recvBuff[recvBuffSize];
    char exitMessage[1024];
    strcpy(exitMessage, "Exited chat");
    while (1) {
        int read, err;
        if ((read = recv(connfd, recvBuff, recvBuffSize, 0)) < 0) {
            perror("Read from client failed: ");
        }
        printf("strlen of recvBuff is %ld\n", strlen(recvBuff));
        if(strcmp(recvBuff, "exit") == 0){
            remove_from_queue(&connfd);
        }
        broadcast_message(recvBuff);
        if (!read) {//done reading
            break;
        }
        if ((err = send(connfd, recvBuff, read, 0)) < 0) {
            perror("Send back to client failed: ");
        }
    }
}

void print_queue(OSQueue *queue){
    printf("now printing queue\n");
    int *p1;
    OSQueue *tmp_queue = osCreateQueue();
    while(osIsQueueEmpty(queue) != 1){
        p1 = osDequeue(queue);
        osEnqueue(tmp_queue, p1);
        printf("connection fd: %d\n", *p1);
    }
    int *p2;
    while(osIsQueueEmpty(tmp_queue) != 1){
        p2 = osDequeue(tmp_queue);
        osEnqueue(queue, p2);
    }

}

void add_to_queue(socketVariables *clientId){
    pthread_mutex_lock(&connection_mutex);
    for(int i = 0; i<MAX_CONNECTIONS; i++){
        if(!clientIdArr[i]){
            clientIdArr[i] = clientId;
            break;
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}

void print_arr_queue(void *data){
    for(int i = 0; i < MAX_CONNECTIONS; i++){
        if(clientIdArr[i]) {//only print connections that exist
            printf("connfd is %d\n", clientIdArr[i]->listenfd);
            printf("connfd id is %d\n", clientIdArr[i]->client_id);
        }
    }
}

void *broadcast_message(void *data){
    printf("print message");
    fflush(stdout);
}

void remove_from_queue(void *data){
    int clientId= *((int*)data);
    free(data);
    for(int i =0; i<=MAX_CONNECTIONS; i++){
        printf("comparing client id in %d place which is %d with given id %d\n", i, clientIdArr[i]->listenfd, clientId);
        if(clientIdArr[i]->listenfd == clientId){
            printf("should be removed\n");
            fflush(stdout);
        }
    }
}
