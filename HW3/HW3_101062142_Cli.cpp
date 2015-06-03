#include "HW3.h"
char recvline[MAXLINE];
struct stat filestat;
FILE *fp;
char buf[MAXLINE];
int sockfd, filefd;
int maxfdp1, serv_port;
fd_set rset;
struct sockaddr_in servaddr;
string input, state = "greet", cmd, username, article, id, reply, filename, serv_ip;
vector<string> tok;
map<string, pair<string, int> >ip_port;
map<string, vector<string> >file_archive;
map<string, vector<string> >::iterator fa_it;
struct SF {
    int part;
    int tot_part;
    string filename;
    string ip;
    int port;
} sf_get, sf_send;

void *send_file(void *ptr) {
    log("SEND FILE");
    struct sockaddr_in addr, client_addr;
    struct stat filestat;
    char sendline[MAXLINE] = {};
    SF sf = *((SF *) ptr);
    FILE *fp;
    fp = fopen(("Upload/" + sf.filename).c_str(), "rb");
    lstat(("Upload/" + sf.filename).c_str(), &filestat);
    long long file_sz = filestat.st_size, offset = 0;
    sf.port += sf.part + 1;
    int listenfd = listen2fd(addr, sf.port);
    int addr_len = sizeof(client_addr);
    int connfd = accept(listenfd, (SA *)&client_addr, (socklen_t *)&addr_len);
    char *file = (char*)malloc(sizeof(char) * file_sz);
    fread(file, sizeof(char), file_sz, fp);
    fclose(fp);
    if (sf.tot_part) {
        offset = (file_sz * sf.part) / sf.tot_part;
        file_sz = (file_sz * (sf.part + 1)) / sf.tot_part - offset;
    }
    sprintf(sendline, "%lld", file_sz);
    log(sendline);
    write(connfd, sendline, strlen(sendline));
    read(connfd, sendline, MAXLINE);

    while (file_sz > 0) {
        int numbytes = min((long long)MAXLINE, file_sz);
        memcpy(sendline, file + offset, numbytes);
        offset += numbytes;
        file_sz -= numbytes;
        printf("%lld\n", file_sz);
        numbytes = write(connfd, sendline, numbytes);
        memset(sendline, 0, sizeof(sendline));
        usleep(1);
    }

    log("END SEND FILE");
    close(connfd);
    close(listenfd);
    free(file);
    printf("Upload %s finished.\n", sf.filename.c_str());
    fflush(stdout);
    return NULL;
}

void *get_file(void *ptr) {
    log("GET FILE");
    usleep(50000);
    char recvline[MAXLINE] = {};
    struct sockaddr_in addr;
    SF sf = *((SF *) ptr);
    FILE *fp;
    fp = fopen(("Download/" + sf.filename).c_str(), "wb");
    sf.port += sf.part + 1;
    int connfd = connect2fd(addr, sf.ip.c_str(), sf.port), yes = 1;
    read(connfd, recvline, MAXLINE);
    write(connfd, recvline, strlen(recvline));
    int file_sz = atoi(recvline);
    while (file_sz > 0) {
        int numbytes = read(connfd, recvline, sizeof(recvline));
        fwrite(recvline, sizeof(char), numbytes, fp);
        memset(recvline, 0, sizeof(recvline));
        file_sz -= numbytes;
    }

    log("END GET FILE");
    fclose(fp);
    close(connfd);
    printf("Download %s finished.\n", sf.filename.c_str());
    fflush(stdout);
    return NULL;
}

void ip_port_send(string ip, int port, string msg) {
    struct sockaddr_in addr;
    int fd = udp_cli(addr, ip.c_str(), port);
    int n;
    const char *sendline = msg.c_str();
    char recvline[MAXLINE + 1];
    sendto(fd, sendline, strlen(sendline), 0, (SA*)&addr, sizeof(addr));
}

