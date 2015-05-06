#include "HW2.h"

int main(int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;
    string input;
    if (argc != 3) {
        puts("Usage: ./HW2_101062142_Cli [IP] [port]");
        return 0;
    }

    sockfd = udp_cli(servaddr, argv[1], atoi(argv[2]));

    while (getline(cin, input)) {
        string resp = send_to_server(sockfd, servaddr, input);
        cout << resp << endl;
    }

    return 0;
}
