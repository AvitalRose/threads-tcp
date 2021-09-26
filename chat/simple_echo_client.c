//
// Created by avitalrose on 9/24/21.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd = 0;
    char recvBuff[1024];
    char sendBuff[1024];
    char clientName[1024];
    struct sockaddr_in serv_addr;

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

    //---Entered the server---
    printf("Welcome to the chat!\nPlease enter your name (up to 100 characters)\n");
    fgets(clientName, sizeof(clientName),stdin);
    fflush(stdin);
    strcat(clientName, ": ");
    printf("name length: %ld\n", strlen(clientName));
    fflush(stdout);

    while(1){
        printf("Enter message: \n");
        fgets(sendBuff, sizeof(sendBuff),stdin);
        fflush(stdin);
        sendBuff[strlen(sendBuff) -1] = '\0';
        if(strcmp(sendBuff, "exit") == 0){
            printf("Bye Bye!\n");
//            strcat(clientName, ": exited chat");
//            strcpy(sendBuff, clientName);
//            if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
//            {
//                puts("Send failed");
//                return 1;
//            }
            break;
        }



        //Send some data
        char tmpClientName[1024];
        strcpy(tmpClientName, clientName);
        strcat(clientName, sendBuff);//adding name to data
        strcpy(sendBuff, clientName);//putting back to send buff, for clarity
        clientName[0] = '\0';
        strcpy(clientName, tmpClientName);

        if(send(sockfd ,sendBuff, strlen(sendBuff) , 0) < 0)
        {
            puts("Send failed");
            return 1;
        }
        sendBuff[0]='\0';
        //Receive a reply from the server
        if(recv(sockfd , recvBuff , 1024 , 0) < 0)
        {
            puts("recv failed");
            break;
        }

        printf("Server Reply: %s", recvBuff);
        recvBuff[0]='\0';

    }
    return 1;
}
