#include "HW2.h"
sqlite3 *db; // sqlite3 db struct
char recvline[MAXLINE + 1];
int n, port;
int sockfd;
fd_set rset, allset;
struct sockaddr_in servaddr, cliaddr;
string input, output, cmd, query, ip, username;
vector<string> tok;
pair<vector<map<string, string> >, int> result;
map<string, pair<string, int> > online;
map<string, pair<string, int> >::iterator iter_online;
map<pair<string, int>, string> ronline;

string service(string input) {
    tok = strtok(input);
    if (!tok.size()) {
        return "WTF";
    }
    char acc[MAXLINE], pwd[MAXLINE];
    if (tok[0] == "L") {
        log("Login");
        query = strfmt("SELECT * FROM account WHERE username='%s' and password='%s'", tok[1].c_str(), tok[2].c_str());
        result = exec_sql(db, query);
        if (result.first.size() == 1) {
            string username = result.first[0]["username"];
            ip = get_ip(cliaddr);
            port = get_port(cliaddr);
            online[username] = make_pair(ip, port);
            ronline[make_pair(ip, port)] = username;
            return "S_L SUCCESS " + username;
        }
        return "LOGIN FAIL";
    } else if (tok[0] == "R") {
        log("Register");
        query = strfmt("INSERT INTO account VALUES ('%s', '%s')", tok[1].c_str(), tok[2].c_str());
        result = exec_sql(db, query);
        if (result.second == SQLITE_OK)
            return "S_R SUCCESS";
        return "S_R FAIL";
    } else if (tok[0] == "SU") {
        string su = "S_SU ";
        for(iter_online = online.begin(); iter_online != online.end(); iter_online++) {
            su += (iter_online->first) + " ";
        }
        return su;
    } else if (tok[0] == "LO") {
        username = tok[1];
        ronline.erase(online[username]);
        online.erase(username);
        return "S_LO SUCCESS";
    } else if (tok[0] == "R") {

    } else if (tok[0] == "R") {

    } else if (tok[0] == "R") {

    } else {
        puts("Wrong command");
    }
    return "WTF is " + input;
}

void dg_echo(int sockfd) {
    int n;
    socklen_t len = sizeof(cliaddr);
    char mesg[MAXLINE + 1];
    while (1) {
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (SA*)&cliaddr, &len);
        mesg[n] = 0;
        print_ip_port(cliaddr);
        printf("GET: %s\n", mesg);
        output = service(mesg);
        sendto(sockfd, output.c_str(), output.length(), 0, (SA*)&cliaddr, len);
    }
}

int main(int argc, char **argv) {

    if (argc != 2) {
        puts("Usage: ./HW2_101062142_Ser [port]");
        return 0;
    }
    sockfd = udp_ser(servaddr, atoi(argv[1]));
    init_db(db);

    dg_echo(sockfd);
    sqlite3_close(db);
    return 0;
}
