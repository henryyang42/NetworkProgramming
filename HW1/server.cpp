#include "HW1.h"
vector<string> files;
string server_path = "", temp_path;

struct stat filestat;
FILE *fp;
char buf[MAXLINE];
int numbytes, totalbytes;
string resp, filename;

string server_cmd(string cmd_str, int sockfd) {
    string cmd[10];
    for (int i = 0; i < 10; i++)
        cmd[i] = "";
    int tok_ct = 0;
    stringstream ss;
    ss << cmd_str;
    while (ss >> cmd[tok_ct++]);

    if (cmd[0] == "exit") {
        return "Bye";
    } else if (cmd[0] == "put") {
        if ( (fp = fopen((string(UPLOAD) + "/" + cmd[1]).c_str(), "wb")) == NULL) {
            return "Server file error.";
        }
        resp = "Server reciving file...";
        write(sockfd, resp.c_str(), resp.length());
        //Receive file
        totalbytes = atoi(cmd[2].c_str());
        printf("Reciving %d bytes...", totalbytes);
        while (totalbytes > 0) {
            numbytes = read(sockfd, buf, sizeof(buf));
            totalbytes -= numbytes;
            numbytes = fwrite(buf, sizeof(char), numbytes, fp);
        }
        puts("done.");
        fclose(fp);
        return "Transfer file " + cmd[1] + " done.";
    } else if (cmd[0] == "get") {
        filename = server_path + cmd[1];
        if (lstat(filename.c_str(), &filestat) < 0 || (fp = fopen(filename.c_str(), "rb")) == 0) {
            resp = "Server file error.";
            write(sockfd, resp.c_str(), resp.length());
            usleep(100);
            return "-1";
        }
        resp = "Server transfering file...";
        write(sockfd, resp.c_str(), resp.length());
        usleep(100);
        resp = to_string(filestat.st_size);
        write(sockfd, resp.c_str(), resp.length());
        usleep(100);
        totalbytes = 0;
        while (!feof(fp)) {
            numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
            numbytes = write(sockfd, buf, numbytes);
            totalbytes += numbytes;
        }
        printf("Total %d bytes sent.\n", totalbytes);
        fclose(fp);
        return "Transfer file " + cmd[1] + " done.";
    } else if (cmd[0] == "cd") {
        if (cmd[1] == "")
            cmd[1] = ".";
        temp_path = server_path + cmd[1];
        files = get_dir(temp_path.c_str());
        if (files.size()) {
            server_path += cmd[1] + (cmd[1][cmd[1].length() - 1] == '/' ? "" : "/");
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
        resp = server_cmd(string(buf), sockfd);
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
        puts("Usage: ./HW1_101062142_Ser [port]");
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
