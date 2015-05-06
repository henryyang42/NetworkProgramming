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
void dg_cli(FILE *fp, int sockfd, SA *pservaddr, socklen_t servlen) {
    int n;
    char sendline[MAXLINE], recvline[MAXLINE + 1];

    connect(sockfd, (struct sockaddr *) pservaddr, servlen);
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        write(sockfd, sendline, strlen(sendline));
        n = read(sockfd, recvline, MAXLINE);
        recvline[n] = 0; /* null terminate */
        fputs(recvline, stdout);
    }
}

int main (int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1]; /*max text line length */
    struct sockaddr_in servaddr; /*declare server address*/

    if (argc != 3) /*deal with error message*/
        printf("usage: ./server <IPaddress> port");
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        printf("socket error");
    /*create an Internet(AF_INET) stream(SOCK_STREAM) socket*/

    bzero(&servaddr, sizeof(servaddr)); /*reset address to zero*/
    servaddr.sin_family = AF_INET; /*IPv4*/
    servaddr.sin_port = htons(atoi(argv[2])); /*Port*/
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        printf("inet_ption error for %s\n", argv[1]);

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        printf("connect error\n");
    dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));

    return 0;
}
