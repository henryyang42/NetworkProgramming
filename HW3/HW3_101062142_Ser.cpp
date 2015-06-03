#include "HW3.h"
sqlite3 *db; // sqlite3 db struct
char recvline[MAXLINE], buf[MAXLINE];
int n, port;
int sockfd;
fd_set rset, allset;
struct stat filestat;
FILE *fp;
struct sockaddr_in servaddr, cliaddr;
string input, output, cmd, query, ip, username, article, title, content, id, message, tmp, filename;
vector<string> tok;
pair<vector<map<string, string> >, int> result;
vector<map<string, string> > rows, blacklist;
map<string, struct sockaddr_in> online_addr;
map<string, struct sockaddr_in>::iterator iter_online;
map<string, set<string> >file_archive;
map<string, set<string> >::iterator fa_it;
pthread_mutex_t fa_mutex;
void send_to_user(string username, string s) {
    struct sockaddr_in cliaddr = online_addr[username];
    socklen_t len = sizeof(cliaddr);
    sendto(sockfd, s.c_str(), s.length(), 0, (SA*)&cliaddr, len);
}

string get_username() {
    for (iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
        if (get_ip(cliaddr) == get_ip(iter_online->second) && get_port(cliaddr) == get_port(iter_online->second))
            return iter_online->first;
    }
    return "NULL";
}

void broadcast_to_user(string msg) {
    for (iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
        username = iter_online->first;
        if (username != tok[1])
            send_to_user(username, msg);
    }
}

string gfl() {
    string fl = "";
    for (fa_it = file_archive.begin(); fa_it != file_archive.end(); fa_it++) {
        fl += fa_it->first;
        for (set<string>::iterator it = (fa_it->second).begin(); it != (fa_it->second).end(); it++) {
            fl += " " + (*it);
        }

        fl += "\n";
    }
    return fl;
}


