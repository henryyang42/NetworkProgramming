#include "HW1.h"

int connect2fd(struct sockaddr_in &addr, char *ip, int port) {
    int sockfd;
    bzero(&addr, sizeof(addr));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("socket error");
        exit(0);
    }
    // IPV4
    addr.sin_family = AF_INET;
    // IP
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
        printf("inet_ption error for %s\n", ip);
    // Port
    addr.sin_port = htons(port);

    if (connect(sockfd, (SA *) &addr, sizeof(addr)) < 0)
        printf("connect error");

    return sockfd;
}

int listen2fd(struct sockaddr_in &addr, int port) {
    int sockfd;
    bzero(&addr, sizeof(addr));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("socket error");
        exit(0);
    }
    // IPV4
    addr.sin_family = AF_INET;
    // IP
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Port
    addr.sin_port = htons(port);

    if (bind(sockfd, (SA *)&addr, sizeof(addr)) < 0)
        puts("bind error");
    listen(sockfd, LISTENQ);

    return sockfd;
}

void print_ip_port(struct sockaddr_in &addr) {
    printf("IP address: %s, ", inet_ntoa(addr.sin_addr));
    printf("port: %d\n", (int) ntohs(addr.sin_port));
}

string files2string(vector<string> files) {
    if (files.size()) {
        stringstream buffer;
        for (int i = 2; i < files.size(); i++) {
            buffer.width(18);
            buffer << left << files[i];
            if (i != 2 && i % 5 == 2)
                buffer << "\n";
        }
        return buffer.str()+"\n";
    } else {
        return "Path not found.\n";
    }
}

vector<string> get_dir(const char *path) {
    vector<string> files;
    DIR           *d;
    struct dirent *dir;
    d = opendir(path);
    if (strlen(path) == 0)
        d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            files.push_back(string(dir->d_name));
        }
        closedir(d);
    }
    return files;
}

string exec(string cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "ERROR";
    char buffer[128];
    std::string result = "";
    while(!feof(pipe)) {
        if(fgets(buffer, 128, pipe) != NULL)
            result += buffer;
    }
    pclose(pipe);
    return result;
}
