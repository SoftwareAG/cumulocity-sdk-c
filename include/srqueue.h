#ifndef SRQUEUE_H
#define SRQUEUE_H
#include <queue>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

#define Q_TIMEOUT -1
#define Q_BUSY -2
#define Q_EMPTY -3
#define Q_NOTIME -4
#define Q_UNKNOW -5

template<typename T>
class SrQueue
{
public:
        typedef std::pair<T, int> Event;

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

        Event get() {
                sem_wait(&sem);
                Event e;
                if (pthread_mutex_lock(&mutex) == 0) {
                        if (q.size()) {
                                e = std::make_pair(q.front(), 0);
                                q.pop();
                        } else {
                                e.second = Q_EMPTY;
                        }
                        pthread_mutex_unlock(&mutex);
                } else {
                        e.second = Q_UNKNOW;
                }
                return e;
        }
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
                                e = std::make_pair(q.front(), 0);
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
        int put(const T& item) {
                if (pthread_mutex_lock(&mutex) == 0) {
                        q.push(item);
                        pthread_mutex_unlock(&mutex);
                        sem_post(&sem);
                        return 0;
                }
                return -1;
        }
        size_t size() const { return q.size(); }
        bool empty() const { return q.empty(); }
private:
        std::queue<T> q;
        sem_t sem;
        pthread_mutex_t mutex;
};

#endif /* SRQUEUE_H */
