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

#ifndef SRWATCHDOGTIMER_H
#define SRWATCHDOGTIMER_H

#include <sys/shm.h>
#include <unistd.h>

#define SR_WDT_SIZE 8

/**
 *  \class SrWatchdogTimer
 *  \brief Watchdog timer.
 *
 *  SrWatchdogTimer is a simple and efficient software watchdog timer
 *  implementation. It must be used in couple with the srwatchdogd daemon for
 *  effect.
 */
class SrWatchdogTimer
{
public:

    SrWatchdogTimer() : shm(NULL), shmid(shmget(getppid(), SR_WDT_SIZE, 0666)), c(0)
    {
        shm = shmid != -1 ? (char*) shmat(shmid, NULL, 0) : shm;
        shm = shm == (char*) -1 ? NULL : shm;

        if (shm)
        {
            *shm = 1;
        }
    }

    virtual ~SrWatchdogTimer()
    {
        shmdt(shm);
    }

    /**
     *  \brief Start the watchdog timer.
     *  \return 0 on success, -1 on failure.
     */

    int start()
    {
        return shm ? 0 : -1;
    }

    /**
     *  \brief *Kicking the dog*. No effect when start() failed.
     */
    void kick()
    {
        if (shm)
        {
            shm[1] = c++;
        }
    }

private:

    char *shm;
    int shmid;
    char c;
};

#endif /* SRWATCHDOGTIMER_H */
