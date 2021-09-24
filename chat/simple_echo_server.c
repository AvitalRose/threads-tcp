//
// Created by avitalrose on 9/24/21.
//


#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define recvBuffSize 1024

int main() {

    int listenfd, len, connfd;
    struct sockaddr_in serv_addr, remote_addr;
    char recvBuff[1024];
    char addressBuff[300];

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket opening error: ");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(15470);

    if ((bind(listenfd, (struct sockaddr *) &serv_addr, (socklen_t)sizeof(struct sockaddr_in))) < 0) {
        perror("Socket bind error: ");
        return 1;
    }

    if ((listen(listenfd, 10)) < 0) {
        perror("Listen error: ");
        return 1;
    }

    while (1) {
        len = sizeof(struct sockaddr_in);
        if ((connfd = accept(listenfd, (struct sockaddr *) &remote_addr, (socklen_t * ) & len)) < 0) {
            perror("Accept failed: ");
        } else {
            inet_ntop(AF_INET, &(remote_addr.sin_addr), addressBuff, len);
            printf("got connection from: %s:%d\n", addressBuff, ntohs(remote_addr.sin_port));
        }
        while (1) {
            int read, err;
            if ((read = recv(connfd, recvBuff, recvBuffSize, 0)) < 0) {
                perror("Read from client failed: ");
            }
            printf("%s\n", recvBuff);
            if (!read) {//done reading
                break;
            }
            if ((err = send(connfd, recvBuff, read, 0)) < 0) {
                perror("Send back to client failed: ");

            }
        }
    }
}
