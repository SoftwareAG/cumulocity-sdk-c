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
                (TIMER_VALUE_IN_NS <= elapsedTime)
                        && (elapsedTime
                                <= (TIMER_VALUE_IN_NS
                                        + (SR_AGENT_VAL * MS_TO_NS))));

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
