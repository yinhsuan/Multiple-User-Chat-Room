#include <iostream>
using namespace std;

class ClientInfo {
    private:
        int id;
        string name;
        string ip;
        int port;
        int fd;
        bool isActive;
        int pid;
        bool isFromClientPipe;
        bool isToClientPipe;
        int fromClientPipeNo;
        int toClientPipeNo;
        bool isClientPrintMsg;
        string msg;
        int fromClientFD; // input from user pipe => share memory

    public:
        ClientInfo();
        ~ClientInfo();

        void setID(int _inID);
        int getID();

        void setName(string _inName);
        string getName();

        void setIP(string _inIP);
        string getIP();

        void setPort(int _inPort);
        int getPort();
        
        void setFD(int _inFD); // ssock
        int getFD();

        void setIsActive(bool _inIsActive);
        bool getIsActive();

        void setPid(int _inPid);
        int getPid();

        void setIsFromClientPipe(bool _inIsFromClientPipe);
        bool getIsFromClientPipe();

        void setIsToClientPipe(bool _inIsToClientPipe);
        bool getIsToClientPipe();

        void setFromClientPipeNo(int _inFromClientPipeNo);
        int getFromClientPipeNo();

        void setToClientPipeNo(int _inFromClientPipeNo);
        int getToClientPipeNo();

        void setIsClientPrintMsg(bool _inIsClientPrintMsg);
        bool getIsClientPrintMsg();

        void setMsg(string _inMsg);
        string getMsg();

        void setFromClientFD(int _inFromClientFD);
        int getFromClientFD();

};