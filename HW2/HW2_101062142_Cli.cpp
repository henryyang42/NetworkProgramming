#include "HW2.h"
char recvline[MAXLINE];
struct stat filestat;
FILE *fp;
char buf[MAXLINE];
int sockfd, filefd;
int maxfdp1;
fd_set rset;
struct sockaddr_in servaddr;
string input, state = "greet", cmd, username, article, id, reply, filename;
vector<string> tok;

void greet() {
    system("clear");
    puts("*************Welcome*****************");
    puts("[R]egister [L]ogin");
}

void panel() {
    system("clear");
    printf("*************Hello %s*****************\n", username.c_str());
    puts("[SU]Show User [SA]Show Article [A]dd Article");
    puts("[Y]ell [T]ell [LO]gout [D]elete Account");
    puts("[DI]ctionary");
}

void show_article_list(string input) {
    system("clear");
    printf("*************Article List*****************\n");
    puts("[E]nter Article [DA]Delete Article [B]ack");
    stringstream ss;
    ss << input;
    getline(ss, input);
    while (getline(ss, input))
        cout << input << endl;
    puts("");
}

void show_article(string input) {
    system("clear");
    printf("*************Article*****************\n");
    puts("[UP]load [DO]wnload [RE]sponse [B]ack");
    stringstream ss;
    ss << input;
    getline(ss, input);
    while (getline(ss, input))
        cout << input << endl;
    puts("");
}

void dictionary(string input) {
    system("clear");
    printf("*************Dictionary*****************\n");
    stringstream ss;
    ss << input;
    getline(ss, input);
    getline(ss, input);
    getline(ss, input);
    while (getline(ss, input))
        cout << input << endl;
    puts("\n[B]ack");
}

void service(string input) {
    log(input.c_str());
    tok = strtok(input);
    cmd = "";
    if (!tok.size()) {
        return;
    }
    char acc[MAXLINE], pwd[MAXLINE], title[MAXLINE], content[MAXLINE];
    // Client -> Server
    if (tok[0] == "L") {
        printf("Account: "); get(acc);
        printf("Password: "); get(pwd);
        cmd = strfmt("L %s %s", acc, pwd);
    } else if (tok[0] == "R") {
        printf("Account: "); get(acc);
        printf("Password: "); get(pwd);
        cmd = strfmt("R %s %s", acc, pwd);
    } else if (tok[0] == "SU") {
        cmd = "SU";
    } else if (tok[0] == "SA") {
        cmd = "SA";
    } else if (tok[0] == "A") {
        printf("Title: "); get(title);
        printf("Content: "); get(content);
        cmd = strfmt("A \n%s\n%s\n%s", username.c_str(), title, content);
    } else if (tok[0] == "E") {
        cmd = "E " + tok[1];
        id = tok[1];
    } else if (tok[0] == "DA") {
        cmd = "DA " + tok[1] + " " + username;
    } else if (tok[0] == "Y") {
        article = get_article(1, input);
        cmd = strfmt("Y %s %s", username.c_str(), article.c_str());
    } else if (tok[0] == "T") {
        article = get_article(2, input);
        cmd = strfmt("T %s %s %s", username.c_str(), tok[1].c_str(), article.c_str());
    } else if (tok[0] == "LO") {
        cmd = "LO " + username;
    } else if (tok[0] == "D") {
        cmd = "D " + username;
    } else if (tok[0] == "B") {
        panel();
    } else if (tok[0] == "RE") {
        cout << "Reply: "; getline(cin, reply);
        cmd = strfmt("RE %s %s %s", username.c_str(), id.c_str(), reply.c_str());
    } else if (tok[0] == "AB") {
        cmd = "AB " + id + " " + tok[1];
    } else if (tok[0] == "DB") {
        cmd = "DB " + id + " " + tok[1];
    } else if (tok[0] == "DI") {
        cmd = input;
    } else if (tok[0] == "UP") {
        filename = tok[1];
        if (lstat(filename.c_str(), &filestat) < 0 || (fp = fopen(filename.c_str(), "rb")) == 0) {
            puts("File error.");
            return;
        }
        int sz = filestat.st_size;
        cmd = strfmt("UP %s %s %s %d", id.c_str(), username.c_str(), filename.c_str(), sz);
        send_to_server(sockfd, servaddr, cmd);
        sleep(1);
        int numbytes, totalbytes = 0;
        while (!feof(fp)) {
            numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
            numbytes = sendto(sockfd, buf, numbytes, 0, (SA*)&servaddr, sizeof(servaddr));
            totalbytes += numbytes;
            usleep(TICK);

        }
        printf("Total %d bytes sent.\n", totalbytes);
        fclose(fp);


        cmd = "";
    } else if (tok[0] == "DO") {
        cmd = "DO " + tok[1];
        filename = tok[1];
        send_to_server(sockfd, servaddr, cmd);
        usleep(WAIT);
        memset(recvline, 0, sizeof(recvline));
        recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
        usleep(WAIT);
        if ((fp = fopen(("Download/" + filename).c_str(), "wb")) != NULL) {
            //Receive file
            int totalbytes = atoi(recvline);
            printf("Reciving %d bytes...", totalbytes);
            while (totalbytes > 0) {
                socklen_t len = sizeof(servaddr);
                int numbytes = recvfrom(sockfd, buf, MAXLINE, 0, (SA*)&servaddr, &len);
                ;
                if (numbytes < 0) {
                    puts("File is broken");
                    fclose(fp);
                    exec("rm Download/"+filename);
                    return;
                }
                totalbytes -= numbytes;
                numbytes = fwrite(buf, sizeof(char), numbytes, fp);
            }
            puts("done.");
            fclose(fp);
        }
        return;
        cmd = "";
    }

    // Server -> Client
    else if (tok[0] == "S_L") {
        if (tok[1] == "SUCCESS") {
            username = tok[2];
            panel();
        } else {
            puts("Login fail");
        }
    } else if (tok[0] == "S_R") {
        cout << "Register " << tok[1] << endl;
    }  else if (tok[0] == "S_LO") {
        greet();
    } else if (tok[0] == "S_SU") {
        puts("=====Online User=====");
        for (int i = 1; i < tok.size(); i++)
            cout << tok[i] << endl;
    } else if (tok[0] == "S_T") {
        article = get_article(1, input);
        cout << "|" << article << endl;
    } else if (tok[0] == "S_A") {

    } else if (tok[0] == "S_SA") {
        show_article_list(input);
    } else if (tok[0] == "S_EA") {
        show_article(input);
    } else if (tok[0] == "S_DI") {
        dictionary(input);
    } else if (tok[0] == "OK") {
        log("OK");
        return;
    } else {
        cout << "Wrong command: " << input << endl;
        return;
    }
    if (cmd != "")
        send_to_server(sockfd, servaddr, cmd);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Usage: ./HW2_101062142_Cli [IP] [port]");
        return 0;
    }
    exec("mkdir Download");
    sockfd = udp_cli(servaddr, argv[1], atoi(argv[2]));
    filefd = fileno(stdin);

    FD_ZERO(&rset);
    greet();
    while (1) {
        FD_SET(filefd, &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(filefd, sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, 0);
        if (FD_ISSET(sockfd, &rset)) {
            /* socket is readable */
            log("WRITE");
            int n;
            if ((n = recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL)) == 0) {
                printf("str_cli: server terminated prematurely");
                return 0;
            }
            recvline[n] = 0;
            service(recvline);
        }
        if (FD_ISSET(filefd, &rset)) {
            /* input is readable */
            log("READ");
            getline(cin, input);
            service(input);
        }
    }

    return 0;
}
