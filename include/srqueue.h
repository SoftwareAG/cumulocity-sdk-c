#ifndef SRQUEUE_H
#define SRQUEUE_H
#include <queue>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

/**
 *  \class SrQueue srqueue.h
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
        enum ErrCode {Q_OK = 0, Q_TIMEOUT, Q_BUSY, Q_EMPTY, Q_NOTIME};
        /**
         *  \brief Event wraps the element type and an error code.
         *
         *  The error code must be first checked before access the element T.
         *  When the error code is not 0, element T is default constructed,
         *  thus element T requires a default constructor.
         */
        typedef std::pair<T, ErrCode> Event;
        /**
         *  \brief SrQueue constructor.
         */
        SrQueue(): q() {
                if (pthread_mutex_init(&mutex, NULL))
                        std::cerr << "Mutex init failed.\n";
                if (sem_init(&sem, 0, 0))
                        std::cerr << "Semaphore init failed.\n";
        }
        virtual ~SrQueue() {
                sem_destroy(&sem);
                pthread_mutex_destroy(&mutex);
        }
        /**
         *  \brief get an element from the queue.
         *
         *  This a blocking call, it never returns until the underlying semaphore
         *  signals that there is at least one element available in the queue.
         *  Notice you still have to check the error code after this function
         *  returns, as the mutex locking could fail, or another thread may have
         *  retrieved the element first.
         *
         *  \return the element T with error code.
         */
        Event get() {
                sem_wait(&sem);
                Event e;
                if (pthread_mutex_lock(&mutex) == 0) {
                        if (q.size()) {
                                e = std::make_pair(q.front(), Q_OK);
                                q.pop();
                        } else {
                                e.second = Q_EMPTY;
                        }
                        pthread_mutex_unlock(&mutex);
                } else {
                        e.second = Q_BUSY;
                }
                return e;
        }
        /**
         *  \brief get an element from the queue.
         *
         *  Similar to the get function with no parameter, except this function
         *  waits at most millisec milliseconds instead of waiting forever. This
         *  function can fail additionally when timed out. Calling this function
         *  with negative value causes undefined behavior.
         *
         *  \return the element T with error code.
         */
        Event get(int millisec) {
                timespec t;
                Event e;
                if (clock_gettime(CLOCK_REALTIME, &t) == -1) {
                        e.second = Q_NOTIME;
                        return e;
                }

                t.tv_sec += millisec / 1000;
                t.tv_nsec += (millisec % 1000) * 1000000;
                if (sem_timedwait(&sem, &t)) {
                        e.second = Q_TIMEOUT;
                        return e;
                }

                if (pthread_mutex_trylock(&mutex) == 0) {
                        if (q.size()) {
                                e = std::make_pair(q.front(), Q_OK);
                                q.pop();
                        } else {
                                e.second = Q_EMPTY;
                        }
                        pthread_mutex_unlock(&mutex);
                } else {
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
        int put(const T& item) {
                if (pthread_mutex_lock(&mutex) == 0) {
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
         *  Beware this function is not thread-safe, it should only be used as a
         *  hint rather than an accurate measure.
         *
         *  \return the number of elements currently in the queue.
         */
        size_t size() const {return q.size();}
        /**
         *  \brief check if the queue is empty.
         *
         *  Beware this function is not thread-safe, it should only be used as a
         *  hint rather than an accurate measure.
         *
         *  \return true if the queue is empty, false otherwise.
         */
        bool empty() const {return q.empty();}
private:
        std::queue<T> q;
        sem_t sem;
        pthread_mutex_t mutex;
};

#endif /* SRQUEUE_H */
