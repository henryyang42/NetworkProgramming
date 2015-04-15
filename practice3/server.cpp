#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cmath>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAXLINE 10000
#define LISTENQ 1024
typedef struct sockaddr SA;

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    pid = wait(&stat);
    printf("child %d terminated\n", pid);
    return;
}

void str_echo(FILE* fp, int sockfd) {
    int maxfdp1, stdineof;
    fd_set rset;
    ssize_t n;
    char sendline[MAXLINE], recvline[MAXLINE]; //MAXLINE is defined by user

    stdineof = 0;
    FD_ZERO(&rset);
    while (1) {
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = fmax(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(sockfd, &rset)) {
            /* socket is readable */
            int len;
            if ((len = read(sockfd, recvline, MAXLINE)) == 0) {
                if (stdineof == 1)
                    printf("str_cli: server terminated normally\n");
                else
                    printf("str_cli: server terminated prematurely\n");
                return;
            }
            memset(recvline + len, 0, MAXLINE - len);
            fputs(recvline, stdout);
            write(sockfd, recvline, MAXLINE);
        }
        if (FD_ISSET(fileno(fp), &rset)) { /* input is readable */
            if (fgets(sendline, MAXLINE, fp) == NULL) { //EOF client至server 方向連結已斷
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, sendline, strlen(sendline));
        }
    }

}

int main(int argc, char **argv) {
    int listenfd, connfd, sockfd, n;
    int nearby, clients[FD_SETSIZE];
    struct sockaddr_in servaddr, client_addr;
    time_t ticks;
    char sendline[MAXLINE], recvline[MAXLINE]; //MAXLINE is defined by user

    fd_set rset, allset;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*for any interface on server */
    servaddr.sin_port = htons(atoi(argv[1]));
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    //signal(SIGCHLD, sig_chld);

    int maxfd = listenfd; /* initialize */
    int maxi = -1; /* index into client[] array */
    for (int i = 0; i < FD_SETSIZE; i++)
        clients[i] = -1; /* -1 indicates available entry */
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while (1) {
        rset = allset; /* structure assignment */
        nearby = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            /* new client connection */
            int addr_len = sizeof(client_addr);
            connfd = accept(listenfd, (SA *)&client_addr, (socklen_t *)&addr_len);
            printf("IP address is: %s, ", inet_ntoa(client_addr.sin_addr));
            printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
            for (int i = 0; i < FD_SETSIZE; i++)
                if (clients[i] < 0) {
                    clients[i] = connfd; /* save descriptor */
                    if (i == FD_SETSIZE)
                        printf("too many clients");
                    FD_SET(connfd, &allset);
                    if (connfd > maxfd)
                        maxfd = connfd;
                    if (i > maxi)
                        maxi = i;
                    break;
                }
        }
        for (int i = 0; i <= maxi; i++) {
            if ( (sockfd = clients[i]) < 0)
                continue;
            if (FD_ISSET(sockfd, &rset)) {
                if ( (n = read(sockfd, recvline, MAXLINE)) == 0) {
                    /* connection closed by client */
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    clients[i] = -1;
                } else {
                    if (recvline[0] == 'e') {
                        printf("Shutdown\n");
                        FD_CLR(sockfd, &allset);
                        clients[i] = -1;
                    } else
                        write(sockfd, recvline, n);
                }
            }

        }
    }
    return 0;
}
