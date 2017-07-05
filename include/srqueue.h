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

#ifndef SRQUEUE_H
#define SRQUEUE_H

#include <queue>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

/**
 *  \class SrQueue
 *  \brief Multi-thread communication queue.
 *
 *  SrQueue is a consumer/producer queue for multi-thread communication.
 *  It uses the mutex facility from pthread to implement atomic get and
 *  put operations, and semaphore to avoid busy waiting.
 */
template<typename T> class SrQueue
{
public:
    /**
     *  \brief Enumeration of all possible error code.
     */
    enum ErrCode
    {
        Q_OK = 0, Q_TIMEOUT, Q_BUSY, Q_EMPTY, Q_NOTIME
    };
    /**
     *  \brief Event wraps the element type and an error code.
     *
     *  The error code must be first checked before access the element T.
     *  When the error code is not 0, element T is default constructed,
     *  thus element T requires a default constructor.
     */
    typedef std::pair<T, ErrCode> Event;
    SrQueue() :
            q()
    {
        mutex = PTHREAD_MUTEX_INITIALIZER;
        memset(&sem, 0, sizeof(sem));
        sem_init(&sem, 0, 0);
    }

    virtual ~SrQueue()
    {
        sem_destroy(&sem);
        pthread_mutex_destroy(&mutex);
    }
    /**
     *  \brief get an element from the queue.
     *
     *  This a blocking call, it never returns until the underlying semaphore
     *  signals that there is at least one element available in the queue.
     *  \note You must still check the error code after this function
     *  returns, as the mutex locking may fail, or another thread may have
     *  retrieved the element first.
     *
     *  \return the element T with error code.
     */
    Event get()
    {
        sem_wait(&sem);
        Event e;

        if (pthread_mutex_lock(&mutex) == 0)
        {
            if (q.empty())
            {
                e.second = Q_EMPTY;
            } else
            {
                e = std::make_pair(q.front(), Q_OK);
                q.pop();
            }
            pthread_mutex_unlock(&mutex);
        } else
        {
            e.second = Q_BUSY;
        }

        return e;
    }

    /**
     *  \brief get an element from the queue.
     *
     *  Similar to the get function with no parameter, except this function
     *  waits at most milliseconds instead of waiting forever. This
     *  function can fail additionally when timed out. Calling this function
     *  with negative value causes undefined behaviour.
     *
     *  \return the element T with error code.
     */
    Event get(int millisec)
    {
        timeval tv = { 0, 0 };
        gettimeofday(&tv, NULL);
        timeval delta = { millisec / 1000, (millisec % 1000) * 1000 };
        timeval res;
        timeradd(&tv, &delta, &res);
        const timespec tp = {   res.tv_sec, res.tv_usec * 1000};
        Event e;

        // Testing for q.empty() is because for some version of
        // sem_timedwait could timeout and return with -1 while
        // sem_getvalue() is actual non-0.
        if ((sem_timedwait(&sem, &tp) == -1) && q.empty())
        {
            e.second = Q_TIMEOUT;
            return e;
        }

        if (pthread_mutex_trylock(&mutex) == 0)
        {
            if (q.empty())
            {
                e.second = Q_EMPTY;
            } else
            {
                e = std::make_pair(q.front(), Q_OK);
                q.pop();
            }
            pthread_mutex_unlock(&mutex);
        } else
        {
            e.second = Q_BUSY;
        }

        return e;
    }

    /**
     *  \brief put element item into the queue.
     *
     *  \param item the element to put into the queue.
     *  \return 0 on success, -1 otherwise.
     */
    int put(const T& item)
    {
        if (pthread_mutex_lock(&mutex) == 0)
        {
            q.push(item);

            pthread_mutex_unlock(&mutex);
            sem_post(&sem);

            return 0;
        }
        return -1;
    }

    /**
     *  \brief get the number of elements in the queue.
     *
     *  \note This function is not thread-safe, it should only be used as a
     *  hint rather than an accurate measure.
     *
     *  \return the number of elements currently in the queue.
     */
    size_t size() const
    {
        return q.size();
    }

    /**
     *  \brief check if the queue is empty.
     *
     *  \note This function is not thread-safe, it should only be used as a
     *  hint rather than an accurate measure.
     *
     *  \return true if the queue is empty, false otherwise.
     */
    bool empty() const
    {
        return q.empty();
    }

private:

    std::queue<T> q;
    sem_t sem;
    pthread_mutex_t mutex;
};

#endif /* SRQUEUE_H */
