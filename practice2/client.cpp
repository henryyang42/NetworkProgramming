#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define MAXLINE 10000
typedef struct sockaddr SA;

int main (int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1]; /*max text line length */
    struct sockaddr_in servaddr; /*declare server address*/

    if (argc != 2) /*deal with error message*/
        printf("usage: ./server <IPaddress>");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket error");
    /*create an Internet(AF_INET) stream(SOCK_STREAM) socket*/

    bzero(&servaddr, sizeof(servaddr)); /*reset address to zero*/
    servaddr.sin_family = AF_INET; /*IPv4*/
    servaddr.sin_port = htons(8002); /*Port*/
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        printf("inet_ption error for %s\n", argv[1]);

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        printf("connect error\n");
    while ((n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* null terminate */
        if (fputs(recvline, stdout) == EOF)
            printf("fputs error\n");
    }
    if (n < 0)
        printf("read error\n");
    return 0
}
