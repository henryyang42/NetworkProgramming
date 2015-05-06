#include "practice.h"

void str_cli(FILE *fp, int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    while (fgets(sendline, MAXLINE, fp) != NULL) {
        write(sockfd, sendline, strlen (sendline));
        if (read(sockfd, recvline, MAXLINE) == 0) {
            printf("str_cli: server terminated prematurely");
            exit(0);
        }
        fputs(recvline, stdout);
    }
}

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

    if (argc != 4) /*deal with error message*/
        printf("usage: ./server <type> <IPaddress> port");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket error");
    /*create an Internet(AF_INET) stream(SOCK_STREAM) socket*/
    if (string(argv[1]) == "tcp") {
        bzero(&servaddr, sizeof(servaddr)); /*reset address to zero*/
        servaddr.sin_family = AF_INET; /*IPv4*/
        servaddr.sin_port = htons(atoi(argv[3])); /*Port*/
        if (inet_pton(AF_INET, argv[2], &servaddr.sin_addr) <= 0)
            printf("inet_ption error for %s\n", argv[2]);

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
            printf("connect error\n");
        str_cli(stdin, sockfd);
    } else {
        bzero(&servaddr, sizeof(servaddr)); /*reset address to zero*/
        servaddr.sin_family = AF_INET; /*IPv4*/
        servaddr.sin_port = htons(atoi(argv[3])); /*Port*/
        if (inet_pton(AF_INET, argv[2], &servaddr.sin_addr) <= 0)
            printf("inet_ption error for %s\n", argv[2]);

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
            printf("connect error\n");
        dg_cli(stdin, sockfd, (SA *) &servaddr, sizeof(servaddr));
    }

    return 0;
}