string gl() {
    char recvline[MAXLINE] = {};
    recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);
    return string(recvline);
}
void sd(string s) {
    sendto(sockfd, s.c_str(), strlen(s.c_str()), 0, (SA*)&servaddr, sizeof(servaddr));
    gl();
}
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
    puts("[DI]ctionary [U]pload [D]ownload [PD]Parallel Download");
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

void *get_file_list(void *ptr) {
    while (1) {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir ("Upload/")) != NULL) {
            /* print all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) {
                string filename(ent->d_name);
                log(ent->d_name);
                ip_port_send(serv_ip, serv_port, "FR " + username + " " + filename);
            }
            closedir (dir);
        } else {
            /* could not open directory */
            perror ("");
        }
        sleep(10);
    }
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
    if (tok[0] == "PD") {
        pthread_t t1[10];
        string fn = tok[1];
        SF sf_get[10];
        int SS = 10;
        vector<string>ns;
        for(int i = 0; i < file_archive[fn].size(); i++){
            if(file_archive[fn][i] != username)
                ns.push_back(file_archive[fn][i]);
        }
        if (ns.size() == 0) {
            puts("No one has that file!");
            return;
        }
        for (int i = 0; i < SS; i++) {
            string name= ns[i%ns.size()];
            cmd = strfmt("S_D %s %s %d %d", username.c_str(), fn.c_str(), i, SS);
            cout << cmd << endl;
            ip_port_send(ip_port[name].first, ip_port[name].second, cmd);
            printf("Download %s %d/%d from %s\n", fn.c_str(), i+1, SS, name.c_str());
            sf_get[i].filename = strfmt("%s.%d", fn.c_str(), i);
            sf_get[i].tot_part = SS;
            sf_get[i].part = i;
            sf_get[i].ip = ip_port[name].first;
            sf_get[i].port = 1234;
            pthread_create(&t1[i], NULL, get_file, (void*)&sf_get[i]);
        }
        cmd = "";
        FILE *fp = fopen(("Download/" + fn).c_str(), "wb");
        char buf[MAXLINE];
        for (int i = 0; i < SS; i++) {
            string filename = "Download/" + strfmt("%s.%d", fn.c_str(), i);
            pthread_join(t1[i], NULL);
            FILE *ff = fopen(filename.c_str(), "rb");
            while (!feof(ff)) {
                int numbytes = fread(buf, sizeof(char), sizeof(buf), ff);
                fwrite(buf, sizeof(char), numbytes, fp);
            }
            fclose(ff);
            exec("rm " + filename);
        }
        fclose(fp);
    } else if (tok[0] == "U") {
        cmd = "D " + username + " " + tok[2];
        if (ip_port.find(tok[1]) != ip_port.end()) {
            ip_port_send(ip_port[tok[1]].first, ip_port[tok[1]].second, cmd);
        }
        cmd = "";
    } else if (tok[0] == "D") {
        cmd = "S_D " + username + " " + tok[2];
        if (ip_port.find(tok[1]) != ip_port.end()) {
            ip_port_send(ip_port[tok[1]].first, ip_port[tok[1]].second, cmd);
            sf_get.ip = ip_port[tok[1]].first;
            sf_get.port = 1234;
            sf_get.filename = tok[2];
            pthread_t t1;
            pthread_create(&t1, NULL, get_file, (void*)&sf_get);
        }
        cmd = "";
    } else if (tok[0] == "S_D") {
        if (ip_port.find(tok[1]) != ip_port.end()) {
            SF sf_send;
            sf_send.ip = ip_port[tok[1]].first;
            sf_send.port = 1234;
            sf_send.part = sf_send.tot_part = 0;
            if(tok.size() >= 5){
                sf_send.part = atoi(tok[3].c_str());
                sf_send.tot_part = atoi(tok[4].c_str());
            }
            sf_send.filename = tok[2];
            pthread_t t1;
            pthread_create(&t1, NULL, send_file, (void*)&sf_send);
        } else {
            log("name GG");
        }
    } else if (tok[0] == "SF") {
        cmd = "SF";
    } else if (tok[0] == "T") {
        article = get_article(2, input);
        cmd = strfmt("S_T %s %s", username.c_str(), article.c_str());
        if (ip_port.find(tok[1]) != ip_port.end()) {
            ip_port_send(ip_port[tok[1]].first, ip_port[tok[1]].second, cmd);
        }
        cmd = "";
    } else if (tok[0] == "L") {
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
    }  else if (tok[0] == "LO") {
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
    }
    // Server -> Client
    else if (tok[0] == "S_SF") {
        stringstream ss(input);
        getline(ss, input);
        file_archive.clear();
        while(getline(ss, input)){
            cout << input << endl;
            tok = strtok(input);
            for(int i = 1; i < tok.size(); i++)
                file_archive[tok[0]].push_back(tok[i]);
        }
    }
    else if (tok[0] == "S_L") {
        if (tok[1] == "SUCCESS") {
            username = tok[2];
            pthread_t file_lookup_thread;
            pthread_create(&file_lookup_thread, NULL, get_file_list, NULL);
            panel();
        } else {
            puts("Login fail");
        }
    } else if (tok[0] == "S_SU") {
        ip_port.clear();
        puts("=====Online User=====");
        for (int i = 1; i < tok.size(); i += 3) {
            cout << strfmt("%s %s %s", tok[i].c_str(), tok[i + 1].c_str(), tok[i + 2].c_str()) << endl;
            ip_port[tok[i]] = make_pair(tok[i + 1], atoi(tok[i + 2].c_str()));
        }
    } else if (tok[0] == "S_R") {
        cout << "Register " << tok[1] << endl;
    }  else if (tok[0] == "S_LO") {
        greet();
    } else if (tok[0] == "S_T") {
        article = get_article(1, input);
        cout << "|" << article << endl;
    } else if (tok[0] == "S_SA") {
        show_article_list(input);
    } else if (tok[0] == "S_EA") {
        show_article(input);
    } else if (tok[0] == "S_DI") {
        dictionary(input);
    } else if (tok[0] == "OK" || tok[0] == "ACK") {
        log("OK");
        return;
    } else {
        log(("Wrong command: " + input).c_str());
        return;
    }
    if (cmd != "") {
        send_to_server(sockfd, servaddr, cmd);
        cmd = "";
    }
}
void test() {
    log("TEST");
    pthread_t t1[10], t2[10];
    string fn; cin >> fn;
    int SS = 10;
    for (int i = 0; i < SS; i++) {
        sf_get.filename = strfmt("%s.%d", fn.c_str(), i);
        sf_send.filename = fn;
        sf_send.ip = sf_get.ip = "127.0.0.1";
        sf_send.port = sf_get.port = 1234;
        sf_send.tot_part = SS;
        sf_get.part = sf_send.part = i;
        pthread_create(&t2[i], NULL, send_file, (void*)&sf_send);
        pthread_create(&t1[i], NULL, get_file, (void*)&sf_get);
        usleep(100000);
    }
    FILE *fp = fopen(("Download/" + fn).c_str(), "wb");
    char buf[MAXLINE];
    for (int i = 0; i < SS; i++) {
        string filename = "Download/" + strfmt("%s.%d", fn.c_str(), i);
        pthread_join(t1[i], NULL);
        FILE *ff = fopen(filename.c_str(), "rb");
        while (!feof(ff)) {
            int numbytes = fread(buf, sizeof(char), sizeof(buf), ff);
            fwrite(buf, sizeof(char), numbytes, fp);
        }
        fclose(ff);
        exec("rm " + filename);
    }
    fclose(fp);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        puts("Usage: ./HW2_101062142_Cli [IP] [port]");
        return 0;
    }
    exec("mkdir Download");
    exec("mkdir Upload");
    sockfd = udp_cli(servaddr, argv[1], atoi(argv[2]));
    serv_ip = string(argv[1]);
    serv_port = atoi(argv[2]);
    filefd = fileno(stdin);

    FD_ZERO(&rset);
    greet();
    //test();
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
