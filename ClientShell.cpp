#include "ClientShell.h"

// void showCommandPrompt() {
// 	cout << "% "; 
// }

ClientShell::ClientShell() {
    // clientInfo = new ClientInfo();
    int lineId = -1;
}

ClientShell::~ClientShell() {;}

ClientInfo ClientShell::getClientInfo() {
    return clientInfo;
}

// void ClientShell::broadcastToAll() {

// }


string ClientShell::npshellReadLine() {
	string line;
	getline(cin, line);
	return line;
}

ProcessMode ClientShell::npshellSplitLine(string inputLine, vector<ShellCommand> *commandList, int lineId) {
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
                    // cout << "check point 1" << endl;
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
                // ToNumberCat       
                // cStatus = (cStatus == IsCat) ? IsTwoWayCat : IsCat; 
                // if (cStatus == IsTwoWayCat) {     
                //     // cout << "check point 2" << endl;    
                //     commandList->at(commandId).setIOState(TwoWayNumberCat);
                // }
                // else {     
                //     // cout << "check point 3" << endl;    
                //     commandList->at(commandId).setIOState(ToNumberCat);
                // }
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
            // cout << "enter <" << endl;
            // FromNumberCat
            // cStatus = (cStatus == IsCat) ? IsTwoWayCat : IsCat;
            // if (cStatus == IsTwoWayCat) {
            //     // cout << "check point 4" << endl; 
            //     commandList->at(commandId).setIOState(TwoWayNumberCat);
            // }
            // else {
            //     // cout << "check point 5" << endl; 
            //     commandList->at(commandId).setIOState(FromNumberCat);
            // }
            string tmp = parseWord.substr(1);
            int parseNumberCatNum = 0;
            parseNumberCatNum = atoi(tmp.c_str());
            commandList->at(commandId).setFromNumberCatNo(parseNumberCatNum);
            commandList->at(commandId).setIsReadFromNumberCat(true);
            // commandList->at(commandId).setToNumberCatNo(clientInfo.getID());
            pMode = ToWait; // if ToWait => stock QQ
        }
		else if (cStatus == IsMain) {
                 
            // cout << "check point 6" << endl;    
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
			commandList->at(commandId).addCommandArgv(parseWord);
		} 
		else if (cStatus == IsFile) {
			commandList->at(commandId).setDstFileName(parseWord);
			cStatus = IsDone;
		} 
	}
	return pMode;
}

