#ifndef SRREPORTER_H
#define SRREPORTER_H
#include "srtypes.h"
#include "srnethttp.h"
#include "srqueue.h"
#include "srlogger.h"


class SrReporter
{
public:
        using string = std::string;
        SrReporter(const string &server, const string &xid, const string &auth,
                   SrQueue<SrNews> &out, SrQueue<SrOpBatch> &in,
                   uint16_t cap=1000):
                http(server + "/s", xid, auth), out(out), in(in),
                _cap(cap), sleeping(false) {}
        virtual ~SrReporter() {}

        uint16_t capacity() const { return _cap; }
        void setCapacity(uint16_t cap) { _cap = cap;}
        int start();
        bool isSleeping() const { return sleeping; }
        void sleep() {
                sleeping = true;
                srNotice("reporter: slept.");
        }
        void resume() {
                sleeping = false;
                srNotice("reporter: resumed.");
        }

protected:
        static void *func(void *arg);

private:
        std::deque<string> buffer;
        SrNetHttp http;
        pthread_t tid;
        SrQueue<SrNews> &out;
        SrQueue<SrOpBatch> &in;
        uint16_t _cap;
        bool sleeping;
};

#endif /* SRREPORTER_H */
