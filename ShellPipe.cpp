#include "ShellPipe.h"

ShellPipe::ShellPipe() {
    isReadOpen = false;
    isWriteOpen = false;
}

ShellPipe::~ShellPipe() {;}

int ShellPipe::getFdRead() {
    return fd[0];
}

int ShellPipe::getFdWrite() {
    return fd[1];
}

void ShellPipe::openWholePipe() {
    if (pipe(fd) < 0) {
        cout << "pipe create error" << endl;
    } else {
        isReadOpen = true;
        isWriteOpen = true;
    }
}

void ShellPipe::closeWholePipe() {
    if (isReadOpen) {
        close(fd[0]);
        isReadOpen = false;
    }
    if (isWriteOpen) {
        close(fd[1]);
        isWriteOpen = false;
    }   
}

void ShellPipe::closeRead() {
    close(fd[0]);
    isReadOpen = false;
}

void ShellPipe::closeWrite() {
    close(fd[1]);
    isWriteOpen = false;
}

void ShellPipe::setIsReadByClient(bool _inIsReadByClient) {
    isReadByClient = _inIsReadByClient;
}

bool ShellPipe::getIsReadByClient() {
    return isReadByClient;
}