#ifndef HW2_H
#define HW2_H
#include <arpa/inet.h>
#include <cstdio>
#include <dirent.h>
#include <termios.h>
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
#include <map>
#include <sqlite3.h>
#include <stdarg.h>  // For va_start, etc.
#include <memory>    // For std::unique_ptr
#include <utility>
#define LISTENQ 1024
#define MAXLINE 2048
#define FILETIMEOUT 200000
#define WAIT 200000
#define TICK 200
#define DEBUG

using namespace std;


typedef struct sockaddr SA;

int udp_cli(struct sockaddr_in &addr, char *ip, int port);
int udp_ser(struct sockaddr_in &addr, int port);

string send_to_server(int sockfd, struct sockaddr_in &servaddr, string s);
// Utility
void print_ip_port(struct sockaddr_in &addr);
string get_ip(struct sockaddr_in &addr);
int get_port(struct sockaddr_in &addr);

vector<string> strtok(string s);
std::string strfmt(const std::string fmt_str, ...);
void get(char s[]);
string get_article(int x, string s);
int max(int a, int b);
void log(const char *s);
string exec(string cmd);

// Sqlite3
void init_db(sqlite3* &db);
pair<vector<map<string, string> >, int> exec_sql(sqlite3* &db, string query);
int callback(void *NotUsed, int argc, char **argv, char **azColName);



#endif
