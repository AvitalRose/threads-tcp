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


#define MAX_CONNECTIONS 5
#define recvBuffSize 1024

typedef struct{
    int listenfd;
    int client_id;
} socketVariables;


pthread_mutex_t connection_mutex;

socketVariables *clientsArr[MAX_CONNECTIONS];
int chat_size;
int uid = 1;


void *handle_connection(void* p_uid);
void *new_connection_handler(void* mainSocketDate);
void add_to_queue(socketVariables *clientId);
void print_arr_queue(void *data);
void *broadcast_message(char message[], int uid);
void remove_from_queue(int uid);



int main(){

    int listenfd;
    struct sockaddr_in serv_addr;


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
    pthread_mutex_init(&connection_mutex, NULL);

    while(1){
        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t *) &len)) < 0) {
            perror("Accept failed: ");
        }
        pthread_t new_connection_thread;
        int *p_client = malloc(sizeof(int));
        *p_client = connfd;
        pthread_create(&new_connection_thread, NULL, handle_connection, p_client);
    }


}

void *handle_connection(void *p_uid){
    printf("enterd function");

    //set up connection
    int connfd = *((int*)p_uid);
    free(p_uid);
    char recvBuff[recvBuffSize];
    char exitMessage[1024];
    strcpy(exitMessage, "Exited chat");

    //set up socket
    socketVariables *client = (socketVariables *) malloc(sizeof(socketVariables));
    client->client_id = uid;
    client->listenfd = connfd;
    uid++;
    add_to_queue(client);

    while (1) {
        int read, err;
        if ((read = recv(connfd, recvBuff, recvBuffSize, 0)) < 0) {
            perror("Read from client failed: ");
        }
        printf("strlen of recvBuff is %ld, recvBuff is %s\n", strlen(recvBuff), recvBuff);
//        if(strcmp(recvBuff, "exit") == 0){
//            remove_from_queue(uid);
//        }
        broadcast_message(recvBuff, client->client_id);
        if (!read) {//done reading
            break;
        }
        if ((err = send(connfd, recvBuff, read, 0)) < 0) {
            perror("Send back to client failed: ");
        }
    }
}


void add_to_queue(socketVariables *clientId){
    pthread_mutex_lock(&connection_mutex);
    for(int i = 0; i<MAX_CONNECTIONS; i++){
        if(!clientsArr[i]){
            clientsArr[i] = clientId;
            break;
        }
    }
    chat_size++;
    pthread_mutex_unlock(&connection_mutex);
}

void print_arr_queue(void *data){
    pthread_mutex_lock(&connection_mutex);
    for(int i = 0; i < MAX_CONNECTIONS; i++){
        if(clientsArr[i]) {//only print connections that exist
            printf("connfd is %d\n", clientsArr[i]->listenfd);
            printf("connfd id is %d\n", clientsArr[i]->client_id);
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}

void *broadcast_message(char message[], int uid){
    pthread_mutex_lock(&connection_mutex);
    printf("print message %s from %d\n", message, uid);
    fflush(stdout);
    for(int i = 0; i<MAX_CONNECTIONS; i++){
        if(clientsArr[i]){
            if(clientsArr[i]->client_id != uid){
                printf("send to %d\n", clientsArr[i]->client_id);
                if(send(clientsArr[i]->listenfd, message, strlen(message), 0) < 0){
                    perror("Broadcast message failed: ");
                }
            }
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}

void remove_from_queue(int uid){
    pthread_mutex_lock(&connection_mutex);
    for(int i =0; i<=MAX_CONNECTIONS; i++){
        printf("comparing client id in %d place which is %d with given id %d\n", i, clientsArr[i]->listenfd, uid);
        if(clientsArr[i]->client_id == uid){
            printf("should be removed\n");
            fflush(stdout);
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}
