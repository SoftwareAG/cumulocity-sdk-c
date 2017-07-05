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

#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <syslog.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SIZE 8

class Shm
{
public:

    Shm() : _shm(NULL), shmid(shmget(getpid(), SIZE, IPC_CREAT | 0666))
    {
        _shm = shmid != -1 ? (char*) shmat(shmid, NULL, 0) : _shm;
        _shm = _shm == (char*) -1 ? NULL : _shm;

        if (_shm)
        {
            for (int i = 0; i < SIZE; ++i)
                _shm[i] = 0;
        }
    }

    char *shm()
    {
        return _shm;
    }

    virtual ~Shm()
    {
        shmdt(_shm);
        shmctl(shmid, IPC_RMID, NULL);
    }

private:

    char *_shm;
    int shmid;
};

static bool _quit = false;
extern "C"
{
static void quit(int sig)
{
    _quit = true;
}
}

extern "C" int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        return 0;
    }

    Shm sh;
    char* const shm = sh.shm();

    if (shm == NULL)
    {
        syslog(LOG_CRIT, "shm fail");
        return -1;
    }

    const unsigned int val = strtoul(argv[2], NULL, 10);
    char shadow[SIZE] = { 0 };

    struct sigaction sa;
    sa.sa_handler = quit;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    while (!_quit)
    {
        timespec t0 = { 0 };
        clock_gettime(CLOCK_MONOTONIC_COARSE, &t0);
        *shm = 0;
        const pid_t child = fork();

        if (child == -1)
        {
            return 0;
        } else if (child == 0)
        {
            char **progv = (char**) argv + 1;
            return execv(progv[0], progv);
        } else
        {
            syslog(LOG_NOTICE, "forked child %d", child);

            while (!_quit)
            {
                int status;

                if (waitpid(child, &status, WNOHANG) > 0)
                {
                    if (WIFSIGNALED(status))
                    {
                        break;
                    }
                    else if (WIFEXITED(status))
                    {
                        _quit = true;
                    }
                }

                timespec t;
                clock_gettime(CLOCK_MONOTONIC_COARSE, &t);
                bool b = true;

                for (int i = 1; i <= *shm; ++i)
                {
                    b &= shm[i] != shadow[i];
                }

                if (b)
                {
                    t0.tv_sec = t.tv_sec;

                    for (int i = 1; i <= *shm; ++i)
                    {
                        shadow[i] = shm[i];
                    }

                } else if (val && t.tv_sec - t0.tv_sec > val)
                {
                    syslog(LOG_NOTICE, "child %d hangs", child);

                    kill(child, SIGKILL);
                    waitpid(-1, NULL, 0);
                    break;
                }

                sleep(2);
            }
        }
    }

    return 0;
}
