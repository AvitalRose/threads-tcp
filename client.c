// TCP - client

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define recvBuffSize 1024
#define messageBuffSize 1024

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[recvBuffSize];
    char message[messageBuffSize];
    struct sockaddr_in serv_addr,my_add;

    if(argc != 2)
    {
        printf("\n Usage: %s <ip of server> \n", argv[0]);
        return 1;
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error: Could not create socket \n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(15470);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error: Connect Failed \n");
        return 1;
    }

//    while ((n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) > 0)
//    {
//        recvBuff[n] = '\0';
//        sleep(2);
//        if(fputs(recvBuff, stdout) == EOF)
//        {
//            printf("\n Error: Fputs error\n");
//        }
//    }
    while(1){
        printf("enter message: ");
        fegets(message, sizeof(message), )//should be sizeof -1? check edge options


        //Send message
        if(send(sockfd, message, strlen(message), 0) < 0){
            perror("Send failed: ");
            return 1;
        }

        //Receive a message
        if(recv(sockfd, recvBuff, recvBuffSize, 0) < 0){
            perror("Recv failed");
            break;
        }

        printf("Server Reply: %s\n", recvBuff);
        recvBuff[0]='\0';

    }

//    if(n < 0)
//    {
//        printf("\n Read error \n");
//    }

    return 0;
}
