#include "MultiClientInfo.h"

MultiClientInfo::MultiClientInfo() {
    id = 0;
    // name = "";
    // ip = "";
    port = 0;
    fd = 0; 
    isActive = false;
    isFromClientPipe = false;
    isToClientPipe = false;
    fromClientPipeNo = -1;
    toClientPipeNo = -1;
    isOpenRead = false;
    for (int i=0; i<16; i++) {
        ip[i] = '0';
    }
    for (int i=0; i<40; i++) {
        name[i] = '0';
    }
    for (int i=0; i<4000; i++) {
        msg[i] = '0';
    }
}

MultiClientInfo::~MultiClientInfo(){;}

void MultiClientInfo::setID(int _inID) {
    id = _inID;
}

int MultiClientInfo::getID() {
    return id;
}

void MultiClientInfo::setName(const char* _inName) {
    // name = _inName;
    strcpy(name, _inName);
}

char* MultiClientInfo::getName() {
    return name;
}

void MultiClientInfo::setIP(const char* _inIP) {
    // ip = _inIP;
    strcpy(ip, _inIP);
}

char* MultiClientInfo::getIP() {
    return ip;
}

void MultiClientInfo::setPort(int _inPort) {
    port = _inPort;
}

int MultiClientInfo::getPort() {
    return port;
}

void MultiClientInfo::setFD(int _inFD) {
    fd = _inFD;
}

int MultiClientInfo::getFD() {
    return fd;
}

void MultiClientInfo::setIsActive(bool _inIsActive) {
    isActive = _inIsActive;
}

bool MultiClientInfo::getIsActive() {
    return isActive;
}

void MultiClientInfo::setPid(int _inPid) {
    pid = _inPid;
}

int MultiClientInfo::getPid() {
    return pid;
}

void MultiClientInfo::setIsFromClientPipe(bool _inIsFromClientPipe) {
    isFromClientPipe = _inIsFromClientPipe;
}

bool MultiClientInfo::getIsFromClientPipe() {
    return isFromClientPipe;
}

void MultiClientInfo::setIsToClientPipe(bool _inIsToClientPipe) {
    isToClientPipe = _inIsToClientPipe;
}

bool MultiClientInfo::getIsToClientPipe() {
    return isToClientPipe;
}

void MultiClientInfo::setFromClientPipeNo(int _inFromClientPipeNo) {
    fromClientPipeNo = _inFromClientPipeNo;
}

int MultiClientInfo::getFromClientPipeNo() {
    return fromClientPipeNo;
}

void MultiClientInfo::setToClientPipeNo(int _inFromClientPipeNo) {
    toClientPipeNo = _inFromClientPipeNo;
}

int MultiClientInfo::getToClientPipeNo() {
    return toClientPipeNo;
}

void MultiClientInfo::setIsClientPrintMsg(bool _inIsClientPrintMsg) {
    isClientPrintMsg = _inIsClientPrintMsg;
}
bool MultiClientInfo::getIsClientPrintMsg() {
    return isClientPrintMsg;
}

void MultiClientInfo::setMsg(const char* _inMsg) {
    // msg = _inMsg;
    strcpy(msg, _inMsg);
}

char* MultiClientInfo::getMsg() {
    return msg;
}

void MultiClientInfo::setFromClientFD(int _inFromClientFD) {
    fromClientFD = _inFromClientFD;
}

int MultiClientInfo::getFromClientFD() {
    return fromClientFD;
}

void MultiClientInfo::setIsOpenRead(bool _inIsOpenRead) {
    isOpenRead = _inIsOpenRead;
}

bool MultiClientInfo::getIsOpenRead() {
    return isOpenRead;
}