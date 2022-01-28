#include "MultiClientShell.h"



void MultiClientShell::showCommandPrompt() {
	cout << "% "; 
}

MultiClientShell::MultiClientShell() {
    // clientInfo = new ClientInfo();
    // int lineId = -1;
}

MultiClientShell::~MultiClientShell() {;}

// ClientInfo MultiClientShell::getClientInfo() {
//     return clientInfo;
// }

// void MultiClientShell::broadcastToAll() {

// }


string MultiClientShell::npshellReadLine() {
	string line;
	getline(cin, line);
	return line;
}

ProcessMode MultiClientShell::npshellSplitLine(string inputLine, vector<ShellCommand> *commandList, int lineId) {
	vector<ShellCommand> tokens;
	stringstream ss(inputLine);
	string parseWord;
	ProcessMode pMode = ToWait;
	int commandId = -1;
	CommandStatus cStatus = IsMain;
	

	while (ss >> parseWord) {
        if (commandId != -1) {
            wholeLineCommand += " ";
        }
        wholeLineCommand += parseWord;

		if (parseWord == "exit") {
			pMode = ToExit;
			return pMode;
		}
        if (pMode == ToChatCmd) {
            commandList->at(commandId).addCommandArgv(parseWord);
        }
		else if (parseWord[0] == '|' || parseWord[0] == '!') {
			// Normal pipe
			if (parseWord.size() == 1) {
				if (parseWord[0] == '|') {
                    cout << "check point 1" << endl;
					commandList->at(commandId).setIOState(ToStdOutPipe);
				} 
				else if (parseWord[0] == '!') {
					commandList->at(commandId).setIOState(ToStdOutandStdErrPipe);
				}				
				cStatus = IsMain;				
			}
			// Number pipe => should NOT wait
			else {			
				if (parseWord[0] == '|') {
					commandList->at(commandId).setIOState(ToStdOutNumberPipe);
				} 
				else if (parseWord[0] == '!') {
					commandList->at(commandId).setIOState(ToStdOutandStdErrNumberPipe);
				}	
				string tmp = parseWord.substr(1);
                int parseNumberPipeNum = 0;
				parseNumberPipeNum = atoi(tmp.c_str());
				commandList->at(commandId).setNumberPipeNum(parseNumberPipeNum);	
				pMode = NoWait;
			}			
		} 
		else if (parseWord[0] == '>') {
            if (parseWord.size() == 1) {
                commandList->at(commandId).setIOState(ToFile);
			    cStatus = IsFile;
            }
            else {
                cout << "check point 2" << endl;
                string tmp = parseWord.substr(1);
                int parseNumberCatNum = 0;
				parseNumberCatNum = atoi(tmp.c_str());
                commandList->at(commandId).setToNumberCatNo(parseNumberCatNum);
                commandList->at(commandId).setIOState(ToNumberCat);
                // commandList->at(commandId).setFromNumberCatNo(clientInfo.getID());
                pMode = ToWait;
            }
		}
        else if (parseWord[0] == '<') {
            cout << "check point 3" << endl; 
            string tmp = parseWord.substr(1);
            int parseNumberCatNum = 0;
            parseNumberCatNum = atoi(tmp.c_str());
            commandList->at(commandId).setFromNumberCatNo(parseNumberCatNum);
            commandList->at(commandId).setIsReadFromNumberCat(true);
            // commandList->at(commandId).setToNumberCatNo(clientInfo.getID());
            pMode = ToWait; 
        }
		else if (cStatus == IsMain) {
                 
            cout << "check point 4" << endl;    
			if (parseWord == "setenv" || parseWord == "printenv") {
				pMode = ToEnvVar;
			} 
            else if (parseWord == "who" || parseWord == "yell" || parseWord == "tell" || parseWord == "name") {
                pMode = ToChatCmd;
            }
			commandId++;
			commandList->push_back(ShellCommand(parseWord, commandId));
			commandList->at(commandId).addCommandArgv(parseWord);
			commandList->at(commandId).setIOState(ToStdOut);
			commandList->at(commandId).setCommandId(commandId);
			commandList->at(commandId).setEachLineId(lineId);
			cStatus = IsArgv;
		} 
		else if (cStatus == IsArgv) {
            cout << "check point 5" << endl;    
			commandList->at(commandId).addCommandArgv(parseWord);
		} 
		else if (cStatus == IsFile) {
			commandList->at(commandId).setDstFileName(parseWord);
			cStatus = IsDone;
		} 
	}
	return pMode;
}



