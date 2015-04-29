#include "HW1.h"

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    pid = wait(&stat);
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
        /* parent closes connected socket */
    }
    return 0;
}
