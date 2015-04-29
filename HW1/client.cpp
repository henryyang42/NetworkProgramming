#include "HW1.h"

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

int main (int argc, char **argv) {
        int sockfd;
        char recvline[MAXLINE + 1];
        struct sockaddr_in servaddr;

        if (argc != 3)
            printf("usage: ./client <IPaddress> port");

        sockfd = connect2fd(servaddr, argv[1], atoi(argv[2]));
        str_cli(stdin, sockfd);
    return 0;
}