pid_t MultiClientShell::npshellExeCommand(int id, ShellCommand command, map<int, ShellPipe> *pipelist, map<int, ShellPipe> *lineList, MultiClientInfo *clientList, int maxClientNum) {
	pid_t pid;
	int commandId = command.getCommandId();
	int thisLineId = command.getEachLineId();
	int numberPipeNum = command.getNumberPipeNum();
	int stdInRedirectNo = 0;
	int stdOutRedirectNo = 0;
	int stdErrRedirectNo = 0;
	char* fileName = "";
	CommandStdIOState cStatus = command.getIOState();
    int thisFD = clientList[id-1].getFD();
    int fromClientNo = 0;
    int toClientNo = 0;
    stringstream ss;
    string senderName = "";
    string receiverName = "";
    bool isToClientStdOut = false;
    // int thisSock = clientInfo.getFD();
    int nullFD = open("/dev/null", O_RDWR);
    
    ss.str("");
    ss.clear();

    // cout << "command name: " << command.getCommandName() << endl;

	// 1. specify this command input
	if (pipelist->count(commandId) != 0) {
        // cout << "check point #" << endl;
		stdInRedirectNo = pipelist->at(commandId).getFdRead();
        // cout << "in no: " << stdInRedirectNo << endl;
	}

	if (commandId == 0 && lineList->count(thisLineId) != 0) {
        cout << "check point 6" << endl;
		stdInRedirectNo = lineList->at(thisLineId).getFdRead();
	}

    // FromNumberCat
    if (command.getIsReadFromNumberCat()) {
        fromClientNo = command.getFromNumberCatNo();
        toClientNo = clientList[id-1].getID();
        if (1 > fromClientNo || fromClientNo > 30 || !clientList[fromClientNo-1].getIsActive()) {
            cout << "check point 7" << endl;
            ss << "*** Error: user #" << fromClientNo << " does not exist yet. ***" << endl;
            // clientPrintMsg(ss.str().c_str(), thisFD);
            cout << ss.str().c_str();
            // return -1;
            stdInRedirectNo = nullFD;
            // cout << "stdInRedirectNo: " << stdInRedirectNo << endl;
        }
        // else if (clientPipe->count(pair<int, int>(fromClientNo, toClientNo)) == 0) {
        else if (clientList[id-1].getFromClientPipeNo() != fromClientNo) {
            cout << "check point 8" << endl;
            ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " does not exist yet. ***" << endl;
            // clientPrintMsg(ss.str().c_str(), thisFD);
            cout << ss.str().c_str();
            // return -1;
            stdInRedirectNo = nullFD;
            // cout << "stdInRedirectNo: " << stdInRedirectNo << endl;
        }
        else {
            cout << "check point 9" << endl;
            stdInRedirectNo = clientList[id-1].getFromClientFD();
            clientList[id-1].setIsFromClientPipe(false);
            clientList[id-1].setFromClientPipeNo(-1);
            clientList[fromClientNo-1].setIsToClientPipe(false);
            clientList[fromClientNo-1].setToClientPipeNo(-1);
            // cout << "stdInRedirectNo: " << stdInRedirectNo << endl;

            // read from user pipe => broadcast
            ss.str("");
            ss.clear();
            senderName = clientList[fromClientNo-1].getName();
            ss << "*** " << clientList[id-1].getName() << " (#" << clientList[id-1].getID()  << ")" << " just received from " <<  senderName << " (#" << fromClientNo << ")" << " by '" << wholeLineCommand << "' ***" << endl;
            clientYell(ss.str().c_str(), clientList, maxClientNum);
        }
    }

    ss.str("");
    ss.clear();
	// 2. specify this command output
	switch(cStatus) {
		case ToStdOut:
            cout << "check point a" << endl;
			break;
		case ToStdOutPipe:
            cout << "check point b" << endl;
			if (pipelist->count(commandId+1) == 0) {
				pipelist->insert(pair<int, ShellPipe>(commandId+1, ShellPipe()));
				pipelist->at(commandId+1).openWholePipe();
			}
			stdOutRedirectNo = pipelist->at(commandId+1).getFdWrite();

            // cout << "out no: " << stdOutRedirectNo << endl;
			break;
		case ToStdOutandStdErrPipe:
            // cout << "check point 5" << endl;
			if (pipelist->count(commandId+1) == 0) {
				pipelist->insert(pair<int, ShellPipe>(commandId+1, ShellPipe()));
				pipelist->at(commandId+1).openWholePipe();
			}
			stdOutRedirectNo = pipelist->at(commandId+1).getFdWrite();
			stdErrRedirectNo = pipelist->at(commandId+1).getFdWrite();
			break;
		case ToFile:
            // cout << "check point 3" << endl;
			stdOutRedirectNo = open(command.getDstFileName().c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0666);
			break;
		case ToStdOutNumberPipe:
            // cout << "check point 4" << endl;
			if (lineList->count(thisLineId+numberPipeNum) == 0) {
				lineList->insert(pair<int, ShellPipe>(thisLineId+numberPipeNum, ShellPipe()));
				lineList->at(thisLineId+numberPipeNum).openWholePipe();
			}
			stdOutRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			break;
		case ToStdOutandStdErrNumberPipe:
            // cout << "check point 5" << endl;
			if (lineList->count(thisLineId+numberPipeNum) == 0) {
				lineList->insert(pair<int, ShellPipe>(thisLineId+numberPipeNum, ShellPipe()));
				lineList->at(thisLineId+numberPipeNum).openWholePipe();
			}
			stdOutRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			stdErrRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			break;
        case ToNumberCat:
            fromClientNo = clientList[id-1].getID();
            toClientNo = command.getToNumberCatNo();
            // cout << "clientPipe->size() in ToNumberCat: " << clientPipe->size() << endl;
            if (1 > toClientNo || toClientNo > 30 || !clientList[toClientNo-1].getIsActive()) {
                cout << "check point c" << endl;
                ss << "*** Error: user #" << toClientNo << " does not exist yet. ***" << endl;
                // clientPrintMsg(ss.str().c_str(), thisFD);
                cout << ss.str().c_str();
                // return -1;
                stdOutRedirectNo = nullFD;
            }
            else if (clientList[id-1].getToClientPipeNo() == toClientNo) {
                cout << "check point d" << endl;
                ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " already exists. ***" << endl;
                // clientPrintMsg(ss.str().c_str(), thisFD);
                cout << ss.str().c_str();
                // return -1;
                stdOutRedirectNo = nullFD;
            }
            else {            
                // ss << "./user_pipe/" << fromClientNo << "to" << toClientNo;
                cout << "check point e" << endl;
                clientList[id-1].setIsToClientPipe(true);
                clientList[id-1].setToClientPipeNo(toClientNo);
                clientList[toClientNo-1].setIsFromClientPipe(true);
                clientList[toClientNo-1].setFromClientPipeNo(fromClientNo);
                clientList[toClientNo-1].setIsOpenRead(false);

                // cout << "fromClientNo: " << fromClientNo << endl;
                // cout << "toClientNo: " << toClientNo << endl;

                char fifopW[30];
                sprintf(fifopW, "./user_pipe/%dto%d", fromClientNo, toClientNo);
                // cout << "clientList[toClientNo-1].getPid()" << clientList[toClientNo-1].getPid() << endl;
                kill(clientList[toClientNo-1].getPid(), SIGUSR2);
                // cout << "after kill" << endl;
                mkfifo(fifopW, 0666);
                stdOutRedirectNo = open(fifopW, O_WRONLY);
                isToClientStdOut = true;
                // cout << "open W: " << stdOutRedirectNo << endl;

                ss.str("");
                ss.clear();
                senderName = clientList[fromClientNo-1].getName();
                receiverName = clientList[toClientNo-1].getName();
                ss << "*** " << senderName << " (#" << fromClientNo << ")" << " just piped '" << wholeLineCommand << "' to " << receiverName << " (#" << toClientNo << ") ***" << endl;
                clientYell(ss.str().c_str(), clientList, maxClientNum);
            }
            
            break;
	}	
    // cout << "check point 2" << endl;

	while((pid = fork()) < 0) {
		usleep(100);
	}

    // cout << "clientList[0].getFromClientFD: " << clientList[0].getFromClientFD() << endl;

	// 1. change this command input
	// 2. change this command output
	if(pid < 0) {
		cout << "Fork Error!" << endl;
	} 
	// child process
	else if (pid == 0) {
        // cout << "in No: " << stdInRedirectNo << endl;
        // cout << "out No: " << stdOutRedirectNo << endl;

        // -------- STDIN -------
		if (stdInRedirectNo) {
            // cout << "stdInRedirectNo: " << stdInRedirectNo << endl;
			dup2(stdInRedirectNo, STDIN_FILENO);
			close(stdInRedirectNo);

			if (pipelist->count(commandId) != 0) {
                cout << "check point A" << endl;
				pipelist->at(commandId).closeWholePipe();
			} 
			else if (lineList->count(thisLineId) != 0) {
                cout << "check point B" << endl;
				lineList->at(thisLineId).closeWholePipe();
			}
            else {
                // cout << "check point 7" << endl;
                if (!clientList[id-1].getIsFromClientPipe()) {
                    cout << "check point C" << endl;
                    close(clientList[id-1].getFromClientFD());
                }                
            }
		}
        // -------- STDOUT -------
		if (stdOutRedirectNo) {
            cout << "check point D" << endl;
            // cout << "stdOutRedirectNo: " << stdOutRedirectNo << endl;
			dup2(stdOutRedirectNo, STDOUT_FILENO);
			if (stdOutRedirectNo != stdErrRedirectNo) {
                cout << "check point F" << endl;
				close(stdOutRedirectNo);
			}			
		}
        // -------- STDERR -------
		if (stdErrRedirectNo) {
            // cout << "stdErrRedirectNo" << endl;
            cout << "check point E" << endl;
			dup2(stdErrRedirectNo, STDERR_FILENO);
			close(stdErrRedirectNo);
		}
        // cout << "check point 7" << endl;
        // close(thisFD);
       

		const char * cName = command.getCommandName().c_str();
		char* arg[command.getCommandArgv().size()+1];

		for (int i=0; i < command.getCommandArgv().size(); i++){
			const char *tmp = command.getCommandArgv()[i].c_str();
			arg[i] = new char [command.getCommandArgv()[i].length()+1];
			strcpy(arg[i], tmp);
		}
		arg[command.getCommandArgv().size()] = NULL;
        

		int exeCmd = execvp(cName, arg);
        // cout << "check point 8" << endl;
		if (exeCmd == -1){
            cerr << "Unknown command: [" << cName << "]." << endl;
        }
		exit(0);
	}		
	// parent process
	else {
		if (stdInRedirectNo) {
			if (pipelist->count(commandId) != 0) {
                cout << "check point *" << endl;
				pipelist->at(commandId).closeWholePipe();
				pipelist->erase(commandId);
			} 
			else if (lineList->count(thisLineId) != 0) {
                cout << "check point #" << endl;
				lineList->at(thisLineId).closeWholePipe();
				lineList->erase(thisLineId);
			}	
            else {
                // cout << "check point 5" << endl;
                if (!clientList[id-1].getIsFromClientPipe()) {
                    cout << "check point @" << endl;
                    close(clientList[id-1].getFromClientFD());
                }   
                // close(clientList[id-1].getFromClientFD());
            }
		}
        // if (stdOutRedirectNo && clientList[id-1].getIsToClientPipe()) {
        if (stdOutRedirectNo && isToClientStdOut) {
            cout << "check point $" << endl;
            close(stdOutRedirectNo); // number pipe for 2 times will print error
        }
	}
	return pid;
}


