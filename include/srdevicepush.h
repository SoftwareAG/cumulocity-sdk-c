#ifndef SRDEVICEPUSH_H
#define SRDEVICEPUSH_H
#include "srtypes.h"
#include "srqueue.h"
#include "srnetsocket.h"
#include "srlogger.h"


class SrDevicePush
{
public:
        using string = std::string;

        SrDevicePush(const string &server, const string &xid,
                     const string &auth, const string &chn,
                     SrQueue<SrOpBatch> &queue);
        virtual ~SrDevicePush() {}

        int start();
        bool isSleeping() const { return sleeping; }
        void sleep() {
                sleeping = true;
                srNotice("push: slept.");
        }
        void resume() {
                sleeping = false;
                srNotice("push: resumed.");
        }

public:
        SrQueue<SrOpBatch> &queue;

protected:
        int handshake();
        int subscribe();
        int connect();
        void process(string &s);
        static void *func(void *arg);

private:
        SrNetSocket sock;
        pthread_t tid;
        string bayeuxID;
        string channel;
        const string header;
        size_t bnum;
        uint8_t bayeuxPolicy;
        bool sleeping;
};

#endif /* SRDEVICEPUSH_H */