pid_t ClientShell::npshellExeCommand(ShellCommand command, map<int, ShellPipe> *pipelist, map<int, ShellPipe> *lineList, map<pair<int,int>, ShellPipe> *clientPipe, ClientShell *clientList, int maxClientNum) {
	pid_t pid;
	int commandId = command.getCommandId();
	int thisLineId = command.getEachLineId();
	int numberPipeNum = command.getNumberPipeNum();
	int stdInRedirectNo = 0;
	int stdOutRedirectNo = 0;
	int stdErrRedirectNo = 0;
	char* fileName = "";
	CommandStdIOState cStatus = command.getIOState();
    int thisFD = clientInfo.getFD();
    int fromClientNo = 0;
    int toClientNo = 0;
    stringstream ss;
    string senderName = "";
    string receiverName = "";
    int thisSock = clientInfo.getFD();
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
        // cout << "check point 0" << endl;
		stdInRedirectNo = lineList->at(thisLineId).getFdRead();
	}

    // FromNumberCat
    if (command.getIsReadFromNumberCat()) {
        fromClientNo = command.getFromNumberCatNo();
        toClientNo = clientInfo.getID();
        if (1 > fromClientNo || fromClientNo > 30 || !clientList[fromClientNo-1].getClientInfo().getIsActive()) {
            ss << "*** Error: user #" << fromClientNo << " does not exist yet. ***" << endl;
            clientPrintMsg(ss.str().c_str(), thisSock);
            // return -1;
            stdInRedirectNo = nullFD;
        }
        else if (clientPipe->count(pair<int, int>(fromClientNo, toClientNo)) == 0) {
            // cout << "check point 3" << endl;
            // ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " does not exist yet. ***" << endl;
            ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " does not exist yet. ***" << endl;
            clientPrintMsg(ss.str().c_str(), thisSock);
            // return -1;
            stdInRedirectNo = nullFD;
        }
        else if (clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).getIsReadByClient()) {
            // cout << "check point 4" << endl;
            ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " does not exist yet. ***" << endl;
            clientPrintMsg(ss.str().c_str(), thisSock);
            // return -1;
            stdInRedirectNo = nullFD;
        }
        else {
            // cout << "check point 5" << endl;
            stdInRedirectNo = clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).getFdRead();
            clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).setIsReadByClient(true);

            // read from user pipe => broadcast
            // ss.str("");
            // ss.clear();
            senderName = clientList[fromClientNo-1].clientInfo.getName();
            ss << "*** " << clientInfo.getName() << " (#" << clientInfo.getID()  << ")" << " just received from " <<  senderName << " (#" << fromClientNo << ")" << " by '" << wholeLineCommand << "' ***" << endl;
            clientYell(ss.str().c_str(), clientList, maxClientNum);
        }
    }

    ss.str("");
    ss.clear();
	// 2. specify this command output
	switch(cStatus) {
		case ToStdOut:
            // cout << "check point 0" << endl;
			break;
		case ToStdOutPipe:
            // cout << "check point 1" << endl;
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
            fromClientNo = clientInfo.getID();
            toClientNo = command.getToNumberCatNo();
            // cout << "clientPipe->size() in ToNumberCat: " << clientPipe->size() << endl;
            if (1 > toClientNo || toClientNo > 30 || !clientList[toClientNo-1].getClientInfo().getIsActive()) {
                ss << "*** Error: user #" << toClientNo << " does not exist yet. ***" << endl;
                clientPrintMsg(ss.str().c_str(), thisSock);
                // return -1;
                stdOutRedirectNo = nullFD;
            }
            else if (clientPipe->count(pair<int, int>(fromClientNo, toClientNo)) == 0) {
                // cout << "check point 1" << endl;
                clientPipe->insert(pair<pair<int, int>, ShellPipe>(pair<int, int>(fromClientNo, toClientNo), ShellPipe()));
                clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).openWholePipe();
                clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).setIsReadByClient(false);
                stdOutRedirectNo = clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).getFdWrite();

                senderName = clientList[fromClientNo-1].clientInfo.getName();
                receiverName = clientList[toClientNo-1].clientInfo.getName();
                ss << "*** " << senderName << " (#" << fromClientNo << ")" << " just piped '" << wholeLineCommand << "' to " << receiverName << " (#" << toClientNo << ") ***" << endl;
                clientYell(ss.str().c_str(), clientList, maxClientNum);
            }
            else if (clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).getIsReadByClient()) {
                // cout << "check point 1" << endl;
                clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).openWholePipe();
                clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).setIsReadByClient(false);
                stdOutRedirectNo = clientPipe->at(pair<int, int>(fromClientNo, toClientNo)).getFdWrite();

                senderName = clientList[fromClientNo-1].clientInfo.getName();
                receiverName = clientList[toClientNo-1].clientInfo.getName();
                ss << "*** " << senderName << " (#" << fromClientNo << ")" << " just piped '" << wholeLineCommand << "' to" << receiverName << " (#" << toClientNo << ") ***" << endl;
                clientYell(ss.str().c_str(), clientList, maxClientNum);
            }
            else {
                // cout << "check point 2" << endl;
                ss << "*** Error: the pipe #" << fromClientNo << "->#" << toClientNo << " already exists. ***" << endl;
                clientPrintMsg(ss.str().c_str(), thisSock);
                // return -1;
                stdOutRedirectNo = nullFD;
            }
            break;
	}	
    // cout << "check point 2" << endl;

	while((pid = fork()) < 0) {
		usleep(100);
	}

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
                // cout << "check point 4" << endl;
				pipelist->at(commandId).closeWholePipe();
			} 
			else if (lineList->count(thisLineId) != 0) {
                // cout << "check point 5" << endl;
				lineList->at(thisLineId).closeWholePipe();
			}
            // fromClientNo & toClientNo already given value on previous switch
            else if (clientPipe->count(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())) != 0) {
                clientPipe->at(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())).closeWholePipe();
                clientPipe->at(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())).setIsReadByClient(true);
            }
		}
        else {
            // cout << "stdInRedirectNo: " << stdInRedirectNo << endl;
            // cout << "check point 6" << endl;
            dup2(thisFD, STDIN_FILENO);
        }
        // -------- STDOUT -------
		if (stdOutRedirectNo) {
            // cout << "stdOutRedirectNo: " << stdOutRedirectNo << endl;
			dup2(stdOutRedirectNo, STDOUT_FILENO);
			if (stdOutRedirectNo != stdErrRedirectNo) {
				close(stdOutRedirectNo);
			}			
		}
        else {
            // cout << "stdOutRedirectNo: " << stdOutRedirectNo << endl;
            dup2(thisFD, STDOUT_FILENO);
        }
        // -------- STDERR -------
		if (stdErrRedirectNo) {
            // cout << "stdErrRedirectNo" << endl;
			dup2(stdErrRedirectNo, STDERR_FILENO);
			close(stdErrRedirectNo);
		}
        else {
            dup2(thisFD, STDERR_FILENO);
        }
        // cout << "check point 7" << endl;
        close(thisFD);
       

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
				pipelist->at(commandId).closeWholePipe();
				pipelist->erase(commandId);
			} 
			else if (lineList->count(thisLineId) != 0) {
				lineList->at(thisLineId).closeWholePipe();
				lineList->erase(thisLineId);
			}	
            else if (clientPipe->count(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())) != 0) {
                clientPipe->at(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())).closeWholePipe();
                clientPipe->at(pair<int, int>(command.getFromNumberCatNo(), clientInfo.getID())).setIsReadByClient(true);
            }
		}
	}
	return pid;
}


