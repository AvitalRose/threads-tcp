//
// Created by avitalrose on 9/26/21.
//


#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>


#define BUFF_SIZE 1024

void* listen_function(void *p_listenfd);
void* send_function(void *p_listenfd);

pthread_mutex_t socket_mutex;

int main(int argc, char *argv[]) {
    int sockfd = 0;
    struct sockaddr_in serv_addr;
    pthread_mutex_init(&socket_mutex, NULL);

    if (argc != 2) {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error: Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(15470);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: Connect Failed \n");
        return 1;
    }
    printf("---Welcome to chat----\n");

    //Thread for listening;
    pthread_t listen_t;
    int *p_listenfd = malloc(sizeof(int));
    *p_listenfd = sockfd;
    if((pthread_create(&listen_t, NULL, listen_function, p_listenfd)) != 0){
        perror("new connection handler thread creation failed: ");
        return 1;
    }

    //Thread for sending;
    pthread_t sending_t;
    if((pthread_create(&sending_t, NULL, send_function, p_listenfd)) != 0){
        perror("new connection handler thread creation failed: ");
        return 1;
    }
    if((pthread_join(listen_t, NULL)) != 0){
        perror("new connection handler join fail: ");
        return 1;
    }

//    while(1){
//        fflush(stdout);
//        fgets(sendBuff, sizeof(sendBuff),stdin);
//        fflush(stdin);
//        sendBuff[strlen(sendBuff) -1] = '\0';
//        if(strcmp(sendBuff, "exit") == 0){
//            printf("Bye Bye!\n");
//            strcpy(sendBuff, "exit");
//            if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
//            {
//                puts("Send exit failed");
//                return 1;
//            }
//            break;
//        }
//        //Send some data
//        if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
//        {
//            puts("Send failed");
//            return 1;
//        }
//        sendBuff[0]='\0';
//        //Receive a reply from the server
//        if(recv(sockfd , recvBuff , 1024 , 0) < 0)
//        {
//            puts("recv failed");
//            break;
//        }
//        printf("%s\n", recvBuff);
//        fflush(stdout);
//        recvBuff[0]='\0';
//
//    }
//    return 1;
}

void* listen_function(void *p_listenfd){
    int sockfd = *((int*)p_listenfd);
    free(p_listenfd);
    char recvBuff[BUFF_SIZE];
    int re = 0;
    while (1){
        //Receive a reply from the server
        re = recv(sockfd , recvBuff , BUFF_SIZE , 0);
        if(re < 0)
        {
            puts("recv failed");
            break;
        }
        printf("%s\n", recvBuff);
        fflush(stdout);
        recvBuff[0]='\0';
    }
}

void* send_function(void *p_listenfd){
    int sockfd = *((int*)p_listenfd);
    free(p_listenfd);
    char sendBuff[BUFF_SIZE];
    printf("Enter message:\n");
    fflush(stdout);
    while(1){
        fflush(stdout);
        fgets(sendBuff, sizeof(sendBuff),stdin);
        fflush(stdin);
        sendBuff[strlen(sendBuff) -1] = '\0';
        if(strcmp(sendBuff, "exit") == 0){
            printf("Bye Bye!\n");
            strcpy(sendBuff, "exit");
            if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
            {
                puts("Send exit failed");
                return NULL;
            }
            break;
        }
        //Send some data
        if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
        {
            puts("Send failed");
            return NULL;
        }
        sendBuff[0]='\0';
    }
}

