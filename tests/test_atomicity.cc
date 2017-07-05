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
    (void) arg;
    srInfo("thread: Hello world");
    srInfo("thread: mamamia!");
    srInfo("thread: a notice.");
    return NULL;
}

int main()
{
    cerr << "Test SrLogger atomicity: ";
    const char* const path = "tests/a.txt";

    unlink(path);
    srLogSetLevel(SRLOG_INFO);
    srLogSetDest(path);

    pthread_t tid;
    pthread_create(&tid, NULL, foo, NULL);
    srInfo("main: Hello world");
    srInfo("main: mamamia!");
    srInfo("main: a notice.");
    pthread_join(tid, NULL);

    vector<string> vec
    { "INFO: main: Hello world", "INFO: main: mamamia!",
            "INFO: main: a notice.", "INFO: thread: Hello world",
            "INFO: thread: mamamia!", "INFO: thread: a notice." };

    vector<string> res;
    {
        ifstream in(path);
        string line;
        while (getline(in, line))
        {
            res.push_back(line);
        }

        unlink(path);
    }

    for_each(res.begin(), res.end(), [](string &s)
    {
        s.erase(0, s.find("INFO: "));
    });

    sort(vec.begin(), vec.end());
    sort(res.begin(), res.end());

    assert(vec == res);
    cerr << "OK!" << endl;

    return 0;
}
