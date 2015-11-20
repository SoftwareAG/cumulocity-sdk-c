#ifndef SRTIMER_H
#define SRTIMER_H
#include <utility>
#include <time.h>
#include "srtypes.h"

class SrTimer;
class SrAgent;

/**
 *  \brief Comparison operator for timespec.
 *  \param l reference to timespec lhs.
 *  \param r reference to timespec rhs.
 *  \return true if l smaller than or equal to r, false otherwise.
 */
bool operator<=(const timespec &l, const timespec &r);

class AbstractTimerFunctor
{
public:
        virtual ~AbstractTimerFunctor() {}
        /**
         *  \brief Timer callback interface.
         *  \param timer reference to the timer that fires the callback.
         *  \param agent reference to the SrAgent instance.
         */
        virtual void operator()(SrTimer &timer, SrAgent &agent) = 0;
};


class SrTimer
{
public:
        /**
         *  \brief SrTimer Constructor.
         *
         *  Beware that the timer is inactive when first created.
         *  \param millisec timer period in milliseconds.
         *  \param callback functor to be executed when the timer fires.
         *  \return return type
         */
        SrTimer(int millisec, AbstractTimerFunctor *callback = NULL):
                cb(callback), val(millisec), active(false) {}
        virtual ~SrTimer() {}

        /**
         *  \brief Query if the timer is active.
         *  \return true if active, false otherwise.
         */
        bool isActive() const { return active; }
        /**
         *  \brief Get the current period of the timer.
         *  \return the current timer period in milliseconds.
         */
        int interval() const { return val; }
        /**
         *  \brief Get the schedule time of the timer.
         *  \return the schedule time. Meaningless if tiemr is inactive.
         */
        const timespec &shedTime() const { return beg; }
        /**
         *  \brief Get the fire time of the timer.
         *  \return the schedule time. Meaningless if tiemr is inactive.
         */
        const timespec &fireTime() const { return end; }
        /**
         *  \brief Run the connected callback if not NULL.
         *  \param agent reference to the SrAgent instance.
         */
        void run(SrAgent &agent) { if (cb) (*cb)(*this, agent); }
        /**
         *  \brief Set the period for the timer.
         *  \param millisec the new period in milliseconds. Calling this
         *  function with a negative interval causing undefined behavior.
         */
        void setInterval(int millisec) { val = millisec; }
        /**
         *  \brief Connect callback functor to the timer.
         *
         *  The functor is called when the timer is fired. The old callback is
         *  overwritten if any. Calling this function with NULL resets the
         *  callback.
         *
         *  \param functor A subclass of AbstractTimerFunctor.
         */
        void connect(AbstractTimerFunctor *functor) { cb = functor; }
        /**
         *  \brief Starts the timer.
         *
         *  Sets the timer to active and the schedule time to now.
         */
        void start() {
                clock_gettime(CLOCK_MONOTONIC, &beg);
                end.tv_sec = beg.tv_sec + val / 1000;
                end.tv_nsec = beg.tv_nsec + (val % 1000) * 1000000;
                active = true;
        }
        /**
         *  \brief Stops the timer.
         *
         *  Sets the timer to inactive.
         */
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
