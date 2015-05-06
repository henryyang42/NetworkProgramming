#include "HW2.h"
sqlite3 *db; // sqlite3 db struct
char *zErrMsg = 0;
int rc;

int main(int argc, char **argv) {
    int sockfd;
    int nready, client[FD_SETSIZE];
    fd_set rset, allset;
    struct sockaddr_in servaddr, cliaddr;
    if (argc != 2) {
        puts("Usage: ./HW2_101062142_Ser [port]");
        return 0;
    }

    init_db(db);

    sockfd = udp_ser(servaddr, atoi(argv[1]));
    dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
    sqlite3_close(db);
    return 0;
}
