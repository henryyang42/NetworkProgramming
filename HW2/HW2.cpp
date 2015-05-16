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


string send_to_server(int sockfd, struct sockaddr_in &servaddr, string s) {
    int n;
    const char *sendline = s.c_str();
    char recvline[MAXLINE + 1];
    //sendto(sockfd, sendline, strlen(sendline), 0, (SA*)&servaddr, sizeof(servaddr));
    //n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
    write(sockfd, sendline, strlen(sendline));
    //n = read(sockfd, recvline, MAXLINE);
    //recvline[n] = 0;
    //return string(recvline);
    return "done";
}

// Utility

void print_ip_port(struct sockaddr_in &addr) {
    printf("IP address: %s, ", inet_ntoa(addr.sin_addr));
    printf("port: %d\n", (int) ntohs(addr.sin_port));
}

string get_ip(struct sockaddr_in &addr) {
    return string(inet_ntoa(addr.sin_addr));
}

int get_port(struct sockaddr_in &addr) {
    return (int) ntohs(addr.sin_port);
}

vector<string> strtok(string s) {
    vector<string> l;
    stringstream ss;
    ss << s;
    while (ss >> s)
        l.push_back(s);
    return l;
}

std::string strfmt(const std::string fmt_str, ...) {
    int final_n, n = ((int)fmt_str.size()) * 2; /* Reserve two times as much as the length of the fmt_str */
    std::string str;
    std::unique_ptr<char[]> formatted;
    va_list ap;
    while (1) {
        formatted.reset(new char[n]); /* Wrap the plain char array into the unique_ptr */
        strcpy(&formatted[0], fmt_str.c_str());
        va_start(ap, fmt_str);
        final_n = vsnprintf(&formatted[0], n, fmt_str.c_str(), ap);
        va_end(ap);
        if (final_n < 0 || final_n >= n)
            n += abs(final_n - n + 1);
        else
            break;
    }
    return std::string(formatted.get());
}

void get(char s[]) {
    fgets(s, MAXLINE, stdin);
    size_t len = strlen(s) - 1;
    if (s[len] == '\n')
        s[len] = '\0';
}

string get_article(int x, string s) {
    stringstream ss;
    ss << s;
    while (x-- > 0)
        ss >> s;
    getline(ss, s);
    return s;
}

int max(int a, int b) {
    return a > b ? a : b;
}

void log(const char *s) {
#ifdef DEBUG
    puts(s);
#endif
}
// Sqlite3
vector<map<string, string> > fetch_result;

const char init_SQL[10][400] = {
    // Create account table in database
    "CREATE TABLE account (username varchar(30) PRIMARY KEY NOT NULL, password varchar(30))",
    "CREATE TABLE article (id integer PRIMARY KEY NOT NULL, title varchar(30), content varchar(300), time timestamp DEFAULT CURRENT_TIMESTAMP, username varchar(30), ip varchar(30), port integer, hit integer DEFAULT 1)",
    "CREATE TABLE reply (id integer NOT NULL, time timestamp DEFAULT CURRENT_TIMESTAMP, username varchar(30), ip varchar(30), port integer, message varchar(50))",
    "SELECT * from account",
    "", "", ""
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
    for (int i = 0; i < 10; i++) {
        exec_sql(db, init_SQL[i]);
    }
}

pair<vector<map<string, string> >, int> exec_sql(sqlite3* &db, string query) {
    char *zErrMsg = 0;
    fetch_result.clear();
    int rc = sqlite3_exec(db, query.c_str(), callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        printf("%s\n", query.c_str());
        sqlite3_free(zErrMsg);
    }

    return make_pair(fetch_result, rc);
}

int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    int i;
    map<string, string> m;
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        m[azColName[i]] = argv[i] ? argv[i] : "NULL";
    }
    fetch_result.push_back(m);
    printf("\n");
    return 0;
}
