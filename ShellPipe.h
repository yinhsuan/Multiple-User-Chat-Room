#include <iostream>
#include <unistd.h>
#include <sys/types.h>
using namespace std;

class ShellPipe {
    private:
        int fd[2]; // fd[0]: read => OUT // fd[1]: write => IN
        // bool isOpen;
        bool isReadOpen;
        bool isWriteOpen;
        bool isReadByClient;

    public:
        ShellPipe();
        ~ShellPipe();

        int getFdRead();
        int getFdWrite();

        void openWholePipe();
        void closeWholePipe();

        void closeRead();
        void closeWrite();

        void setIsReadByClient(bool _inIsReadByClient);
        bool getIsReadByClient();
};