void MultiClientShell::setEnvironmentVar(vector<ShellCommand> cList) {
	string cName = cList[0].getCommandName();
	vector<string> argv = cList[0].getCommandArgv();
    stringstream ss;
    // int thisSock = clientList[id-1].getFD();

    if (cName == "setenv") {
		setenv(argv[1].c_str(), argv[2].c_str(), 1);
	}
	else if (cName == "printenv") {
		char *env = getenv(argv[1].c_str());
		if (env != NULL) {
			cout << env << endl;
		}
	}
	else {
		cout << "Not a Environment Variable" << endl;
	}
}


// void MultiClientShell::resetEnvVar() {
//     // setenv("PATH", "bin:.", 1); // 1: overwrite; 0: not overwrite
//     for (map<string,string>::iterator iter=clientEnvVar.begin(); iter!=clientEnvVar.end(); ++iter) {
//         setenv(iter->first.c_str(), iter->second.c_str(), 1);
//     }
// }

// void MultiClientShell::setClientEnvVar(const char *env, const char *val) {
//     setenv(env, val, 1);
//     if (clientEnvVar.count(string(env)) == 0) {
//         clientEnvVar.insert(pair<string, string>(string(env), string(val)));
//     }
//     else {
//         clientEnvVar[string(env)] = string(val);  
//     }      
// }


