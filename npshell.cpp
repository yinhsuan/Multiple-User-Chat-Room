#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>
#include <map>
#include <fcntl.h>
#include <cstring>
#include <sys/wait.h>
#include <math.h>
#include "ShellCommand.cpp"
#include "ShellPipe.cpp"
using namespace std;

enum ProcessMode {
	NoWait = 0,
	ToWait = 1,
	ToExit = 2,
	ToEnvVar = 3
};

void showCommandPrompt() {
	cout << "% "; 
}

string npshellReadLine() {
	string line;
	getline(cin, line);
	return line;
}

ProcessMode npshellSplitLine(string inputLine, vector<ShellCommand> *commandList, int lineId) {
	vector<ShellCommand> tokens;
	stringstream ss(inputLine);
	string parseWord;
	ProcessMode pMode = ToWait;
	int commandId = -1;
	CommandStatus cStatus = IsMain;
	int parseNumberPipeNum = 0;

	while (ss >> parseWord) {
		if (parseWord == "exit") {
			pMode = ToExit;
			return pMode;
		}
		if (parseWord[0] == '|' || parseWord[0] == '!') {
			// Normal pipe
			if (parseWord.size() == 1) {
				if (parseWord[0] == '|') {
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
				parseNumberPipeNum = atoi(tmp.c_str());
				commandList->at(commandId).setNumberPipeNum(parseNumberPipeNum);	
				pMode = NoWait;
			}			
		} 
		else if (parseWord[0] == '>') {
			commandList->at(commandId).setIOState(ToFile);
			cStatus = IsFile;
		}
		else if (cStatus == IsMain) {
			if (parseWord == "setenv" || parseWord == "printenv") {
				pMode = ToEnvVar;
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

pid_t npshellExeCommand(ShellCommand command, map<int, ShellPipe> *pipelist, map<int, ShellPipe> *lineList) {
	pid_t pid;
	int commandId = command.getCommandId();
	int thisLineId = command.getEachLineId();
	int numberPipeNum = command.getNumberPipeNum();
	int stdInRedirectNo = 0;
	int stdOutRedirectNo = 0;
	int stdErrRedirectNo = 0;
	char* fileName = "";
	CommandStdIOState cStatus = command.getIOState();

	// 1. specify this command input
	if (pipelist->count(commandId) != 0) {
		stdInRedirectNo = pipelist->at(commandId).getFdRead();
	}

	if (commandId == 0 && lineList->count(thisLineId) != 0) {
		// lineList->at(thisLineId).openWholePipe(); => if add, then next line will be stocked
		stdInRedirectNo = lineList->at(thisLineId).getFdRead();
	}

	// 2. specify this command output
	switch(cStatus) {
		case ToStdOut:
			break;
		case ToStdOutPipe:
			if (pipelist->count(commandId+1) == 0) {
				pipelist->insert(pair<int, ShellPipe>(commandId+1, ShellPipe()));
				pipelist->at(commandId+1).openWholePipe();
			}
			stdOutRedirectNo = pipelist->at(commandId+1).getFdWrite();
			break;
		case ToStdOutandStdErrPipe:
			if (pipelist->count(commandId+1) == 0) {
				pipelist->insert(pair<int, ShellPipe>(commandId+1, ShellPipe()));
				pipelist->at(commandId+1).openWholePipe();
			}
			stdOutRedirectNo = pipelist->at(commandId+1).getFdWrite();
			stdErrRedirectNo = pipelist->at(commandId+1).getFdWrite();
			break;
		case ToFile:
			stdOutRedirectNo = open(command.getDstFileName().c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0666);
			break;
		case ToStdOutNumberPipe:
			if (lineList->count(thisLineId+numberPipeNum) == 0) {
				lineList->insert(pair<int, ShellPipe>(thisLineId+numberPipeNum, ShellPipe()));
				lineList->at(thisLineId+numberPipeNum).openWholePipe();
			}
			stdOutRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			break;
		case ToStdOutandStdErrNumberPipe:
			if (lineList->count(thisLineId+numberPipeNum) == 0) {
				lineList->insert(pair<int, ShellPipe>(thisLineId+numberPipeNum, ShellPipe()));
				lineList->at(thisLineId+numberPipeNum).openWholePipe();
			}
			stdOutRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			stdErrRedirectNo = lineList->at(thisLineId+numberPipeNum).getFdWrite();
			break;
	}	

	
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
		if (stdInRedirectNo) {
			dup2(stdInRedirectNo, STDIN_FILENO);
			close(stdInRedirectNo);
			
			if (pipelist->count(commandId) != 0) {
				pipelist->at(commandId).closeWholePipe();
			} 
			else if (lineList->count(thisLineId) != 0) {
				lineList->at(thisLineId).closeWholePipe();
			}			
		}
		if (stdOutRedirectNo) {
			dup2(stdOutRedirectNo, STDOUT_FILENO);
			if (stdOutRedirectNo != stdErrRedirectNo) {
				close(stdOutRedirectNo);
			}			
		}
		if (stdErrRedirectNo) {
			dup2(stdErrRedirectNo, STDERR_FILENO);
			close(stdErrRedirectNo);
		}
		const char * cName = command.getCommandName().c_str();
		char* arg[command.getCommandArgv().size()+1];

		for (int i=0; i < command.getCommandArgv().size(); i++){
			const char *tmp = command.getCommandArgv()[i].c_str();
			arg[i] = new char [command.getCommandArgv()[i].length()+1];
			strcpy(arg[i], tmp);
		}
		arg[command.getCommandArgv().size()] = NULL;
		int exeCmd = execvp(cName, arg);
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
		}
	}
	return pid;
}

void signalHandler(int receivedSignal) {
	// parent process call "wait" => be hanged
	// Clean recycle child process
	int status;
	pid_t pid;
	// WNOHANG: return immediately if no child has exited.
	while (waitpid(-1, &status, WNOHANG) > 0);
}

void setEnvironmentVar(vector<ShellCommand> cList) {
	string cName = cList[0].getCommandName();
	vector<string> argv = cList[0].getCommandArgv();
	
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

void executeNPshell() {
	ProcessMode pStatus = NoWait;
	vector<ShellCommand> commandList; // for each line commands
	map<int, ShellPipe> pipeList; // for line command pipe
	map<int, ShellPipe> lineList; // for number pipe
	pid_t lastCommand;
	pid_t middleCommand;
	int lineId = -1;
	string inputLine;

	// -------------- Set environmental variable at the start -------------- 	 
	setenv("PATH", "bin:.", 1); // 1: overwrite; 0: not overwrite
	signal(SIGCHLD, signalHandler);

	// Enter shell
	showCommandPrompt();
	while (getline(cin, inputLine)) {
		// -------------- Initial -------------- 
		commandList.clear();
		if (!inputLine.empty()) {
			lineId++;
		}
		// -------------- Store each command in one line --------------
		pStatus = npshellSplitLine(inputLine, &commandList, lineId);
		if (pStatus == ToExit) {
			// return EXIT_SUCCESS;
			break;
		}
		else if (pStatus == ToEnvVar) {
			setEnvironmentVar(commandList);
		}
		else {
			// -------------- Execute each command -------------- 
			for (vector<ShellCommand>::iterator iter = commandList.begin(); iter != commandList.end(); ++iter) {
				// last command => wait
				lastCommand = npshellExeCommand(*iter, &pipeList, &lineList);
			}
			//wait
			if (pStatus == ToWait) {
				int status;
				waitpid(lastCommand, &status, 0);
			}
		}
		showCommandPrompt();
	}
	// return EXIT_SUCCESS;
}