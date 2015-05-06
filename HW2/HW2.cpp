#include "HW2.h"


int udp_cli(struct sockaddr_in &servaddr, char *ip, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        puts("socket error");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr) <= 0) {
        printf("inet_ption error for %s\n", ip);
        exit(0);
    }
    servaddr.sin_port = htons(port);

    return sockfd;
}


int udp_ser(struct sockaddr_in &servaddr, int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        puts("socket error");
        exit(0);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        puts("bind error");
        exit(0);
    }

    return sockfd;
}

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen){
    int n;
    socklen_t len;
    char mesg[MAXLINE];
    while(1){
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        mesg[n] = 0;
        print_ip_port(*(struct sockaddr_in*)pcliaddr);
        printf("GET: %s\n", mesg);
        sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
}

string send_to_server(int sockfd, struct sockaddr_in &servaddr, string s) {
    int n;
    const char *sendline = s.c_str();
    char recvline[MAXLINE+1];
    sendto(sockfd, sendline, strlen(sendline), 0, (SA*)&servaddr, sizeof(servaddr));
    n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
    recvline[n] = 0;
    return string(recvline);
}

// Utility

void print_ip_port(struct sockaddr_in &addr) {
    printf("IP address: %s, ", inet_ntoa(addr.sin_addr));
    printf("port: %d\n", (int) ntohs(addr.sin_port));
}

vector<string> strtok(string s) {
    vector<string> l;
    stringstream ss;
    ss << s;
    while(ss >> s)
        l.push_back(s);
    return l;
}

