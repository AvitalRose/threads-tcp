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
#include <stdbool.h>


#define MAX_CONNECTIONS 5
#define recvBuffSize 1024

typedef struct{
    int listenfd;
    int client_id;
    char client_name[1024];
} socketVariables;


pthread_mutex_t connection_mutex;

socketVariables *clientsArr[MAX_CONNECTIONS];
int chat_size = 0;
int uid = 1;


void *handle_connection(void* p_uid);
void *new_connection_handler(void* mainSocketDate);
void add_to_queue(socketVariables *clientId);
void print_arr_queue();
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
    char addressBuff[300];
    char max_message[1024];
    strcpy(max_message, "Server reached max connections, try later");
    char welcome_message[1024];
    strcpy(welcome_message, "====Welcome to the chat====");
    pthread_mutex_init(&connection_mutex, NULL);

    while(1){
        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t *) &len)) < 0) {
            perror("Accept failed: ");
        }
        chat_size = chat_size + 1;
        printf("chat size is %d\n", chat_size);
        if(chat_size > MAX_CONNECTIONS){
            printf("reached max connections");
            if(send(connfd, max_message, strlen(max_message), 0) < 0){
                perror("sending max message failed\n");
            }
            if(close(connfd)){
                perror("close fd failed: ");
            }
            chat_size--;
        } else {
            if(send(connfd, welcome_message, sizeof (welcome_message), 0) < 0){
                perror("sending welcome message failed\n");
            }
            pthread_t new_connection_thread;
            int *p_client = malloc(sizeof(int));
            *p_client = connfd;
            pthread_create(&new_connection_thread, NULL, handle_connection, p_client);
        }
    }


}

void *handle_connection(void *p_uid){
    //set up connection
    printf("entered handle connection\n");
    int connfd = *((int*)p_uid);
    free(p_uid);
    char recvBuff[recvBuffSize];
    char exitMessage[1024];
    strcpy(exitMessage, "Exited chat");
    bool setNameFlag = false;

    //set up socket
    socketVariables *client = (socketVariables *) malloc(sizeof(socketVariables));
    client->client_id = uid;
    client->listenfd = connfd;
    uid++;
    if(recv(connfd, recvBuff, recvBuffSize, 0) < 0){
        perror("Read name failed: ");
    }
    strcpy(client->client_name, recvBuff);
    memset(recvBuff, 0, sizeof(recvBuff));

    add_to_queue(client);
    print_arr_queue();

    while (1) {
        int read, err;
        if ((read = recv(connfd, recvBuff, recvBuffSize, 0)) < 0) {
            perror("Read from client failed: ");
        }
        if(read > 0){
            broadcast_message(recvBuff, client->client_id);
        } else{
            remove_from_queue(client->client_id);
            int return_value = 1;
            pthread_exit(&return_value);
        }
        memset(recvBuff, 0, sizeof(recvBuff));
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
    pthread_mutex_unlock(&connection_mutex);
}

void print_arr_queue(){
    pthread_mutex_lock(&connection_mutex);
    for(int i = 0; i < MAX_CONNECTIONS; i++){
        if(clientsArr[i]) {//only print connections that exist
            printf("connfd is %d\n", clientsArr[i]->listenfd);
            printf("connfd id is %d\n", clientsArr[i]->client_id);
            printf("connfd name is %s\n", clientsArr[i]->client_name);
        }
    }
    pthread_mutex_unlock(&connection_mutex);
}

void *broadcast_message(char message[], int uid){
    pthread_mutex_lock(&connection_mutex);
    char message_with_name[2048];
    for (int i = 0; i<MAX_CONNECTIONS; i++){
        if(clientsArr[i]){
            if(clientsArr[i]->client_id == uid){
                strcpy(message_with_name, clientsArr[i]->client_name);
            }
        }
    }
    strcat(message_with_name, ": ");
    strcat(message_with_name, message);
    printf("message to broadcast: %s\n", message_with_name);
    for(int i = 0; i<MAX_CONNECTIONS; i++){
        if(clientsArr[i]){
            if(clientsArr[i]->client_id != uid){
                printf("send to %d\n", clientsArr[i]->client_id);
                if(send(clientsArr[i]->listenfd, message_with_name, strlen(message_with_name), 0) < 0){
                    perror("Broadcast message failed: ");
                }
            }
        }
    }
    memset(message, 0, strlen(message));
    pthread_mutex_unlock(&connection_mutex);
}

void remove_from_queue(int uid){
    pthread_mutex_lock(&connection_mutex);
    for(int i =0; i<MAX_CONNECTIONS; i++){
        if(clientsArr[i]){
            printf("comparing client id in %d place which is %d with given id %d\n", i, clientsArr[i]->listenfd, uid);
            if(clientsArr[i]->client_id == uid){
                printf("should be removed %d\n", uid);
                if(close(clientsArr[i]->listenfd) !=0){
                    //when closing- does it terminate the thread too?
                    perror("close socket failed: ");
                }
                clientsArr[i] = NULL;
                fflush(stdout);
            }
        }
    }
    chat_size--;
    pthread_mutex_unlock(&connection_mutex);
}
