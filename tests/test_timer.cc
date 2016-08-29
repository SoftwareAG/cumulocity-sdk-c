#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sragent.h>
using namespace std;

const int val = 100;
const int N = 1000 * 1000000;
const int F = val * 1000000;


class Callback: public AbstractTimerFunctor
{
public:
        Callback() {}
        void operator()(SrTimer &timer, SrAgent &agent) {
                const timespec &st = timer.shedTime();
                timespec ts;
                clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
                assert((ts.tv_sec-st.tv_sec) * N + (ts.tv_nsec-st.tv_nsec) >= F);
                cerr << "OK!" << endl;
                exit(0);
        }
        virtual ~Callback() {}
private:
};



int main()
{
        cerr << "Test SrTimer: ";
        SrAgent agent("", "", NULL, NULL);
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
        Callback callback;
        SrTimer timer(val, &callback);
        timer.start();
        agent.addTimer(timer);
        agent.loop();
        return 0;
}
