#include "ClientInfo.h"

ClientInfo::ClientInfo() {
    id = 0;
    name = "";
    ip = "";
    port = 0;
    fd = 0; 
    isActive = false;
    isFromClientPipe = false;
    isToClientPipe = false;
    fromClientPipeNo = -1;
    toClientPipeNo = -1;
}

ClientInfo::~ClientInfo(){;}

void ClientInfo::setID(int _inID) {
    id = _inID;
}

int ClientInfo::getID() {
    return id;
}

void ClientInfo::setName(string _inName) {
    name = _inName;
}

string ClientInfo::getName() {
    return name;
}

void ClientInfo::setIP(string _inIP) {
    ip = _inIP;
}

string ClientInfo::getIP() {
    return ip;
}

void ClientInfo::setPort(int _inPort) {
    port = _inPort;
}

int ClientInfo::getPort() {
    return port;
}

void ClientInfo::setFD(int _inFD) {
    fd = _inFD;
}

int ClientInfo::getFD() {
    return fd;
}

void ClientInfo::setIsActive(bool _inIsActive) {
    isActive = _inIsActive;
}

bool ClientInfo::getIsActive() {
    return isActive;
}

void ClientInfo::setPid(int _inPid) {
    pid = _inPid;
}

int ClientInfo::getPid() {
    return pid;
}

void ClientInfo::setIsFromClientPipe(bool _inIsFromClientPipe) {
    isFromClientPipe = _inIsFromClientPipe;
}

bool ClientInfo::getIsFromClientPipe() {
    return isFromClientPipe;
}

void ClientInfo::setIsToClientPipe(bool _inIsToClientPipe) {
    isToClientPipe = _inIsToClientPipe;
}

bool ClientInfo::getIsToClientPipe() {
    return isToClientPipe;
}

void ClientInfo::setFromClientPipeNo(int _inFromClientPipeNo) {
    fromClientPipeNo = _inFromClientPipeNo;
}

int ClientInfo::getFromClientPipeNo() {
    return fromClientPipeNo;
}

void ClientInfo::setToClientPipeNo(int _inFromClientPipeNo) {
    toClientPipeNo = _inFromClientPipeNo;
}

int ClientInfo::getToClientPipeNo() {
    return toClientPipeNo;
}

void ClientInfo::setIsClientPrintMsg(bool _inIsClientPrintMsg) {
    isClientPrintMsg = _inIsClientPrintMsg;
}
bool ClientInfo::getIsClientPrintMsg() {
    return isClientPrintMsg;
}

void ClientInfo::setMsg(string _inMsg) {
    msg = _inMsg;
}

string ClientInfo::getMsg() {
    return msg;
}

void ClientInfo::setFromClientFD(int _inFromClientFD) {
    fromClientFD = _inFromClientFD;
}

int ClientInfo::getFromClientFD() {
    return fromClientFD;
}