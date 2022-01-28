#include <iostream>
#include <netinet/in.h>
#include <sstream>
// #include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include<signal.h>
#include <map>
#include <cstring>
#include "MultiClientInfo.cpp"
#include "ShellCommand.cpp"
#include "ShellPipe.cpp"
using namespace std;

#define MAXCLIENTNUM 30

enum ProcessMode {
	NoWait = 0,
	ToWait = 1,
	ToExit = 2,
	ToEnvVar = 3,
    ToChatCmd = 4,
};

class MultiClientShell {
    private:
        // ClientInfo clientInfo;
        // int lineId;
        // map<string, string> clientEnvVar;
        map<int, ShellPipe> lineList; // for number pipe
        map<int, ShellPipe> pipeList; // for line command pipe
        string wholeLineCommand;
        

    public:
        MultiClientShell();
        ~MultiClientShell();

        // ClientInfo getClientInfo();

        // void broadcastToAll();
        // void clientPrintMsg(const char *msg, int ssock);
        void showCommandPrompt();
        void clientYell(const char *msg, MultiClientInfo *clientList, int maxClientNum);

        void executeShell(int newClientID, sockaddr_in clientAddress, int ssock, MultiClientInfo *clientList, int maxClientNum);
        

        void clientLogIn(int id, sockaddr_in clientAddress, MultiClientInfo *clientList, int ssock, int maxClientNum);
        void clientLogOut(int id, MultiClientInfo *clientList, int maxClientNum);

        void exeChatCommand(int id, vector<ShellCommand> cList, MultiClientInfo *clientList, int maxClientNum);
        void setEnvironmentVar(vector<ShellCommand> cList);
        // void signalHandler(int receivedSignal);
        pid_t npshellExeCommand(int id, ShellCommand command, map<int, ShellPipe> *pipelist, map<int, ShellPipe> *lineList, MultiClientInfo *clientList, int maxClientNum);
        ProcessMode npshellSplitLine(string inputLine, vector<ShellCommand> *commandList, int lineId);
        string npshellReadLine();
        // void resetEnvVar();
        // void setClientEnvVar(const char *env, const char *val);
        void setShareMemory(MultiClientInfo *clientList);
};