void ClientShell::setEnvironmentVar(vector<ShellCommand> cList) {
	string cName = cList[0].getCommandName();
	vector<string> argv = cList[0].getCommandArgv();
    stringstream ss;
    int thisSock = clientInfo.getFD();
	
	if (cName == "setenv") {
        setClientEnvVar(argv[1].c_str(), argv[2].c_str());
		// setenv(argv[1].c_str(), argv[2].c_str(), 1);
	}
	else if (cName == "printenv") {
		char *env = getenv(argv[1].c_str());
		if (env != NULL) {
			// cout << env << endl;
            ss << env << endl;
            clientPrintMsg(ss.str().c_str(), thisSock);
		}
	}
	else {
		cout << "Not a Environment Variable" << endl;
	}
}


void ClientShell::resetEnvVar() {
    // setenv("PATH", "bin:.", 1); // 1: overwrite; 0: not overwrite
    for (map<string,string>::iterator iter=clientEnvVar.begin(); iter!=clientEnvVar.end(); ++iter) {
        setenv(iter->first.c_str(), iter->second.c_str(), 1);
    }
}

void ClientShell::setClientEnvVar(const char *env, const char *val) {
    setenv(env, val, 1);
    if (clientEnvVar.count(string(env)) == 0) {
        clientEnvVar.insert(pair<string, string>(string(env), string(val)));
    }
    else {
        clientEnvVar[string(env)] = string(val);  
    }      
}


