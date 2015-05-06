#include "HW2.h"

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    if (argc != 2) {
        puts("Usage: ./HW2_101062142_Ser [port]");
        return 0;
    }

    sockfd = udp_ser(servaddr, atoi(argv[1]));
    dg_echo(sockfd, (SA*)&cliaddr, sizeof(cliaddr));
    return 0;
}
