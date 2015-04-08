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
#include <arpa/inet.h>

#define MAXLINE 10000
#define LISTENQ 1024
typedef struct sockaddr SA;


int main(int argc, char **argv) {
    int listenid, connfd;
    struct sockaddr_in servaddr, client_addr;
    char buff[MAXLINE];
    time_t ticks;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /*for any interface on server */
    servaddr.sin_port = htons(8002);
    listenid = socket(AF_INET, SOCK_STREAM, 0);
    bind(listenid, (SA *)&servaddr, sizeof(servaddr));
    listen(listenid, LISTENQ);
    bzero(&client_addr, sizeof(client_addr));

    while (1) {
        int addr_len = sizeof(client_addr);
        connfd = accept(listenid, (SA *)&client_addr, (socklen_t *)&addr_len);
        ticks = time(NULL);
        snprintf(buff, sizeof(buff), "%.24s\r\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));

        printf("execute netstat:\n");
        puts("=========================================");
        int pid = fork();
        if (pid == 0) {
            execlp("/usr/sbin/netstat", "netstat -a", NULL);
        } else {
            waitpid(pid, NULL, 0);
            printf("\nnetstat finished!!\n\n");
            printf("IP address is: %s, ", inet_ntoa(client_addr.sin_addr));
            printf("port is: %d\n", (int) ntohs(client_addr.sin_port));
            puts("=========================================");
            close(connfd);
        }
    }
    return 0;
}