void MultiClientShell::exeChatCommand(int id, vector<ShellCommand> cList, MultiClientInfo *clientList, int maxClientNum) {
    string cName = cList[0].getCommandName();
	vector<string> argv = cList[0].getCommandArgv();
    stringstream ss;
    int thisSock = clientList[id-1].getFD();
    MultiClientInfo cInfo;

    if (cName == "who") {
        ss << "<ID>" << "\t" << "<nickname>" << "\t" << "<IP:port>" << "\t" << "<indicate me>" << endl;
        for (int cId=0; cId<maxClientNum; cId++) {
            // cInfo = clientList[cId].getClientInfo();
            cInfo = clientList[cId];
            if (cInfo.getIsActive()) {
                ss << cInfo.getID() << "\t" << cInfo.getName() << "\t" << cInfo.getIP() << ":" << cInfo.getPort();
                if ((cId+1) == clientList[id-1].getID()) {
                    ss << "\t" << "<-me";
                }
                ss << endl;
            }            
        }
        // clientPrintMsg(ss.str().c_str(), thisSock);
        cout << ss.str().c_str();
    }
    else if (cName == "name") {
        // string newName = argv[1];     
        string newName = "";   
        for (int i=1; i<argv.size()-1; i++) {
            newName += argv[i];
            newName += " ";
        }
        newName += argv[argv.size()-1];
        // check if the name had already exist
        for (int cId=0; cId<maxClientNum; cId++) {
            cInfo = clientList[cId];
            if (cInfo.getID() == clientList[id-1].getID()) {
                continue;
            }
            // if (cInfo.getName() == newName.c_str()) {
            if (strcmp(cInfo.getName(), newName.c_str()) == 0) {
                ss << "*** User '" << newName << "' already exists. ***" << endl;
                // clientPrintMsg(ss.str().c_str(), thisSock);
                cout << ss.str().c_str();
                return;
            }
        }
        // set new name
        clientList[id-1].setName(newName.c_str());
        // broadcast to all client
        ss << "*** User from " << clientList[id-1].getIP() << ":" << clientList[id-1].getPort() << " is named '" << newName << "'. ***" << endl;
        clientYell(ss.str().c_str(), clientList, maxClientNum);
    }
    else if (cName == "yell") {
        string yellMsg = "";
        for (int i=1; i<argv.size()-1; i++) {
            yellMsg += argv[i];
            yellMsg += " ";
        }
        yellMsg += argv[argv.size()-1];
        ss << "*** " << clientList[id-1].getName() << " yelled ***: " << yellMsg << endl;
        clientYell(ss.str().c_str(), clientList, maxClientNum);
    }
    else if (cName == "tell") {
        // check if the receiver exists
        cInfo = clientList[stoi(argv[1])-1];
        if (!cInfo.getIsActive()) {
            ss << "*** Error: user #" << stoi(argv[1]) << " does not exist yet. ***" << endl;
            // clientPrintMsg(ss.str().c_str(), thisSock);
            cout << ss.str().c_str();
            return;
        }
        string tellMsg = "";
        for (int i=2; i<argv.size()-1; i++) {
            tellMsg += argv[i];
            tellMsg += " ";
        }
        tellMsg += argv[argv.size()-1];
        ss << "*** " << clientList[id-1].getName() << " told you ***: " << tellMsg << endl;        
        clientList[stoi(argv[1])-1].setMsg(ss.str().c_str());
        clientList[stoi(argv[1])-1].setIsClientPrintMsg(true);
        kill(clientList[stoi(argv[1])-1].getPid(), SIGUSR1);
        // clientPrintMsg(ss.str().c_str(), cInfo.getFD());
    }
    else {
        cout << "Not a Chat Command" << endl;
    }
    ss.str("");
    ss.clear();
}