string service(string input) {
    tok = strtok(input);
    if (!tok.size()) {
        return "WTF";
    }
    char acc[MAXLINE], pwd[MAXLINE];
    if (tok[0] == "FR") {
        // File regieser
        string username = tok[1];
        string filename = tok[2];
        pthread_mutex_lock(&fa_mutex);
        file_archive[filename].insert(username);
        pthread_mutex_unlock(&fa_mutex);
        return "NORETURN";
    }
    else if (tok[0] == "SF") {
        log("Show File");
        return "S_SF \n" + gfl();
    } else if (tok[0] == "SU") {
        log("Show User");
        string su = "S_SU ";
        for (iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
            su += strfmt("%s %s %d ", (iter_online->first).c_str(),
                get_ip(iter_online->second).c_str(), get_port(iter_online->second));
        }
        return su;
    } else if (tok[0] == "L") {
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
        for (iter_online = online_addr.begin(); iter_online != online_addr.end(); iter_online++) {
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
        for (int i = 0; i < rows.size(); i++) {
            output += strfmt("ID: %-3s | Title: %-20s | Author: %-10s | Hit: %-3s\n",
                             rows[i]["id"].c_str(), rows[i]["title"].c_str(),
                             rows[i]["username"].c_str(), rows[i]["hit"].c_str());
        }
        return output;
    } else if (tok[0] == "DA") {
        log("Delete Article");
        query = strfmt("SELECT * FROM article WHERE id='%s' AND username='%s'", tok[1].c_str(), tok[2].c_str());
        result = exec_sql(db, query);
        if (result.first.size()) {
            query = strfmt("DELETE FROM article WHERE id='%s' AND username='%s'", tok[1].c_str(), tok[2].c_str());
            exec_sql(db, query);
            query = strfmt("DELETE FROM filelist WHERE id='%s'", tok[1].c_str());
            exec_sql(db, query);
            query = strfmt("DELETE FROM blacklist WHERE id='%s'", tok[1].c_str());
            exec_sql(db, query);
        }
        return service("SA");
    } else if (tok[0] == "E") {
        log("Enter Article");
        output = "S_EA \n";
        query = strfmt("SELECT * FROM article WHERE id='%s'", tok[1].c_str());
        result = exec_sql(db, query);
        rows = result.first;
        // Update hit count
        query = strfmt("UPDATE article SET hit='%d' WHERE id='%s'",
                       atoi(rows[0]["hit"].c_str()) + 1, tok[1].c_str());
        exec_sql(db, query);
        query = strfmt("SELECT * from blacklist WHERE id='%s'", rows[0]["id"].c_str());
        blacklist = exec_sql(db, query).first;
        // If is owner

        if (rows[0]["username"] == get_username()) {
            output += "[AB]Add/[DB]Del black list [User ID]\n";
            output += "Blacklist user: ";
            for (int i = 0; i < blacklist.size(); i++) {
                output += blacklist[i]["username"] + " ";
            }
            output += "\n";
        }
        // Premission deny
        for (int i = 0; i < blacklist.size(); i++)
            if (get_username() == blacklist[i]["username"]) {
                output += "Premission Deny!\n";
                return output;
            }

        output += strfmt("Author: %-16s | Hit: %-7s | ID: %-4s\n",
                         rows[0]["username"].c_str(), rows[0]["hit"].c_str(), rows[0]["id"].c_str());
        output += strfmt("IP: %-20s | Port: %-6s\n",
                         rows[0]["ip"].c_str(), rows[0]["port"].c_str());
        output += strfmt("Title: %-17s | Content:\n%-100s\n",
                         rows[0]["title"].c_str(), rows[0]["content"].c_str());

        output += "************* File List *************\n";
        query = strfmt("SELECT * FROM filelist WHERE id='%s'", tok[1].c_str());
        result = exec_sql(db, query);
        rows = result.first;
        for (int i = 0; i < rows.size(); i++) {
            output += strfmt("Uploader: %-14s | Filename: %-s (%-s bytes)\n",
                             rows[i]["username"].c_str(), rows[i]["filename"].c_str(), rows[i]["size"].c_str());
        }
        output += "************* Reply *************\n";
        query = strfmt("SELECT * FROM reply WHERE id='%s'", tok[1].c_str());
        result = exec_sql(db, query);
        rows = result.first;
        for (int i = 0; i < rows.size(); i++) {
            output += strfmt("IP: %-20s | Port: %-6s | Username: %-15s | Message: %-30s\n",
                             rows[i]["ip"].c_str(), rows[i]["port"].c_str(), rows[i]["username"].c_str(), rows[i]["message"].c_str());
        }
        return output;
    } else if (tok[0] == "RE") {
        log("Reply Article");
        message = get_article(3, input);
        username = tok[1];
        id = tok[2];
        struct sockaddr_in cliaddr = online_addr[username];
        query = strfmt("INSERT INTO reply (username, id, message, ip, port) VALUES ('%s', '%s', '%s', '%s', %d)",
                       username.c_str(), id.c_str(), message.c_str(), get_ip(cliaddr).c_str(), get_port(cliaddr));
        exec_sql(db, query);
        return service("E " + id);
    } else if (tok[0] == "AB") {
        id = tok[1];
        query = strfmt("INSERT INTO blacklist (id, username) VALUES (%s, '%s')",
                       id.c_str(), tok[2].c_str());
        exec_sql(db, query);
        return service("E " + id);
    } else if (tok[0] == "DB") {
        id = tok[1];
        query = strfmt("DELETE FROM blacklist WHERE id=%s AND username='%s'",
                       id.c_str(), tok[2].c_str());
        exec_sql(db, query);
        return service("E " + id);
    } else if (tok[0] == "DI") {
        return "S_DI \n" + exec("curl dict://dict.org/d:" + tok[1]);
    } else if (tok[0] == "UP") {
        usleep(WAIT);
        int tot, totalbytes;
        if ((fp = fopen(("Upload/" + tok[3]).c_str(), "wb")) != NULL) {
            puts("Server reciving file...");
            //Receive file
            tot = 0;
            totalbytes = atoi(tok[4].c_str());
            printf("Reciving %d bytes...", totalbytes);
            socklen_t len = sizeof(cliaddr);
            while (tot < totalbytes) {
                int numbytes = recvfrom(sockfd, buf, MAXLINE, 0, (SA*)&cliaddr, &len);
                if (numbytes < 0)
                    break;
                tot += numbytes;
                numbytes = fwrite(buf, sizeof(char), numbytes, fp);
            }
            puts("done.");
            fclose(fp);
        }
        if (tot == totalbytes) {
            query = strfmt("INSERT INTO filelist (id, username, filename, size) VALUES (%s, '%s', '%s', %s)",
                           tok[1].c_str(), tok[2].c_str(), tok[3].c_str(), tok[4].c_str());
            exec_sql(db, query);
        } else {
            exec("rm Upload/" + tok[3]);
        }
        return service("E " + tok[1]);
    } else if (tok[0] == "DO") {
        filename = ("Upload/" + tok[1]);
        if (lstat(filename.c_str(), &filestat) < 0 || (fp = fopen(filename.c_str(), "rb")) == 0) {
            puts("File error.");
            sendto(sockfd, "0", 1, 0, (SA*)&cliaddr,  sizeof(cliaddr));
        } else {
            int sz = filestat.st_size, totalbytes = 0;
            printf("Sending %d bytes...", sz);
            sendto(sockfd, to_string(sz).c_str(), to_string(sz).length(), 0, (SA*)&cliaddr,  sizeof(cliaddr));
            usleep(WAIT);
            while (!feof(fp)) {
                sz = fread(buf, sizeof(char), sizeof(buf), fp);
                sz = sendto(sockfd, buf, sz, 0, (SA*)&cliaddr, sizeof(cliaddr));
                totalbytes += sz;
                usleep(TICK);
            }
            printf(" %d done\n", totalbytes);
            fclose(fp);
        }
        return "OK";
    } else {
        puts("Wrong command");
    }
    return "WTF is " + input;
}

void dg_echo(int sockfd) {
    int n;
    socklen_t len = sizeof(cliaddr);
    char mesg[MAXLINE + 1];
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        log("setsockopt Error");
    }
    while (1) {
        n = recvfrom(sockfd, mesg, MAXLINE, 0, (SA*)&cliaddr, &len);
        if (n == -1) {
            continue;
        }
        mesg[n] = 0;
        print_ip_port(cliaddr);
        output = "ACK";
        sendto(sockfd, output.c_str(), output.length(), 0, (SA*)&cliaddr, len);
        usleep(WAIT);
        printf("GET: %s\n", mesg);
        output = service(mesg);
        if (output != "NORETURN")
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
    exec("mkdir Upload");
    dg_echo(sockfd);
    sqlite3_close(db);
    return 0;
}
