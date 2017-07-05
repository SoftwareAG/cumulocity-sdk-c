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

#include <sragent.h>

#include <iostream>
#include <cstdlib>
#include <cassert>

using namespace std;

namespace
{
const int SECONDS_TO_NS = 1000 * 1000000; // ns
const int MS_TO_NS = 1 * 1000000; // ns

const int TIMER_VALUE_IN_MS = 20; // ms
const int TIMER_VALUE_IN_NS = (TIMER_VALUE_IN_MS * MS_TO_NS); // ns
}


class Callback: public AbstractTimerFunctor
{
public:
    Callback()
    {
    }

    void operator()(SrTimer &timer, SrAgent &agent)
    {
        const timespec &scheduledTime = timer.shedTime();

        timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &currentTime);

        // check time range with some tolerance (SR_AGENT_VAL) of upper limit
        const int elapsedTime = (currentTime.tv_sec - scheduledTime.tv_sec) * SECONDS_TO_NS + (currentTime.tv_nsec - scheduledTime.tv_nsec);

        // check condition
        assert(
                (TIMER_VALUE_IN_NS <= elapsedTime) &&
                 (elapsedTime <= (TIMER_VALUE_IN_NS + (SR_AGENT_VAL * MS_TO_NS))));

        cerr << "OK!" << endl;

        exit(0);
    }

    virtual ~Callback()
    {
    }

private:
};

int main()
{
    cerr << "Test SrTimer: ";
    SrAgent agent("", "", NULL, NULL);

    Callback callback;
    SrTimer timer(TIMER_VALUE_IN_MS, &callback);

    // add timer, start it and enter SrAgent loop
    agent.addTimer(timer);
    timer.start();
    agent.loop();

    return 0;
}