// void childHandler(int signo){
//     int status;
//     while (waitpid(-1, &status, WNOHANG) > 0);
// }

string trimstr(string s){
  size_t n = s.find_last_not_of(" \r\n\t");
    if (n != string::npos){
        s.erase(n + 1, s.size() - n);
    }
    n = s.find_first_not_of(" \r\n\t");
    if (n != string::npos){
        s.erase(0, n);
    }
    return s;
}


void MultiClientShell::executeShell(int newClientID, sockaddr_in clientAddress, int ssock, MultiClientInfo *clientList, int maxClientNum) {
    ProcessMode pStatus = NoWait;
	vector<ShellCommand> commandList; // for each line commands
	// map<int, ShellPipe> pipeList; // for line command pipe
	// map<int, ShellPipe> lineList; // for number pipe
	pid_t lastCommand;
	pid_t middleCommand;
    int thisSock = clientList[newClientID-1].getFD();
    wholeLineCommand = "";
	int lineId = -1;
	string inputLine = "";
    string beforeTrimLine = "";
    // thisId = newClientID;
    // signal(SIGUSR2, openReadFifo);

   
    // inputLine.clear();
    // char inputLine[15000];

	// -------------- Set environmental variable at the start -------------- 
    // resetEnvVar();	 
    // signal(SIGCHLD, childHandler);
    // cout << "cp 1" << endl;
    setShareMemory(clientList);
	setenv("PATH", "bin:.", 1); // 1: overwrite; 0: not overwrite
    // cout << "cp 2" << endl;
    clientLogIn(newClientID, clientAddress, clientList, ssock, maxClientNum);
    // cout << "cp 3" << endl;
	// signal(SIGCHLD, signalHandler);

	// Enter shell
    showCommandPrompt();
	while (getline(cin, beforeTrimLine)) {
        // -------------- Initial -------------- 
        commandList.clear();
        pipeList.clear();
//        inputLine = trimstr(beforeTrimLine);
  
inputLine = beforeTrimLine;      
        // cout << "cp 4" << endl;
        // if (inputLine.empty()) {
        //     // clientPrintMsg("% ", thisSock);
        //     // return NoWait;
        //     // showCommandPrompt();
        //     // continue;
        // }

        // if (!inputLine.empty()) {
        //     lineId++;
        // }
        // cout << "lineId: " << lineId << endl;
        // lineId++;


        if (inputLine.length() == 1) {
            showCommandPrompt();
            continue;
        }
        else {
            lineId++;
            // -------------- Store each command in one line --------------
            pStatus = npshellSplitLine(inputLine, &commandList, lineId);
            if (pStatus == ToExit) {
                clientLogOut(newClientID, clientList, maxClientNum);
                break;
            }
            else if (pStatus == ToEnvVar) {
                setEnvironmentVar(commandList);
            }
            else if (pStatus == ToChatCmd) {
                // cout << "cp 5" << endl;
                exeChatCommand(newClientID, commandList, clientList, maxClientNum);
            }
            else {
                // -------------- Execute each command -------------- 
                for (vector<ShellCommand>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter) {
                    // last command => wait
                    lastCommand = npshellExeCommand(newClientID, *iter, &pipeList, &lineList, clientList, maxClientNum);
                }
                //wait
                if (pStatus == ToWait && lastCommand > 0) {
                    int status;
                    waitpid(lastCommand, &status, 0);
                }
            }
            // clientPrintMsg("% ", thisSock);
            showCommandPrompt();
            inputLine.clear();
            wholeLineCommand.clear();
        } 
	}
}

