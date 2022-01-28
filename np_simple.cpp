#include <iostream>
#include <netinet/in.h>
#include "npshell.cpp"
using namespace std;

#define MAXCLIENTNUM 1

int main (int argc, char *argv[]) {
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    int addressLen = 0; // length of client's address 
    int msock = 0; // master server socket 
    int ssock = 0; // slave server socket 
    int optionValue = 1;
    int portNumber;
    pid_t pid;

    if (argc != 2){
        cerr << "Use ./np_simple <port number>" << endl;
        return 1;
    }
    portNumber = atoi(argv[1]);

    // Open a TCP socket (an Internet stream socket)
    if ((msock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "server: can't open stream socket" << endl;
    }    
    setsockopt(msock ,SOL_SOCKET ,SO_REUSEADDR,(const char*)&optionValue,sizeof(int));
    setsockopt(msock ,SOL_SOCKET ,SO_REUSEPORT,(const char*)&optionValue,sizeof(int));
    bzero((char *) &serverAddress, sizeof(serverAddress)); 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY); 
    serverAddress.sin_port = htons(portNumber);
    
    // Bind msock with address
    if (bind(msock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        cout << "server: can't bind local address" << endl;
    }
    // Listen to socket
    listen(msock, MAXCLIENTNUM);

    // Execute npShell
    while(true) {
        addressLen = sizeof(clientAddress); 
        ssock = accept(msock, (struct sockaddr *) &clientAddress, (socklen_t *) &addressLen); 
        if(ssock < 0) {
            cout << "server: accept error" << endl; 
        }        
        if((pid = fork()) < 0) {
            cout << "server: fork error" << endl;
        }
        // child process
        else if(pid == 0) {
            dup2(ssock, STDIN_FILENO);
            dup2(ssock, STDOUT_FILENO);
            dup2(ssock, STDERR_FILENO);
            close(ssock);
            close(msock);
            executeNPshell();
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            exit(0);
        } 
        // parent process
        else {
            close(ssock);
            int status;
            waitpid(pid, &status, 0);
        } 
    }
    return 0;
}