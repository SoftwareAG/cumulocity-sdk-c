#include <iterator>
#include <fstream>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>
#include <pthread.h>
#include <unistd.h>
#include <srlogger.h>
using namespace std;


void *foo(void *arg)
{
        (void)arg;
        srInfo("thread: Hello world");
        srInfo("thread: mamamia!");
        srInfo("thread: a notice.");
        return NULL;
}


int main()
{
        cerr << "Test SrLogger atomicity: ";
        const char *path = "tests/a.txt";
        unlink(path);
        srLogSetLevel(SRLOG_INFO);
        srLogSetDest(path);
        pthread_t tid;
        pthread_create(&tid, NULL, foo, NULL);
        srInfo("main: Hello world");
        srInfo("main: mamamia!");
        srInfo("main: a notice.");
        pthread_join(tid, NULL);
        vector<string> vec{"INFO: main: Hello world", "INFO: main: mamamia!",
                        "INFO: main: a notice.", "INFO: thread: Hello world",
                        "INFO: thread: mamamia!", "INFO: thread: a notice."};
        vector<string> res;
        {
                ifstream in(path);
                string line;
                while (getline(in, line))
                        res.push_back(line);
                unlink(path);
        }
        for_each(res.begin(), res.end(),
                 [](string &s){s.erase(0, s.find("INFO: "));});
        sort(vec.begin(), vec.end());
        sort(res.begin(), res.end());
        assert(vec == res);
        cerr << "OK!" << endl;
        return 0;
}
