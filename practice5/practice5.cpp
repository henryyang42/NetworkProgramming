#include "practice.h"
int main(int argc, char **argv) {
    int sockfd, n;
    char recvline[MAXLINE + 1];
    struct sockaddr_in servaddr;
    struct in_addr **pptr, *inetaddrp[2], inetaddr;
    struct hostent *hp;
    struct servent *sp;
    if (argc != 3)
        printf("usage: daytimetcpcli1 <hostname> <service>");
    if ( (hp = gethostbyname(argv[1])) == NULL) {
        if (inet_aton(argv[1], &inetaddr) == 0) {
            printf("hostname error for %s: %s", argv[1], hstrerror(h_errno));
        } else {
            inetaddrp[0] = &inetaddr;
            inetaddrp[1] = NULL;
            pptr = inetaddrp;
        }
    } else {
        pptr = (struct in_addr **) hp->h_addr_list;
    }
    if ( (sp = getservbyname(argv[2], "tcp")) == NULL)
        printf("getservbyname error for %s", argv[2]);
    for ( ; *pptr != NULL; pptr++) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        bzero(&servaddr, sizeof(servaddr)); servaddr.sin_family = AF_INET;
        servaddr.sin_port = sp->s_port;
        memcpy(&servaddr.sin_addr, *pptr, sizeof(struct in_addr));
        //printf("trying %s\n", sock_ntop((struct sockaddr *) &servaddr, sizeof(servaddr)));

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) == 0)
            break; /* success */
        printf("connect error");
        close(sockfd);
    }
    if (*pptr == NULL)
        printf("unable to connect");
    while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
        recvline[n] = 0; /* null terminate */
        fputs(recvline, stdout);
    }
    exit(0);

}
