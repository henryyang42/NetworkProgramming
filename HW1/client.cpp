#include "HW1.h"
using namespace std;
int sockfd;
vector<string> files;
string client_path = "", temp_path, filename;

void send_cmd(const char* sendline) {
    char recvline[MAXLINE] = {};
    int len = strlen(sendline);
    write(sockfd, sendline, len);
    if (read(sockfd, recvline, MAXLINE) == 0) {
        printf("str_cli: server terminated prematurely");
        exit(0);
    }

    puts(recvline);
}

void help(){
    puts("put <filename>: upload file to remote");
    puts("get <filename>: download file from remote");
    puts("ls  <path>: list remote file");
    puts("cd  <path>: change remote directory");
    puts("lls <path>: list local file");
    puts("lcd <path>: change local directory");
    puts("clear: clear the terminal screen");
    puts("help: show help info");
    puts("exit: terminate the program");
}

struct stat filestat;
FILE *fp;
char buf[MAXLINE];
int numbytes, totalbytes;
void client_cmd(string cmd_str) {
    string cmd[10];
    for (int i = 0; i < 10; i++)
        cmd[i] = "";
    int tok_ct = 0;
    stringstream ss;
    ss << cmd_str;
    while (ss >> cmd[tok_ct++]);
    if (tok_ct <= 1)
        return;

    if (cmd[0] == "exit") {
        send_cmd((cmd[0]).c_str());
        puts("Exit successfully.");
        exit(0);
    } else if (cmd[0] == "put") {
        filename = client_path + cmd[1];
        if (cmd[1] == "" || lstat(filename.c_str(), &filestat) < 0 || (fp = fopen(filename.c_str(), "rb")) == 0) {
            puts("File error.");
            return;
        }
        send_cmd(("put " + cmd[1] + " " + to_string(filestat.st_size)).c_str());
        totalbytes = 0;
        while (!feof(fp)) {
            numbytes = fread(buf, sizeof(char), sizeof(buf), fp);
            numbytes = write(sockfd, buf, numbytes);
            totalbytes += numbytes;
        }
        printf("Total %d bytes sent.\n", totalbytes);
        fclose(fp);
        memset(buf, 0, sizeof(buf));
        read(sockfd, buf, MAXLINE);
        puts(buf);
    } else if (cmd[0] == "get") {
        send_cmd(cmd_str.c_str());
        memset(buf, 0, sizeof(buf));
        read(sockfd, buf, MAXLINE);
        totalbytes = atoi(buf);
        if (cmd[1] == "" || totalbytes < 0 || (fp = fopen((string(DOWNLOAD) + "/" + cmd[1]).c_str(), "wb")) == NULL) {
            puts("File error.");
            return;
        }
        printf("Reciving %d bytes...", totalbytes);
        while (totalbytes > 0) {
            numbytes = read(sockfd, buf, sizeof(buf));
            totalbytes -= numbytes;
            numbytes = fwrite(buf, sizeof(char), numbytes, fp);
        }
        puts("done.");
        fclose(fp);
        memset(buf, 0, sizeof(buf));
        //read(sockfd, buf, MAXLINE);
        //puts(buf);
    } else if (cmd[0] == "cd") {
        send_cmd(cmd_str.c_str());
    } else if (cmd[0] == "ls") {
        send_cmd(cmd_str.c_str());
    } else if (cmd[0] == "lcd") {
        if (cmd[1] == "")
            cmd[1] = ".";
        temp_path = client_path + cmd[1];
        files = get_dir(temp_path.c_str());
        if (files.size()) {
            client_path += cmd[1] + (cmd[1][cmd[1].length() - 1] == '/' ? "" : "/");
            cout << exec("echo $(cd " + client_path + "; pwd)") << endl;
        }
    } else if (cmd[0] == "lls") {
        if (cmd[1] == "")
            cmd[1] = ".";
        temp_path = client_path + cmd[1];
        files = get_dir(temp_path.c_str());
        cout << files2string(files);
    } else if (cmd[0] == "clear") {
        system("clear");
    } else if (cmd[0] == "help") {
        help();
    } else {
        printf("Command not found.\n");
    }
}

int main (int argc, char **argv) {
    struct sockaddr_in servaddr;
    string cmd_str;
    if (argc != 3) {
        puts("Usage: ./HW1_101062142_Cli [IP] [port]");
        return 0;
    }

    mkdir(DOWNLOAD, S_IRWXU);
    sockfd = connect2fd(servaddr, argv[1], atoi(argv[2]));

    help();

    while (printf(">>> "), getline(cin, cmd_str)) {
        client_cmd(cmd_str);
    }
    return 0;
}
