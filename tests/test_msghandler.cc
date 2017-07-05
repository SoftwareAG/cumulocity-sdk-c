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

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sragent.h>

using namespace std;

class Callback: public AbstractMsgHandler
{
public:
    Callback(const vector<string> &vec) :
            vec(vec)
    {
    }

    void operator()(SrRecord &r, SrAgent &agent)
    {
        std::vector<string> res;
        for (int i = 0; i < r.size(); ++i)
        {
            res.push_back(r[i].second);
        }

        assert(vec == res);
        cerr << "OK!" << endl;

        exit(0);
    }

    virtual ~Callback()
    {
    }

private:

    const vector<string> vec;
};

int main()
{
    cerr << "Test MSG Handler: ";

    SrAgent agent("", "", NULL, NULL);
    vector<string> vec = { "151", "329", "payload" };

    SrOpBatch op;
    for (auto &e : vec)
    {
        op.data += e + ",";
    }

    op.data.erase(op.data.size() - 1);
    Callback callback(vec);
    agent.addMsgHandler(151, &callback);
    agent.ingress.put(op);
    agent.loop();

    return 0;
}
