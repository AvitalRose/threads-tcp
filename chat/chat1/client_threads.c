//
// Created by avitalrose on 9/26/21.
//



#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define BUFF_SIZE 1024

void *send_func(void *p_sockfd);
void *recv_func(void *p_sockfd);
void intHandler(int sig_num);
int terminate();

int sockfd = 0;



int main(int argc, char *argv[]) {
//    int sockfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[BUFF_SIZE];
    char recvBuff[BUFF_SIZE];


    if (argc != 2) {
        printf("Usage: %s <ip of server> \n", argv[0]);
        return -1;
    }

    //setting up name
    printf("Enter name: (from 1 up to 30 characters)\n");
    fflush(stdout);
    fgets(sendBuff, sizeof(sendBuff),stdin);
    if(strlen(sendBuff) <= 0 || strlen(sendBuff)> 30){
        printf("Name to long, try connecting again");
        return -1;
    }
    fflush(stdin);
    sendBuff[strlen(sendBuff) -1] = '\0';


    //setting up client socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error: Could not create socket \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(15470);

    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        printf("inet_pton error occured\n");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Error: Connect Failed \n");
        return -1;
    }

    if(send(sockfd, sendBuff, sizeof(sendBuff), 0) < 0){
        perror("Send name failed: ");
    }

    //get welocmed to chat
    if(recv(sockfd , recvBuff , 1024 , 0) < 0){
        perror("Recv welcome failed: ");
    }
    recvBuff[strlen(recvBuff)] = '\0';
    printf("%s\n", recvBuff);
    if(strcmp(recvBuff, "Server reached max connections, try later") == 0){
        terminate();
    }

    //create listen and receive thread
    pthread_t send_thread;
    int *p_sockfd = malloc(sizeof(int));
    *p_sockfd = sockfd;
    if(pthread_create(&send_thread, NULL, send_func, p_sockfd) != 0){
        perror("Send thread creation fail: ");
        return -1;
    }

    pthread_t recv_thread;
    if(pthread_create(&recv_thread, NULL, recv_func, p_sockfd) != 0){
        perror("Send thread creation fail: ");
        return -1;
    }


    //setting up signal for ctrl+c
    signal(SIGINT, intHandler);

    //wait for both threads to terminate
    pthread_join(send_thread, NULL);
    pthread_join(recv_thread, NULL);
    close(sockfd);

    return 1;
}

//function to handle sending messages;
void *send_func(void *p_sockfd){
    int sockfd= *((int*)p_sockfd);
    char sendBuff[BUFF_SIZE];

    while(1){
        printf("->");
        fflush(stdout);
        fgets(sendBuff, sizeof(sendBuff),stdin);
        fflush(stdin);
        if(strlen(sendBuff) <= 0 || strlen(sendBuff)> 100){
            printf("Message to long, try again");
            continue;
        }
        sendBuff[strlen(sendBuff) -1] = '\0';
        if(strcmp(sendBuff, "exit") == 0){
            terminate();
        }
        //Send some data
        if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
        {
            puts("Send failed");
            return NULL;
        }
        sleep(2);
        memset(sendBuff, 0, sizeof(sendBuff));
    }
}

//function to handle incoming messages
void *recv_func(void *p_sockfd){
    int sockfd= *((int*)p_sockfd);
    char recvBuff[BUFF_SIZE];
    int receive;
    while(1){
        receive = recv(sockfd , recvBuff , 1024 , 0);
        if(receive > 0){
            printf("%s\n", recvBuff);
            printf("->");
            fflush(stdout);
            memset(recvBuff, 0, sizeof(recvBuff));
            recvBuff[0] = '\0';
        } else if(receive == 0) {
            break;
        } else {
            perror("recv failed: ");
            memset(recvBuff, 0, sizeof(recvBuff));
        }
    }
}


//function to handle signal (ctrl+c) and terminate chat
void intHandler(int sig_num){
    terminate();
}


//function to terminate chat
int terminate(){
    char sendBuff[BUFF_SIZE];
    strcpy(sendBuff, "exit");
    if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
    {
        puts("Send exit failed: ");
        return -1;
    }
    close(sockfd);
    printf("Exited chat\n");
    exit(1);
}