#ifndef HW1_H
#define HW1_H
#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 1024
#define MAXLINE 10000
typedef struct sockaddr SA;

int connect2fd(struct sockaddr_in &addr, char *ip, int port);

#endif
