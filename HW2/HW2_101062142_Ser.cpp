#include "HW2.h"
sqlite3 *db; // sqlite3 db struct
char *zErrMsg = 0;
char recvline[MAXLINE+1];
int rc, n;
int sockfd, listenfd, connfd;
int clients[FD_SETSIZE+1], maxfd, maxi = -1;
fd_set rset, allset;
struct sockaddr_in servaddr, cliaddr;

void init_fd_set(){
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    /* -1 indicates available entry */
    for (int i = 0; i < FD_SETSIZE; i++)
        clients[i] = -1;
}

int main(int argc, char **argv) {

    if (argc != 2) {
        puts("Usage: ./HW2_101062142_Ser [port]");
        return 0;
    }
    maxfd = listenfd = udp_ser(servaddr, atoi(argv[1]));
    init_fd_set();
    init_db(db);

    while (1) {
        rset = allset; /* structure assignment */
        select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (FD_ISSET(listenfd, &rset)) {
            /* new client connection */
            int addr_len = sizeof(cliaddr);
            //connfd = accept(listenfd, (SA *)&cliaddr, (socklen_t *)&addr_len);
            socklen_t len = sizeof(cliaddr);
            n = recvfrom(listenfd, recvline, MAXLINE, 0, (SA*)&cliaddr, &len);
            puts(recvline);
            print_ip_port(cliaddr);
            listenfd = udp_ser(servaddr, atoi(argv[1]));
            addr_len = sizeof(cliaddr);
            //connfd = accept(listenfd, (SA *)&cliaddr, (socklen_t *)&addr_len);
            len = sizeof(cliaddr);
            n = recvfrom(listenfd, recvline, MAXLINE, 0, (SA*)&cliaddr, &len);
            puts(recvline);
            print_ip_port(cliaddr);
            return 0;
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


    dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
    sqlite3_close(db);
    return 0;
}
