#include <iostream>
#include <cassert>
#include <unistd.h>
#include <srlogger.h>
using namespace std;


int main()
{
        cerr << "Test SrLogger rotate: ";
        const char *path = "tests/b.txt";
        const char *path2 = "tests/b.txt.1";
        unlink(path);
        unlink(path2);
        srLogSetLevel(SRLOG_DEBUG);
        srLogSetDest(path);
        srLogSetQuota(1);
        for (int i = 0; i < 20; ++i)
                srDebug("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
        bool flag = access(path, F_OK) == 0 && access(path2, F_OK) == 0;
        assert(flag);
        cerr << "OK!" << endl;
        unlink(path);
        unlink(path2);
        return 0;
}
