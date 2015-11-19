#include <string>
#include <cstring>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "srqueue.h"
using namespace std;


void *func(void *arg)
{
        SrQueue<string> *qptr = (SrQueue<string>*)arg;
        while (true) {
                SrQueue<string>::Event e = qptr->get();
                cout << "Thread [" << syscall(SYS_gettid) << "] ";
                if (e.second == 0)
                        cout << "Item: " << e.first << endl;
                else
                        cout << "Error: " << e.second << endl;
                // usleep(10);
        }
        return NULL;
}


int main()
{
        SrQueue<string> q;
        const int N = 5;
        pthread_t tid[N];
        for (size_t i = 0; i < N; ++i) {
                int err = pthread_create(&tid[i], NULL, &func, &q);
                if (err)
                        cerr << "Can not create thread: [%s]" << strerror(err);
        }

        for (size_t i = 0; i < 20; ++i) {
                if (q.put("hello"))
                        cerr << "Put item failed." << endl;
                usleep(10);
        }

        return 0;
}
