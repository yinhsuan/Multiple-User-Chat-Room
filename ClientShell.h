#include <iostream>
#include <netinet/in.h>
#include <sstream>
// #include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <map>
#include "ClientInfo.cpp"
#include "ShellCommand.cpp"
#include "ShellPipe.cpp"
using namespace std;

enum ProcessMode {
	NoWait = 0,
	ToWait = 1,
	ToExit = 2,
	ToEnvVar = 3,
    ToChatCmd = 4,
};

class ClientShell {
    private:
        ClientInfo clientInfo;
        int lineId;
        map<string, string> clientEnvVar;
        map<int, ShellPipe> lineList; // for number pipe
        map<int, ShellPipe> pipeList; // for line command pipe
        string wholeLineCommand;

    public:
        ClientShell();
        ~ClientShell();

        ClientInfo getClientInfo();

        // void broadcastToAll();
        void clientPrintMsg(const char *msg, int ssock);
        void clientYell(const char *msg, ClientShell *clientList, int maxClientNum);

        ProcessMode executeShell(string inputLine, ClientShell *clientList, map<pair<int,int>, ShellPipe> *clientPipe, int maxClientNum);
        

        void clientLogIn(int id, sockaddr_in clientAddress, ClientShell *clientList, int ssock, int maxClientNum);
        void clientLogOut(ClientShell *clientList, map<pair<int,int>, ShellPipe> *clientPipe, int maxClientNum);

        void exeChatCommand(vector<ShellCommand> cList, ClientShell *clientList, int maxClientNum);
        void setEnvironmentVar(vector<ShellCommand> cList);
        // void signalHandler(int receivedSignal);
        pid_t npshellExeCommand(ShellCommand command, map<int, ShellPipe> *pipelist, map<int, ShellPipe> *lineList, map<pair<int,int>, ShellPipe> *clientPipe, ClientShell *clientList, int maxClientNum);
        ProcessMode npshellSplitLine(string inputLine, vector<ShellCommand> *commandList, int lineId);
        string npshellReadLine();
        void resetEnvVar();
        void setClientEnvVar(const char *env, const char *val);
        
};