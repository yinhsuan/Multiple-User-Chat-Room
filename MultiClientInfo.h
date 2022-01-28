#include <iostream>
using namespace std;

class MultiClientInfo {
    private:
        int id;
        char name[40];
        char ip[16];
        int port;
        int fd;
        bool isActive;
        int pid;
        bool isFromClientPipe;
        bool isToClientPipe;
        int fromClientPipeNo;
        int toClientPipeNo;
        bool isClientPrintMsg;
        char msg[4000];
        int fromClientFD; // input from user pipe => share memory
        bool isOpenRead;

    public:
        MultiClientInfo();
        ~MultiClientInfo();

        void setID(int _inID);
        int getID();

        void setName(const char* _inName);
        char* getName();

        void setIP(const char* _inIP);
        char* getIP();

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

        void setMsg(const char* _inMsg);
        char* getMsg();

        void setFromClientFD(int _inFromClientFD);
        int getFromClientFD();

        void setIsOpenRead(bool _inIsOpenRead);
        bool getIsOpenRead();

};