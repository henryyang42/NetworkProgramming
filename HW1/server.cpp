#include "HW1.h"
vector<string> files;
string server_path = "", temp_path;

string server_cmd(string cmd_str) {
    string cmd[10];
    for (int i = 0; i < 10; i++)
        cmd[i] = "";
    int tok_ct = 0;
    stringstream ss;
    ss << cmd_str;
    while (ss >> cmd[tok_ct++]);

    if (cmd[0] == "exit") {

    } else if (cmd[0] == "put") {

    } else if (cmd[0] == "get") {

    } else if (cmd[0] == "cd") {
        if (cmd[1] == "")
            cmd[1] = ".";
        temp_path = server_path + cmd[1];
        files = get_dir(temp_path.c_str());
        if (files.size()) {
            server_path += cmd[1] + (cmd[1][cmd[1].length()-1] == '/' ? "" : "/");
            return exec("echo $(cd " + server_path + "; pwd)");
        } else {
            return "Invalud path.";
        }
    } else if (cmd[0] == "ls") {
        if (cmd[1] == "")
            cmd[1] = ".";
        temp_path = server_path + cmd[1];
        files = get_dir(temp_path.c_str());
        return files2string(files);
    }
    return "Server error";
}


void str_echo(int sockfd, sockaddr_in addr) {
    ssize_t n;
    char buf[MAXLINE];
again:
    while ((n = read(sockfd, buf, MAXLINE)) > 0) {
        printf("GET: %s\n", buf);
        string resp = server_cmd(string(buf));
        fflush(stdout);
        write(sockfd, resp.c_str(), resp.length());
        memset(buf, 0, sizeof(buf));
    }
    if (n < 0 && errno == EINTR)
        goto again;
    else if (n < 0)
        puts("str_echo: read error");

    printf("EXIT-> ");
    print_ip_port(addr);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, client_addr;
    char buff[MAXLINE];

    if (argc != 2) {
        puts("Usage: ./HW1_101062142_Ser port");
        return 0;
    }

    mkdir(UPLOAD, S_IRWXU);
    listenfd = listen2fd(servaddr, atoi(argv[1]));

    while (1) {
        int addr_len = sizeof(client_addr);
        connfd = accept(listenfd, (SA *)&client_addr, (socklen_t *)&addr_len);
        printf("CONNECT-> ");
        print_ip_port(client_addr);
        if (fork() == 0) { /* child process */
            close(listenfd); /* close listening socket */
            str_echo(connfd, client_addr); /* process the request */
            exit (0);
        }
        close(connfd);
        /* parent closes connected socket */
    }
    return 0;
}
