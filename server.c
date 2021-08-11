//
// Created by avitalrose on 8/10/21.
//

// TCP - server

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

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0,len;
    struct sockaddr_in serv_addr,remote_addr;

    char sendBuff[1025];
    char addressBuff[300];
    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET - IPv4 Internet protocols

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(15470);

    bind(listenfd, (struct sockaddr*)&serv_addr, (socklen_t)sizeof(struct sockaddr_in));

    listen(listenfd, 10);

    while(1)
    {
        len=sizeof(struct sockaddr_in);
        connfd = accept(listenfd, (struct sockaddr*)&remote_addr, (socklen_t*)&len);
        inet_ntop(AF_INET,&(remote_addr.sin_addr),addressBuff,len);	//copy the address to string
        printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        sleep(2);

        write(connfd, sendBuff, strlen(sendBuff));

        close(connfd);
        sleep(1);
    }
}
