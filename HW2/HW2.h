#ifndef HW2_H
#define HW2_H
#include <arpa/inet.h>
#include <cstdio>
#include <dirent.h>
#include <vector>
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
#include <sys/stat.h>
#include <string>
#include <sstream>
#include <iostream>

#define LISTENQ 1024
#define MAXLINE 2048

using namespace std;

typedef struct sockaddr SA;

int udp_cli(struct sockaddr_in &addr, char *ip, int port);
int udp_ser(struct sockaddr_in &addr, int port);

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen);

string send_to_server(int sockfd, struct sockaddr_in &servaddr, string s);
// Utility
void print_ip_port(struct sockaddr_in &addr);
vector<string> strtok(string s);



#endif