void MultiClientShell::setShareMemory(MultiClientInfo *clientList) {
    int shm = shmget((key_t)IPC_PRIVATE, sizeof(MultiClientInfo)*MAXCLIENTNUM, IPC_CREAT|IPC_EXCL|0600);
    clientList = (MultiClientInfo*)shmat(shm, 0, 0);
}






void MultiClientShell::clientLogIn(int id, sockaddr_in clientAddress, MultiClientInfo *clientList, int ssock, int maxClientNum) {
    // cout << "Enter clientLogIn" << endl;
    stringstream ss;
    // clientInfo.setIsActive(true);
    clientList[id-1].setIsActive(true);
    // setClientEnvVar("PATH", "bin:.");

    // set client info => ip/ port/ name/ fd/ id
    // char ipStr[16];
    // strcpy(ipStr, inet_ntoa(clientAddress.sin_addr));
    clientList[id-1].setIP(inet_ntoa(clientAddress.sin_addr));
    clientList[id-1].setName("(no name)");
    clientList[id-1].setPort(ntohs(clientAddress.sin_port));
    clientList[id-1].setFD(ssock);
    clientList[id-1].setID(id);
    // clientList[id-1].setPid(pid);

    // welcome message
    // int thisSock = clientList[id-1].getFD();
    // ss.str("");
    // ss.clear();
    cout << "****************************************" << endl;
    cout << "** Welcome to the information server. **" << endl;
    cout << "****************************************" << endl;
    // clientPrintMsg(ss.str().c_str(), thisSock);

    // login message => broadcast
    ss.str("");
    ss.clear();
    ss << "*** User '" << clientList[id-1].getName() << "' entered from " << clientList[id-1].getIP() << ":" << clientList[id-1].getPort() << ". ***" << endl;
    clientYell(ss.str().c_str(), clientList, maxClientNum);

    // show prompt on client
    // showCommandPrompt();
    // clientPrintMsg("% ", thisSock);
}

