#include <string>
#include <set>
#include <iostream>
#include <cassert>
#include <srqueue.h>
using namespace std;

SrQueue<string> Q;
int N = 0;


void *func(void *arg)
{
        const int timeout = 200;
        const timespec ts = {10, 0};
        set<string> &s = (*(set<string>*)arg);
        for (int i = 0; i < N;) {
                auto e = Q.get(timeout);
                if (e.second == 0) {
                        ++i;
                        s.emplace(e.first);
                }
        }
        return NULL;
}


int main()
{
        cerr << "Test SrQueue: ";
        set<string> s = {"hello", "World!", "haha", "abc", "eee", ""};
        set<string> S;
        set<string> S2;
        N = s.size();
        pthread_t tid;
        pthread_create(&tid, NULL, &func, &S);
        const timespec ts = {10, 0};
        for (auto &e: s) {
                if (Q.put(e) == 0)
                        S2.emplace(e);
        }
        pthread_join(tid, NULL);
        assert(S == S2);
        cerr << "OK!" << endl;
        return 0;
}
