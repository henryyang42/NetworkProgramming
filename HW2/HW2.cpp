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
    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) {
        printf("connect error");
        exit(0);
    }

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

    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    return sockfd;
}

void dg_echo(int sockfd, SA *pcliaddr, socklen_t clilen) {
    int n;
    socklen_t len = clilen;;
    char mesg[MAXLINE+1];
    while (1) {
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
    char recvline[MAXLINE + 1];
    //sendto(sockfd, sendline, strlen(sendline), 0, (SA*)&servaddr, sizeof(servaddr));
    //n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
    write(sockfd, sendline, strlen(sendline));
    n = read(sockfd, recvline, MAXLINE);
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
    while (ss >> s)
        l.push_back(s);
    return l;
}

// Sqlite3
const char init_SQL[10][100] = {
    // Create account table in database
    "CREATE TABLE account (username varchar(30) PRIMARY KEY NOT NULL, password varchar(30))",
    // Create a new account in database
    "INSERT INTO account (username, password) VALUES ('root', 'root')"
};


void init_db(sqlite3* &db) {
    // Open the test.db file
    int rc = sqlite3_open("bbs.db", &db);
    char *zErrMsg = 0;
    if (rc) {
        // failed
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(0);
    }
    else {
        // success
        fprintf(stderr, "Open database successfully\n");
    }
    // execute all the init sql statements
    for (int i = 0; i < 2; i++) {
        rc = sqlite3_exec(db, init_SQL[i], callback, 0, &zErrMsg);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            printf("%s\n", init_SQL[i]);
            sqlite3_free(zErrMsg);
        }
    }
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

/* reads from keypress, doesn't echo */
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
/* reads from keypress, echoes */
int getche(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}
