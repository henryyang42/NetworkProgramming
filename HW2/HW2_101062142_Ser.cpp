#include "HW2.h"
sqlite3 *db; // sqlite3 db struct
char recvline[MAXLINE + 1];
int n, port;
int sockfd;
fd_set rset, allset;
struct sockaddr_in servaddr, cliaddr;
string input, output, cmd, query, ip, username, article, title, content;
vector<string> tok;
pair<vector<map<string, string> >, int> result;
vector<map<string, string> > rows;
map<string, struct sockaddr_in> online_addr;
map<string, struct sockaddr_in>::iterator iter_online;

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
        for (iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
            su += (iter_online->first) + " ";
        }
        return su;
    } else if (tok[0] == "LO") {
        log("Logout");
        username = tok[1];
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
        for(iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
            username = iter_online->first;
            if (username != tok[1])
                send_to_user(username, "S_T " + article);
        }
        return "S_T " + article;
    } else if (tok[0] == "A") {
        log("Add Article");
        stringstream ss;
        ss << input;
        getline(ss, username);
        getline(ss, username);
        getline(ss, title);
        getline(ss, content);
        struct sockaddr_in cliaddr = online_addr[username];
        query = strfmt("INSERT INTO article (username, title, content, ip, port) VALUES ('%s', '%s', '%s', '%s', %d)",
            username.c_str(), title.c_str(), content.c_str(), get_ip(cliaddr).c_str(), get_port(cliaddr));
        exec_sql(db, query);
        return service("SA");
    } else if (tok[0] == "SA") {
        log("Show Article");
        output = "S_SA \n";
        query = "SELECT * FROM article WHERE 1";
        result = exec_sql(db, query);
        rows = result.first;
        for(int i = 0; i < rows.size(); i++) {
            output += strfmt("ID: %-3s | Title: %-20s | Author: %-10s | Hit: %-3s\n",
                rows[i]["id"].c_str(), rows[i]["title"].c_str(),
                rows[i]["username"].c_str(), rows[i]["hit"].c_str());
        }
        return output;
    } else if (tok[0] == "DA") {
        log("Delete Article");
        query = strfmt("DELETE FROM article WHERE id='%s'", tok[1].c_str());
        exec_sql(db, query);
        return service("SA");
    } else if (tok[0] == "E") {
        log("Enter Article");
        output = "S_EA \n";
        query = strfmt("SELECT * FROM article WHERE id='%s'", tok[1].c_str());
        result = exec_sql(db, query);
        rows = result.first;
        // Update hit count
        query = strfmt("UPDATE article SET hit='%d' WHERE id='%s'",
            atoi(rows[0]["hit"].c_str())+1, tok[1].c_str());
        exec_sql(db, query);

        output += strfmt("Author: %-16s | Hit: %-7s | ID: %-4s\n",
                rows[0]["username"].c_str(), rows[0]["hit"].c_str(), rows[0]["id"].c_str());
        output += strfmt("IP: %-20s | Port: %-6s\n",
                rows[0]["ip"].c_str(), rows[0]["port"].c_str());
        output += strfmt("Title: %-17s | Content:\n%-300s\n",
                rows[0]["title"].c_str(), rows[0]["content"].c_str());

        return output;
    } else if (tok[0] == "Y") {

    } else if (tok[0] == "Y") {

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
        //output = "ACK";
        //sendto(sockfd, output.c_str(), output.length(), 0, (SA*)&cliaddr, len);
        //usleep(100000);
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
