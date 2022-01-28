#include <iostream>
#include <netinet/in.h>
// #include <sys/ipc.h>

#include <sys/shm.h>
#include "MultiClientShell.cpp"
using namespace std;

// #define MAXCLIENTNUM 30

int msock = 0; // master server socket 
int ssock = 0; // slave server socket 
int newClientID = -1;
MultiClientInfo *clientList; // from share memory


void childHandler(int signo){
    int status;
    int pid = wait(&status);
    for (int i=0; i<MAXCLIENTNUM; i++) {
        if (clientList[i].getPid() == pid) {
            int userid = clientList[i].getID();
            close(clientList[userid-1].getFD());
            clientList[userid-1].setIsActive(false);
        }
    }
    while (waitpid(-1, &status, WNOHANG) > 0);
}

void sigExitHandler(int signo){
    int shmid1 = shmget((key_t)IPC_PRIVATE, sizeof(MultiClientInfo)*MAXCLIENTNUM, IPC_CREAT|IPC_EXCL|0600);
    shmctl(shmid1, IPC_RMID, 0);

    for (int i=0; i<MAXCLIENTNUM; i++){
        if (clientList[i].getIsActive()) {
            close(clientList[i].getFD());
        } 
    }
    close(msock);
    cout << "sigINT" << endl;
    exit(0);
}

void clientMsgHandler(int signo) {
    // cout << "Enter clientPrintMsg" << endl;
    // if (send(ssock, msg, strlen(msg), 0) < 0) {
    //     cout << "send msg failed" << endl;
    // }
    // for (int i=0; i<MAXCLIENTNUM; i++) {
        // cout << "clientMsgHandler" << endl;
        if (clientList[newClientID-1].getIsActive() && clientList[newClientID-1].getIsClientPrintMsg()) {
            cout << clientList[newClientID-1].getMsg();
            //write(clientList[newClientID-1].getFD(), clientList[newClientID-1].getMsg(), strlen(clientList[newClientID-1].getMsg()));
            clientList[newClientID-1].setIsClientPrintMsg(false);
        }
    // }

}

// int thisId;
void openReadFifo(int signo) {
    // cout << "enter openReadFifo" << endl;
    int fromClientNo;
    int toClientNo;
    for (int i=0; i<MAXCLIENTNUM; i++) {
        // if (clientList[i].getFromClientPipeNo() == thisId) {
        //     fromClientNo = thisId;
        //     toClientNo = clientList[i].getID();
        //     break;
        // }
        if (clientList[i].getIsFromClientPipe() && !clientList[i].getIsOpenRead()) {
            fromClientNo = clientList[i].getFromClientPipeNo();
            toClientNo = clientList[i].getID();
            clientList[i].setIsOpenRead(true);
            // clientList[i].setIsFromClientPipe(false);
            break;
        }
    }
    // cout << "openRead fromClientNo: " << fromClientNo << endl;
    // cout << "openRead toClientNo: " << toClientNo << endl;
    char fifopR[30];
    sprintf(fifopR, "./user_pipe/%dto%d", fromClientNo, toClientNo);
    mkfifo(fifopR, 0666);
    clientList[toClientNo-1].setFromClientFD(open(fifopR, O_RDONLY));
    // cout << "open R: " << clientList[toClientNo-1].getFromClientFD() << endl;    
}



void initialShareMemory() {
    MultiClientInfo *loc;
    int shmid1 = shmget((key_t)IPC_PRIVATE, sizeof(MultiClientInfo)*MAXCLIENTNUM, IPC_CREAT|IPC_EXCL|0600);
    loc = (MultiClientInfo*)shmat(shmid1, 0, 0);
    clientList = new (loc) MultiClientInfo[MAXCLIENTNUM];

    // for (int i=0; i<MAXCLIENTNUM; i++){
    //     clientList[i].setIsActive(false);
    // }
}

// void setNewClient(int newClientID, sockaddr_in info){
//     clientList[newClientID-1].setName("(no name)");
//     clientList[newClientID-1].setPort(ntohs(info.sin_port));
//     clientList[newClientID-1].setIP(inet_ntoa(info.sin_addr));
//     clientList[newClientID-1].setID(newClientID);
//     clientList[newClientID-1].setFD(ssock);
// }

int main (int argc, char *argv[]) {
    struct sockaddr_in clientAddress;
    struct sockaddr_in serverAddress;
    int addressLen = 0; // length of client's address     
    int optionValue = 1;
    int portNumber;
    pid_t pid;
    MultiClientShell client;
    
    

    signal(SIGINT,sigExitHandler);
    signal(SIGCHLD,childHandler);
    signal(SIGUSR1, clientMsgHandler);
    signal(SIGUSR2, openReadFifo);
    

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

    // Open Share Memory
    initialShareMemory();

    // Execute npShell
    while(true) {        
        addressLen = sizeof(clientAddress); 
        cout << "To Listen" << endl;
        ssock = accept(msock, (struct sockaddr *) &clientAddress, (socklen_t *) &addressLen); 
        if(ssock < 0) {
            cout << "server: accept error" << endl; 
        }        

        // find available client id
        for (int i=0; i<MAXCLIENTNUM; i++) {
            if (!clientList[i].getIsActive()) {
                newClientID = i+1;
                break;
                // clientList[i].setIsActive(true);
            }
        }
        // setNewClient(newClientID, clientAddress);

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
            // executeNPshell();
            client.executeShell(newClientID, clientAddress, ssock, clientList, MAXCLIENTNUM);
            // close(STDIN_FILENO);
            // close(STDOUT_FILENO);
            // close(STDERR_FILENO);
            exit(0);
        } 
        // parent process
        else {
            // close(ssock);
            clientList[newClientID-1].setPid((int)pid);
            // clientList[newClientID-1].setFD(ssock);

            // user_pid[(int)pid] = newid;
            // clientList[newClientID-1].setPid((int)pid);
            // clientList[newClientID-1].setFD(ssock);
            int status;
            waitpid(pid, &status, WNOHANG);
        } 
    }
    return 0;
}