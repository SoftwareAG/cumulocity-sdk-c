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
        SrWatchdogTimer():
                shm(NULL), shmid(shmget(getppid(), SR_WDT_SIZE, 0666)), c(0) {
                shm = shmid != -1 ? (char*)shmat(shmid, NULL, 0) : shm;
                shm = shm == (char*)-1 ? NULL : shm;
                if (shm)
                        *shm = 1;
        }
        virtual ~SrWatchdogTimer() {shmdt(shm);}
        /**
         *  \brief Start the watchdog timer.
         *  \return 0 on success, -1 on failure.
         */
        int start() {return shm ? 0 : -1;};
        /**
         *  \brief *Kicking the dog*. No effect when start() failed.
         */
        void kick() {if (shm) shm[1] = c++;}
private:
        char *shm;
        int shmid;
        char c;
};

#endif /* SRWATCHDOGTIMER_H */
