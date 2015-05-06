#ifndef PRACTICE_H
#define PRACTICE_H
#include <arpa/inet.h>
#include <cstdio>
#include <dirent.h>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <netdb.h>
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

const char DOWNLOAD[] = "Download";
const char UPLOAD[] = "Upload";

void print_ip_port(struct sockaddr_in &addr);

int connect2fd(struct sockaddr_in &addr, char *ip, int port);
int listen2fd(struct sockaddr_in &addr, int port);

string files2string(vector<string> files);
vector<string> get_dir(const char *path);
string exec(string cmd);

#endif
