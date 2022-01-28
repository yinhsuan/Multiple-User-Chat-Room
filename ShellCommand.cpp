#include "ShellCommand.h"

ShellCommand::ShellCommand(){
    commandName = "";
    Argv.clear();
    commandId = 0;
    ioState = ToStdOut;
    toNumberCatNo = -1;
    fromNumberCatNo = -1;
    isReadFromNumberCat = false;
}

ShellCommand::ShellCommand(string _inName, int _inId){
    commandName = _inName;
    Argv.clear();
    commandId = _inId;
    ioState = ToStdOut;
    isReadFromNumberCat = false;
}

ShellCommand::~ShellCommand(){;}

void ShellCommand::setCommandName(string _inName) {
    commandName = _inName;
}

string ShellCommand::getCommandName() {
    return commandName;
}

void ShellCommand::addCommandArgv(string _inArgv) {
    Argv.push_back(_inArgv);
}

vector<string> ShellCommand::getCommandArgv() {
    return Argv;
}

void ShellCommand::setCommandId(int _inId) {
    commandId = _inId;
}

int ShellCommand::getCommandId() {
    return commandId;
}

void ShellCommand::setIOState(CommandStdIOState _inIOState) {
    ioState = _inIOState;
}

CommandStdIOState ShellCommand::getIOState() {
    return ioState;
}

void ShellCommand::setIsEnvVariable(bool _inIsEnv) {
    isEnvVariable = _inIsEnv;
}

bool ShellCommand::getIsEnvVariable() {
    return isEnvVariable;
}

void ShellCommand::setDstFileName(string _inDstFileName) {
    dstFileName = _inDstFileName;
}

string ShellCommand::getDstFileName() {
    return dstFileName;
}

void ShellCommand::setEachLineId(int _inLineId) {
    eachLineId = _inLineId;
}

int ShellCommand::getEachLineId() {
    return eachLineId;
}

void ShellCommand::setNumberPipeNum(int _inNumberPipeNum) {
    numberPipeNum = _inNumberPipeNum;
}


int ShellCommand::getNumberPipeNum() {
    return numberPipeNum;
}

void ShellCommand::setToNumberCatNo(int _inToNumberCatNo) {
    toNumberCatNo = _inToNumberCatNo;
}

int ShellCommand::getToNumberCatNo() {
    return toNumberCatNo;
}

void ShellCommand::setFromNumberCatNo(int _inFromNumberCatNo) {
    fromNumberCatNo = _inFromNumberCatNo;
}
int ShellCommand::getFromNumberCatNo() {
    return fromNumberCatNo;
}

void ShellCommand::setIsReadFromNumberCat(bool _inIsReadFromNumberCat) {
    isReadFromNumberCat = _inIsReadFromNumberCat;
}

bool ShellCommand::getIsReadFromNumberCat() {
    return isReadFromNumberCat;
}