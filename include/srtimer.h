#ifndef SRTIMER_H
#define SRTIMER_H
#include <utility>
#include <time.h>
#include "srtypes.h"

class SrTimer;
class SrAgent;

bool operator<=(const timespec &l, const timespec &r);

class AbstractTimerFunctor
{
public:
        virtual ~AbstractTimerFunctor() {}
        virtual void operator()(SrTimer &timer, SrAgent &agent) = 0;
};


class SrTimer
{
public:
        SrTimer(int millisec, AbstractTimerFunctor *callback = NULL):
                cb(callback), val(millisec), active(false) {}
        virtual ~SrTimer() {}

        bool isActive() const { return active; }
        int interval() const { return val; }
        const timespec &shedTime() const { return beg; }
        const timespec &fireTime() const { return end; }
        void run(SrAgent &agent) { if (cb) (*cb)(*this, agent); }
        void setInterval(int millisec) { val = millisec; }
        void connect(AbstractTimerFunctor *functor) { cb = functor; }
        void start() {
                clock_gettime(CLOCK_MONOTONIC, &beg);
                end.tv_sec = beg.tv_sec + val / 1000;
                end.tv_nsec = beg.tv_nsec + (val % 1000) * 1000000;
                active = true;
        }
        void stop() { active = false; }

private:
        typedef AbstractTimerFunctor*  Callback;
        Callback cb;
        timespec beg;
        timespec end;
        int val;
        bool active;
};

#endif /* SRTIMER_H */
