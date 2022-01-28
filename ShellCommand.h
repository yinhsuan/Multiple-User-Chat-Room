#include <iostream>
#include <vector>
using namespace std;

enum CommandStdIOState {
    ToStdOut = 0,
	ToStdOutPipe = 1,
	ToStdOutandStdErrPipe = 2,
	ToFile = 3,
    ToStdOutNumberPipe = 4,
    ToStdOutandStdErrNumberPipe = 5,
    ToNumberCat = 6, // ONLY stdout
    // FromNumberCat = 7,
    // TwoWayNumberCat = 8
};

// for npshellSplitLine status
enum CommandStatus {
    IsMain = 0,
    IsArgv = 1,
    IsFile = 2,
    IsCat = 3,
    IsTwoWayCat = 4,
    IsDone = 5
};

class ShellCommand {
    private:
        string commandName;
        vector<string> Argv;
        int commandId;
        CommandStdIOState ioState;
        bool isEnvVariable;
        string dstFileName;
        int eachLineId;
        int numberPipeNum;
        int toNumberCatNo;
        int fromNumberCatNo;
        // int fromClientName;
        bool isReadFromNumberCat; // need to input from user pipe

    public:
        ShellCommand();
        ShellCommand(string _commandName, int _inId);
        ~ShellCommand();

        void setCommandName(string _inName);
        string getCommandName();

        void addCommandArgv(string _inArg);
        vector<string> getCommandArgv();

        void setCommandId(int _inId);
        int getCommandId();

        void setIOState(CommandStdIOState _inIOState);
        CommandStdIOState getIOState();

        void setIsEnvVariable(bool _inIsEnv);
        bool getIsEnvVariable();

        void setDstFileName(string _inDstFileName);
        string getDstFileName();

        void setEachLineId(int _inLineId);
        int getEachLineId();

        void setNumberPipeNum(int _inNumberPipeNum);
        int getNumberPipeNum();

        void setToNumberCatNo(int _inToNumberCatNo);
        int getToNumberCatNo();

        void setFromNumberCatNo(int _inFromNumberCatNo);
        int getFromNumberCatNo();

        void setIsReadFromNumberCat(bool _inIsReadFromNumberCat);
        bool getIsReadFromNumberCat();
};