void ClientShell::exeChatCommand(vector<ShellCommand> cList, ClientShell *clientList, int maxClientNum) {
    string cName = cList[0].getCommandName();
	vector<string> argv = cList[0].getCommandArgv();
    stringstream ss;
    int thisSock = clientInfo.getFD();
    ClientInfo cInfo;

    if (cName == "who") {
        ss << "<ID>" << "\t" << "<nickname>" << "\t" << "<IP:port>" << "\t" << "<indicate me>" << endl;
        for (int cId=0; cId<maxClientNum; cId++) {
            cInfo = clientList[cId].getClientInfo();
            if (cInfo.getIsActive()) {
                ss << cInfo.getID() << "\t" << cInfo.getName() << "\t" << cInfo.getIP() << ":" << cInfo.getPort();
                if ((cId+1) == clientInfo.getID()) {
                    ss << "\t" << "<-me";
                }
                ss << endl;
            }            
        }
        clientPrintMsg(ss.str().c_str(), thisSock);
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
            cInfo = clientList[cId].getClientInfo();
            if (cInfo.getID() == clientInfo.getID()) {
                continue;
            }
            if (cInfo.getName().compare(newName) == 0) {
                ss << "*** User '" << newName << "' already exists. ***" << endl;
                clientPrintMsg(ss.str().c_str(), thisSock);
                return;
            }
        }
        // set new name
        clientInfo.setName(newName);
        // broadcast to all client
        ss << "*** User from " << clientInfo.getIP() << ":" << clientInfo.getPort() << " is named '" << newName << "'. ***" << endl;
        clientYell(ss.str().c_str(), clientList, maxClientNum);
    }
    else if (cName == "yell") {
        string yellMsg = "";
        for (int i=1; i<argv.size()-1; i++) {
            yellMsg += argv[i];
            yellMsg += " ";
        }
        yellMsg += argv[argv.size()-1];
        ss << "*** " << clientInfo.getName() << " yelled ***: " << yellMsg << endl;
        clientYell(ss.str().c_str(), clientList, maxClientNum);
    }
    else if (cName == "tell") {
        // check if the receiver exists
        cInfo = clientList[stoi(argv[1])-1].clientInfo;
        if (!cInfo.getIsActive()) {
            ss << "*** Error: user #" << stoi(argv[1]) << " does not exist yet. ***" << endl;
            clientPrintMsg(ss.str().c_str(), thisSock);
            return;
        }
        string tellMsg = "";
        for (int i=2; i<argv.size()-1; i++) {
            tellMsg += argv[i];
            tellMsg += " ";
        }
        tellMsg += argv[argv.size()-1];
        ss << "*** " << clientInfo.getName() << " told you ***: " << tellMsg << endl;
        clientPrintMsg(ss.str().c_str(), cInfo.getFD());
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

ProcessMode ClientShell::executeShell(string inputLine, ClientShell *clientList, map<pair<int,int>, ShellPipe> *clientPipe, int maxClientNum) {
    ProcessMode pStatus = NoWait;
	vector<ShellCommand> commandList; // for each line commands
	// map<int, ShellPipe> pipeList; // for line command pipe
	// map<int, ShellPipe> lineList; // for number pipe
	pid_t lastCommand;
	pid_t middleCommand;
    int thisSock = clientInfo.getFD();
    wholeLineCommand = "";
	// int lineId = -1;
	// string inputLine;

	// -------------- Set environmental variable at the start -------------- 
    resetEnvVar();	 
    // signal(SIGCHLD, childHandler);
	// setenv("PATH", "bin:.", 1); // 1: overwrite; 0: not overwrite
	// signal(SIGCHLD, signalHandler);

	// Enter shell
	// showCommandPrompt();
	// while (getline(cin, inputLine)) {
    // -------------- Initial -------------- 
    commandList.clear();
    pipeList.clear();

    if (inputLine.empty()) {
        // cout << "Enter isEmpty" << endl;
        clientPrintMsg("% ", thisSock);
        return NoWait;
    }

    // if (!inputLine.empty()) {
    //     lineId++;
    // }
    lineId++;
    // -------------- Store each command in one line --------------
    pStatus = npshellSplitLine(inputLine, &commandList, lineId);
    if (pStatus == ToExit) {
        // return EXIT_SUCCESS;
        return pStatus;
    }
    else if (pStatus == ToEnvVar) {
        setEnvironmentVar(commandList);
    }
    else if (pStatus == ToChatCmd) {
        exeChatCommand(commandList, clientList, maxClientNum);
    }
    else {
        // -------------- Execute each command -------------- 
        for (vector<ShellCommand>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter) {
            // last command => wait
            lastCommand = npshellExeCommand(*iter, &pipeList, &lineList, clientPipe, clientList, maxClientNum);
        }
        //wait
        if (pStatus == ToWait && lastCommand > 0) {
            // cout << "waitpid" << endl;
            int status;
            waitpid(lastCommand, &status, 0);
        }
        // cout << "out of waitpid" << endl;
    }
	    // showCommandPrompt();
	// }
    // return pStatus;
    
    clientPrintMsg("% ", thisSock);
    wholeLineCommand.clear();
    return pStatus;
}






void ClientShell::clientPrintMsg(const char *msg, int ssock) {
    // cout << "Enter clientPrintMsg" << endl;
    if (send(ssock, msg, strlen(msg), 0) < 0) {
        cout << "send msg failed" << endl;
    }
}

void ClientShell::clientLogIn(int id, sockaddr_in clientAddress, ClientShell *clientList, int ssock, int maxClientNum) {
    // cout << "Enter clientLogIn" << endl;
    stringstream ss;
    clientInfo.setIsActive(true);
    setClientEnvVar("PATH", "bin:.");

    // set client info => ip/ port/ name/ fd/ id
    char ipStr[16];
    strcpy(ipStr, inet_ntoa(clientAddress.sin_addr));
    clientInfo.setIP(string(ipStr));
    clientInfo.setName("(no name)");
    clientInfo.setPort(ntohs(clientAddress.sin_port));
    clientInfo.setFD(ssock);
    clientInfo.setID(id);

    // welcome message
    int thisSock = clientInfo.getFD();
    ss.str("");
    ss.clear();
    ss << "****************************************" << endl;
    ss << "** Welcome to the information server. **" << endl;
    ss << "****************************************" << endl;
    clientPrintMsg(ss.str().c_str(), thisSock);

    // login message => broadcast
    ss.str("");
    ss.clear();
    ss << "*** User '" << clientInfo.getName() << "' entered from " << clientInfo.getIP() << ":" << clientInfo.getPort() << ". ***" << endl;
    clientYell(ss.str().c_str(), clientList, maxClientNum);

    // show prompt on client
    // showCommandPrompt();
    clientPrintMsg("% ", thisSock);
}

void ClientShell::clientYell(const char *msg, ClientShell *clientList, int maxClientNum) {
    // cout << "Enter clientYell" << endl;
    int clientSock;
    for (int usrId=0; usrId<maxClientNum; usrId++) {
        if (clientList[usrId].getClientInfo().getIsActive()) {
            clientSock = clientList[usrId].getClientInfo().getFD();            
            clientPrintMsg(msg, clientSock);
        }
    }
}

void ClientShell::clientLogOut(ClientShell *clientList, map<pair<int,int>, ShellPipe> *clientPipe, int maxClientNum) {
    int thisID = clientInfo.getID();

    for (map<int,ShellPipe>::iterator iter=pipeList.begin(); iter!=pipeList.end(); ++iter) {
        iter->second.closeWholePipe();
    }
    for (map<int,ShellPipe>::iterator iter=lineList.begin(); iter!=lineList.end(); ++iter) {
        iter->second.closeWholePipe();
    }
    for (map<pair<int,int>, ShellPipe>::iterator iter=clientPipe->begin(); iter!=clientPipe->end(); ++iter) {
        // From 
        if (iter->first.first == thisID) {
            iter->second.closeWholePipe();
            iter->second.setIsReadByClient(true);
        }
        // To
        if (iter->first.second == thisID) {
            iter->second.closeWholePipe();
            iter->second.setIsReadByClient(true);
        }
    }

    // cout << "Enter clientLogOut" << endl;
    stringstream ss;
    clientInfo.setIsActive(false);
    lineId = -1;
    shutdown(clientInfo.getFD(), SHUT_RDWR);
    close(clientInfo.getFD());
    clientEnvVar.clear();
    lineList.clear();
    pipeList.clear();

    // logout message => broadcast
    ss.str("");
    ss.clear();
    ss << "*** User '" << clientInfo.getName() << "' left. ***" << endl;
    clientYell(ss.str().c_str(), clientList, maxClientNum);
}