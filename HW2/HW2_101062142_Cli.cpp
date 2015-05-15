#include "HW2.h"
char recvline[MAXLINE + 1];
int sockfd, filefd;
int maxfdp1;
fd_set rset;
struct sockaddr_in servaddr;
string input, state = "greet", cmd, username;
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
    puts("[E]nter Article [Y]ell [T]ell [LO]gout");
}

void service(string input) {
    cout << input << endl;
    tok = strtok(input);
    cmd = "";
    if(!tok.size()) {
        return;
    }
    char acc[MAXLINE], pwd[MAXLINE];
    // Client -> Server
    if(tok[0] == "L") {
        printf("Account: "); get(acc);
        printf("Password: "); get(pwd);
        cmd = strfmt("L %s %s", acc, pwd);
    } else if(tok[0] == "R") {
        printf("Account: "); get(acc);
        printf("Password: "); get(pwd);
        cmd = strfmt("R %s %s", acc, pwd);
    } else if(tok[0] == "SU") {
        cmd = "SU";
    } else if(tok[0] == "SA") {

    } else if(tok[0] == "A") {

    } else if(tok[0] == "E") {

    } else if(tok[0] == "Y") {

    } else if(tok[0] == "T") {

    } else if(tok[0] == "LO") {
        cmd = "LO " + username;
    }
    // Server -> Client
    else if(tok[0] == "S_L") {
        if(tok[1] == "SUCCESS") {
            username = tok[2];
            panel();
        }
    } else if(tok[0] == "S_R") {
        cout << "Register " << tok[1] << endl;
    }  else if(tok[0] == "S_LO") {
        greet();
    } else if(tok[0] == "S_SU") {
        puts("=====Online User=====");
        for(int i = 1; i < tok.size(); i++)
            cout << tok[i] << endl;
    } else if(tok[0] == "qaq") {

    } else if(tok[0] == "qaq") {

    }else {
        cout << "Wrong command: " << input << endl;
        return;
    }
    if(cmd != "")
        send_to_server(sockfd, servaddr, cmd);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Usage: ./HW2_101062142_Cli [IP] [port]");
        return 0;
    }

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
            log("socket fd");
            int n;
            if ((n = read(sockfd, recvline, MAXLINE)) == 0) {
                printf("str_cli: server terminated prematurely");
                return 0;
            }
            recvline[n] = 0;
            service(recvline);
        }
        if (FD_ISSET(filefd, &rset)) {
            /* input is readable */
            log("file fd");
            getline(cin, input);
            service(input);
        }
    }

    return 0;
}
