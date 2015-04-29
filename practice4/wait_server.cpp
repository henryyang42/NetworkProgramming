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
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define MAXLINE 10000
#define LISTENQ 1024
typedef struct sockaddr SA;

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    while(waitpid((pid_t)(-1), 0, WNOHANG) > 0){}
    printf("child %d terminated\n", pid);
    return;
}

void str_echo(int sockfd) {
    ssize_t n;
    char buf[MAXLINE]; //MAXLINE is defined by user
again:
    while ((n = read(sockfd, buf, MAXLINE)) > 0)
        write(sockfd, buf, n);
    if (n < 0 && errno == EINTR) /* interrupted by a signal before any data was read*/ goto again; //ignore EINTR
    else if (n < 0)
        printf("str_echo: read error");
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, client_addr;
    char buff[MAXLINE];
    time_t ticks;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*for any interface on server */
    servaddr.sin_port = htons(atoi(argv[1]));
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    bzero(&client_addr, sizeof(client_addr));

    signal(SIGCHLD, sig_chld);


    while (1) {
        int addr_len = sizeof(client_addr);
        connfd = accept(listenfd, (SA *)&client_addr, (socklen_t *)&addr_len);
        printf("IP address is: %s, ", inet_ntoa(client_addr.sin_addr));
        printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
        pid_t pid;
        if ((pid = fork()) == 0) { /* child process */
            close(listenfd); /* close listening socket */
            str_echo(connfd); /* process the request */
            exit (0);
        }
        close(connfd);
        int stat;
        /* parent closes connected socket */
    }
    return 0;
}
