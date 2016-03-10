#ifndef SRWATCHDOGTIMER_H
#define SRWATCHDOGTIMER_H
#include <sys/shm.h>
#include <unistd.h>
#define SR_WDT_SIZE 8


class SrWatchDogTimer
{
public:
        SrWatchDogTimer():
                shm(NULL), shmid(shmget(getppid(), SR_WDT_SIZE, 0666)), c(0) {
                shm = shmid != -1 ? (char*)shmat(shmid, NULL, 0) : shm;
                shm = shm == (char*)-1 ? NULL : shm;
                if (shm)
                        *shm = 1;
        }
        virtual ~SrWatchDogTimer() {shmdt(shm);}
        int start() {return shm ? 0 : -1;};
        void kick() {if (shm) shm[1] = c++;}
private:
        char *shm;
        int shmid;
        char c;
};

#endif /* SRWATCHDOGTIMER_H */
