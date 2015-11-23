#include <pthread.h>
#include "srlogger.h"

void *foo(void *arg)
{
        (void)arg;
        srInfo("thread: Hello world");
        srInfo("thread: mamamia!");
        srNotice("thread: a notice.");
        return NULL;
}


int main()
{
        srLogSetLevel(SRLOG_INFO);
        srLogSetDest("a.txt");
        srLogSetQuota(30);
        pthread_t tid1;
        pthread_create(&tid1, NULL, foo, NULL);
        srInfo("main: Hello world");
        srInfo("main: mamamia!");
        srNotice("main: a notice.");
        pthread_join(tid1, NULL);
        return 0;
}
// g++ -pthread -std=c++11 test_srlogger.cc srlogger.cc
