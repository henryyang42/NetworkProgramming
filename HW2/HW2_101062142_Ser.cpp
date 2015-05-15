#include "HW2.h"
sqlite3 *db; // sqlite3 db struct
char recvline[MAXLINE + 1];
int n, port;
int sockfd;
fd_set rset, allset;
struct sockaddr_in servaddr, cliaddr;
string input, output, cmd, query, ip, username, article;
vector<string> tok;
pair<vector<map<string, string> >, int> result;
map<string, pair<string, int> > online;
map<string, struct sockaddr_in> online_addr;
map<string, pair<string, int> >::iterator iter_online;
map<pair<string, int>, string> ronline;

void send_to_user(string username, string s) {
    struct sockaddr_in cliaddr = online_addr[username];
    socklen_t len = sizeof(cliaddr);
    sendto(sockfd, s.c_str(), s.length(), 0, (SA*)&cliaddr, len);
}

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
            online_addr[username] = cliaddr;
            return "S_L SUCCESS " + username;
        }
        return "S_L FAIL";
    } else if (tok[0] == "R") {
        log("Register");
        query = strfmt("INSERT INTO account VALUES ('%s', '%s')", tok[1].c_str(), tok[2].c_str());
        result = exec_sql(db, query);
        if (result.second == SQLITE_OK)
            return "S_R SUCCESS";
        return "S_R FAIL";
    } else if (tok[0] == "SU") {
        log("Show User");
        string su = "S_SU ";
        for(iter_online = online.begin(); iter_online != online.end(); iter_online++) {
            su += (iter_online->first) + " ";
        }
        return su;
    } else if (tok[0] == "LO") {
        log("Logout");
        username = tok[1];
        ronline.erase(online[username]);
        online.erase(username);
        online_addr.erase(username);
        return "S_LO SUCCESS";
    } else if (tok[0] == "D") {
        log("Delete Account");
        query = strfmt("DELETE FROM account WHERE username='%s'", tok[1].c_str());
        exec_sql(db, query);
        return "S_LO";
    } else if (tok[0] == "T") {
        log("Tell User");
        username = tok[1];
        article = username + ":" + get_article(3, input);
        send_to_user(tok[2], "S_T " + article);
        return "S_T " + article;
    } else if (tok[0] == "Y") {
        log("Yell User");
        username = tok[1];
        article = username + ":" + get_article(2, input);
        for(iter_online = online.begin(); iter_online != online.end(); iter_online++) {
            username = iter_online->first;
            if(username != tok[1])
                send_to_user(username, "S_T " + article);
        }
        return "S_T " + article;
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