void MultiClientShell::clientYell(const char *msg, MultiClientInfo *clientList, int maxClientNum) {
    // cout << "Enter clientYell" << endl;
    int clientSock;
    for (int i=0; i<maxClientNum; i++) {
        if (clientList[i].getIsActive()) {
            // clientSock = clientList[usrId].getFD();            
            // clientPrintMsg(msg, clientSock);
            clientList[i].setIsClientPrintMsg(true);
            // cout << "msg: " << msg;
            clientList[i].setMsg(msg);
            kill(clientList[i].getPid(), SIGUSR1);
        }
    }
}

void MultiClientShell::clientLogOut(int id, MultiClientInfo *clientList, int maxClientNum) {
    int thisID = id;

    for (map<int,ShellPipe>::iterator iter=pipeList.begin(); iter!=pipeList.end(); ++iter) {
        iter->second.closeWholePipe();
    }
    for (map<int,ShellPipe>::iterator iter=lineList.begin(); iter!=lineList.end(); ++iter) {
        iter->second.closeWholePipe();
    }
    // for (map<pair<int,int>, ShellPipe>::iterator iter=clientPipe->begin(); iter!=clientPipe->end(); ++iter) {
    //     // From 
    //     if (iter->first.first == thisID) {
    //         iter->second.closeWholePipe();
    //         iter->second.setIsReadByClient(true);
    //     }
    //     // To
    //     if (iter->first.second == thisID) {
    //         iter->second.closeWholePipe();
    //         iter->second.setIsReadByClient(true);
    //     }
    // }

    // cout << "Enter clientLogOut" << endl;
    stringstream ss;
    // logout message => broadcast
    ss.str("");
    ss.clear();
    ss << "*** User '" << clientList[id-1].getName() << "' left. ***" << endl;
    clientYell(ss.str().c_str(), clientList, maxClientNum);


    clientList[id-1].setIsActive(false);
    // lineId = -1;    
    cout << "cp 1" << endl;
    lineList.clear();
    pipeList.clear();
    shutdown(0, SHUT_RDWR);
    cout << "cp 2" << endl;
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    // close(clientList[id-1].getFD());
 
    // clientEnvVar.clear();
    
    cout << "cp 3" << endl;
    
}
