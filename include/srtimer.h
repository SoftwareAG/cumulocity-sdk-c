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

/**
 *  \class SrTimerHandler
 *  \brief Virtual abstract functor for the SrTimer callback interface.
 */
class SrTimerHandler
{
public:
    virtual ~SrTimerHandler()
    {
    }

    /**
     *  \brief SrTimer callback interface.
     *  \param timer reference to the SrTimer which fires the callback.
     *  \param agent reference to the SrAgent instance.
     */
    virtual void operator()(SrTimer &timer, SrAgent &agent) = 0;
};

typedef SrTimerHandler AbstractTimerFunctor;

/**
 *  \class SrTimer
 *  \brief A periodical timer with millisecond resolution.
 */
class SrTimer
{
public:
    /**
     *  \brief SrTimer constructor.
     *
     *  \note the timer is inactive when first constructed.
     *  \param millisec timer period in milliseconds.
     *  \param callback functor to be executed when the timer fires.
     */
    SrTimer(int millisec, SrTimerHandler *callback = NULL) :
            cb(callback), val(millisec), active(false)
    {
    }
	
    virtual ~SrTimer()
    {
    }

    /**
     *  \brief Check if the timer is active.
     *  \return true if active, false otherwise.
     */
    bool isActive() const
    {
        return active;
    }
	
    /**
     *  \brief Get the current period of the timer.
     *  \return the current timer period in milliseconds.
     */
    int interval() const
    {
        return val;
    }
	
    /**
     *  \brief Get the schedule time of the timer.
     *  \return the schedule time, undefined if timer is inactive.
     */
    const timespec &shedTime() const
    {
        return beg;
    }
	
    /**
     *  \brief Get the fire time of the timer.
     *  \return the schedule time, undefined if timer is inactive.
     */
    const timespec &fireTime() const
    {
        return end;
    }
	
    /**
     *  \brief Run the connected callback if not NULL.
     *  \param agent reference to the SrAgent instance.
     */
    void run(SrAgent &agent)
    {
        if (cb)
        {
            (*cb)(*this, agent);
        }
    }
	
    /**
     *  \brief Set the period to millisec for the timer.
     *
     *  \note This function does not activate the timer.
     *  \note Set a negative interval causes undefined behavior.
     *
     *  \param millisec the new period in milliseconds.
     */
    void setInterval(int millisec)
    {
        val = millisec;
    }
	
    /**
     *  \brief Connect callback functor to the timer.
     *
     *  The functor is called when the timer is fired. The old callback is
     *  overwritten if any. Calling this function with NULL resets the
     *  callback.
     *
     *  \param functor A subclass of SrTimerHandler.
     */
    void connect(SrTimerHandler *functor)
    {
        cb = functor;
    }
	
    /**
     *  \brief Start the timer.
     *
     *  This function activates the timer, sets the schedule time to now,
     *  and the fire time according to the current period.
     */
    void start()
    {
        clock_gettime(CLOCK_MONOTONIC_COARSE, &beg);
        end.tv_sec = beg.tv_sec + val / 1000;
        end.tv_nsec = beg.tv_nsec + (val % 1000) * 1000000;

        if (end.tv_nsec >= 1000000000)
        {
            ++end.tv_sec;
            end.tv_nsec -= 1000000000;
        }

        active = true;
    }
	
    /**
     *  \brief Stop the timer. Sets the timer to inactive.
     */
    void stop()
    {
        active = false;
    }

private:

    SrTimerHandler *cb;
    timespec beg;
    timespec end;
    int val;
    bool active;
};

#endif /* SRTIMER_H */
