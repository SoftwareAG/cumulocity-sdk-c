#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sragent.h>
using namespace std;

const int val = 200;
const int N = 1000 * 1000000;
const int F = val * 1000000;


class Callback: public AbstractTimerFunctor
{
public:
        Callback(const timespec &start) {
                st.tv_sec = start.tv_sec;
                st.tv_nsec = start.tv_nsec;
        }
        void operator()(SrTimer &timer, SrAgent &agent) {
                timespec ts;
                clock_gettime(CLOCK_MONOTONIC, &ts);
                assert((ts.tv_sec-st.tv_sec) * N + (ts.tv_nsec-st.tv_nsec) >= F);
                cerr << "OK!" << endl;
                exit(0);
        }
        virtual ~Callback() {}
private:
        timespec st;
};



int main()
{
        cerr << "Test SrTimer: ";
        SrAgent agent("", "", NULL, NULL);
        timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        Callback callback(ts);
        SrTimer timer(val, &callback);
        timer.start();
        agent.addTimer(timer);
        agent.loop();
        return 0;
}
