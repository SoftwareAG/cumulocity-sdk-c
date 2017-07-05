/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
    const timespec ts =
    { 10, 0 };
    set<string> &s = (*(set<string>*) arg);
    for (int i = 0; i < N;)
    {
        auto e = Q.get(timeout);
        if (e.second == 0)
        {
            ++i;
            s.emplace(e.first);
        }
    }

    return NULL;
}

int main()
{
    cerr << "Test SrQueue: ";

    set<string> s = { "hello", "World!", "haha", "abc", "eee", "" };
    set<string> S;
    set<string> S2;
    N = s.size();
    pthread_t tid;
    pthread_create(&tid, NULL, &func, &S);
    const timespec ts = { 10, 0 };

    for (auto &e : s)
    {
        if (Q.put(e) == 0)
        {
            S2.emplace(e);
        }
    }

    pthread_join(tid, NULL);
    assert(S == S2);
    cerr << "OK!" << endl;

    return 0;
}
