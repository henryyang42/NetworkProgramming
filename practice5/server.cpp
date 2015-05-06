#include "practice.h"
#define SERV_PORT 12345

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

void sig_chld(int signo) {
    pid_t pid;
    int stat;
    pid = wait(&stat);
    printf("child %d terminated\n", pid);
    return;
}


int main(int argc, char **argv) {
    int listenfd, connfd, udpfd, nready, maxfdp1;
    char  mesg[MAXLINE];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;
    void sig_chld(int);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    /* for create UDP socket */
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(udpfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    signal(SIGCHLD, sig_chld);/* must call waitpid() */
    FD_ZERO(&rset);
    maxfdp1 = max(listenfd, udpfd) + 1;
    for ( ; ; ) {
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);
        if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0)
        {
            if (errno == EINTR)
                continue; /* back to for() */ else
                printf("select error");
        }
        if (FD_ISSET(listenfd, &rset)) {
            len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &len);
            print_ip_port(cliaddr);
            if ( (childpid = fork()) == 0) { /* child process */
                close(listenfd);
                str_echo(connfd);
                exit(0);
            }
            close(connfd);
        }
        if (FD_ISSET(udpfd, &rset)) {
            len = sizeof(cliaddr);
            n = recvfrom(udpfd, mesg, MAXLINE, 0, (struct sockaddr *) &cliaddr, &len);
            puts("udp");
            sendto(udpfd, mesg, n, 0, (struct sockaddr *) &cliaddr, len);
        }
    }
    return 0;
}
