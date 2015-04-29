#include "HW1.h"

int connect2fd(struct sockaddr_in &addr, char *ip, int port) {
    int sockfd;
    bzero(&addr, sizeof(addr));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        printf("socket error");
    // IP
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
        printf("inet_ption error for %s\n", ip);
    // Port
    addr.sin_port = htons(port);
    // IPV4
    addr.sin_family = AF_INET;
    if (connect(sockfd, (SA *) &addr, sizeof(addr)) < 0)
        printf("connect error\n");
    return sockfd;
}
