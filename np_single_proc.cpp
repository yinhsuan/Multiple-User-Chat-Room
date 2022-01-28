#include <iostream>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h> 
#include <netinet/in.h>
#include <unistd.h> 
#include <string.h> 
#include <errno.h>
// #include <stdio.h>
#include <map>
// #include "ShellPipe.cpp"
#include "ClientShell.cpp"
using namespace std;

#define MAX_CLIENT_NUM 30
// #define QLEN    5 // maximum connection queue length
// #define BUFSIZE 4096
// extern int errno;
// int errexit(const char *format, ...); 
// int passiveTCP(const char *service, int qlen); 
// int echo(int fd);

int msock = 0;  // master server socket
int ssock = 0; // slave server socket 

void childHandler(int signo){
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void sigINT(int signo){
    close(msock);
    cout << "sigINT" << endl;
    exit(0);
}

int main (int argc, char *argv[]) {
    // char *serviceName = "echo"; // service name or port number 
    struct sockaddr_in clientAddress; // the from address of a client 
    struct sockaddr_in serverAddress;
    int addressLen = 0; // length of client's address 
    
    fd_set rfds; 
    fd_set afds; 
    int nfds = 0; // number of select
    int portNumber;  
    map<pair<int,int>, ShellPipe> clientPipe; // key: pair<fromUser#, toUser#>
    // map<int, ClientShell> clientList; // key: id
    ClientShell *clientList = new ClientShell[MAX_CLIENT_NUM];
    // npshell_class *ClientNp = new npshell_class[MAX_USER_NUM];
    ClientInfo cInfo;
    int result;
    struct timeval timeout;

    signal(SIGINT,sigINT);
    signal(SIGCHLD,childHandler);

    // int alen;
    // int fd, nfds;
    if (argc != 2){
        cerr << "Use ./np_simple <port number>" << endl;
        return 1;
    }
    portNumber = atoi(argv[1]);

    // Open a TCP socket (an Internet stream socket)
    if ((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "server: can't open stream socket" << endl;
    }    
    int optionValue = 1;
    setsockopt(msock, SOL_SOCKET, SO_REUSEADDR, (const char *) &optionValue, sizeof(int));
    setsockopt(msock, SOL_SOCKET, SO_REUSEPORT, (const char *) &optionValue, sizeof(int));
    bzero((char *) &serverAddress, sizeof(serverAddress)); 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
    serverAddress.sin_port = htons(portNumber);
    
    // Bind msock with address
    if (bind(msock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        cout << "server: can't bind local address" << endl;
    }
    // Listen to socket
    listen(msock, MAX_CLIENT_NUM);

    // Execute single-process concurrent
    nfds = getdtablesize();
    FD_ZERO(&afds);
    FD_SET(msock, &afds);
    while(true) {
        timeout.tv_sec = 0; 
        timeout.tv_usec = 500000;
        memcpy(&rfds, &afds, sizeof(rfds));
        // check ON/OFF of client
        do {
            errno = 0;
            result = select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0);
        } while(result == -1 && errno == EINTR);

        if (result == -1){
            //   for (int i=3; i<=33; i++){
            //     errno = 0;
            //     FD_ZERO(&rfds);
            //     FD_SET(i, &rfds);
            //     int resultTmp = select(nfds, &rfds, (fd_set *)0, (fd_set *)0, &timeout);
            //     cout << i << " " << resultTmp << strerror(errno) << endl;
            //   }
            cout << "select: " << strerror(errno) << endl;
        }
        else if (result > 0) {

            // if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) < 0) {
            //     cout << "select: " << strerror(errno) << endl;
            // }
            // accept new client
            if (FD_ISSET(msock, &rfds)) {
                addressLen = sizeof(clientAddress);
                // cout << "cp 1" << endl;
                ssock = accept(msock, (struct sockaddr *) &clientAddress, (socklen_t *) &addressLen); 
                // cout << "cp 2" << endl;
                if(ssock < 0) {
                    cout << "server: accept error" << endl; 
                }
                FD_SET(ssock, &afds);

                // add new client to clientList
                for (int cId=0; cId<MAX_CLIENT_NUM; cId++) {
                    // cout << "cp 3" << endl;
                    cInfo = clientList[cId].getClientInfo();
                    if (!cInfo.getIsActive()) {
                        clientList[cId].clientLogIn(cId+1, clientAddress, clientList, ssock, MAX_CLIENT_NUM);
                        break;
                    }
                }
            }
            // read from client
            for (int cId=0; cId<MAX_CLIENT_NUM; cId++) {
                cInfo = clientList[cId].getClientInfo();
                if (cInfo.getIsActive()) {
                    // cout << "cp 4" << endl;
                    int thisFD = clientList[cId].getClientInfo().getFD();
                    // whether thisFD is on (by rfds) => input from terminal
                    if (FD_ISSET(thisFD, &rfds)) {
                        // cout << "Before recv" << endl;
                        char buffer[15000];
                        int cmdLen = recv(thisFD, buffer, sizeof(buffer), 0);
                        buffer[cmdLen] = '\0';
                        if (cmdLen >= 2) {
                            if (buffer[cmdLen-2] == '\r') {
                                buffer[cmdLen-2] = '\0';
                            }
                        }
                        else if (cmdLen == 1) {
                            if (buffer[cmdLen-1] == '\n') {
                                buffer[cmdLen-1] = '\0';
                            }
                        }
                        string cmd = string(buffer);
                        // cout << "After recv" << endl;


                        // char buffer[100];
                        // int t = recv(this_fd,buffer,sizeof(buffer),0);
                        // buffer[t] = '\0';
                        // if (t >= 2){
                        //     if (buffer[t-2] == '\r') buffer[t-2] = '\0';
                        // }
                        // else if (t == 1){
                        //     if (buffer[t-1] == '\n') buffer[t-1] = '\0';
                        // }
                        // cout << i+1 << buffer << endl;
                        // string tt = string(buffer);

                        ProcessMode ps = clientList[cId].executeShell(cmd, clientList, &clientPipe, MAX_CLIENT_NUM);
                        // cout << "clientPipe size at single: " << clientPipe.size() << endl;
                        if (ps == ToExit) {
                            clientList[cId].clientLogOut(clientList, &clientPipe, MAX_CLIENT_NUM);
                            FD_CLR(thisFD, &afds);
                        }
                    }
                }
            }


        }



        

             

    }
    return 0;
}