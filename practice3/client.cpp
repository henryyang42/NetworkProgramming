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
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cmath>
#define MAXLINE 10000
typedef struct sockaddr SA;

void str_cli(FILE *fp, int sockfd) {
    int maxfdp1;
    fd_set rset;
    char sendline[MAXLINE], recvline[MAXLINE];

    FD_ZERO(&rset);

    while (1) {
        FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = fmax(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, 0);
        if (FD_ISSET(sockfd, &rset)) {
            /* socket is readable */
            if (read(sockfd, recvline, MAXLINE) == 0){
                printf("str_cli: server terminated prematurely");
                return;
            }
            fputs(recvline, stdout);
        }
        if (FD_ISSET(fileno(fp), &rset)) {
            /* input is readable */
            if (fgets(sendline, MAXLINE, fp) == NULL)
                return; /* all done */
            write(sockfd, sendline, strlen(sendline));
        }
    }
}



int main (int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1]; /*max text line length */
    struct sockaddr_in servaddr; /*declare server address*/

    if (argc != 3) /*deal with error message*/
        printf("usage: ./server <IPaddress> port");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket error");
    /*create an Internet(AF_INET) stream(SOCK_STREAM) socket*/

    bzero(&servaddr, sizeof(servaddr)); /*reset address to zero*/
    servaddr.sin_family = AF_INET; /*IPv4*/
    servaddr.sin_port = htons(atoi(argv[2])); /*Port*/
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        printf("inet_ption error for %s\n", argv[1]);

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        printf("connect error\n");
    str_cli(stdin, sockfd);

    if (n < 0)
        printf("read error\n");

    return 0;
}
