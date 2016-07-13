#ifndef SRREPORTER_H
#define SRREPORTER_H
#include "srtypes.h"
#include "srnethttp.h"
#include "srqueue.h"
#include "srlogger.h"

/**
 *  \class SrReporter
 *  \brief The reporter thread for sending all requests to Cumulocity.
 *
 *  The SrReporter is responsible for sending all requests (measurements, alarms,
 *  events, etc.) to Cumulocity. For traffic saving, the SrReporter implements
 *  request aggregation when there are consecutive requests in the queue within
 *  less than 400 milliseconds. It also implements an exponential waiting from 2
 *  seconds up to 512 seconds for counteracting the instability of mobile
 *  networks. Additionally, it implements a capacity limited buffering mechanism
 *  for requests buffering in case the network is unavailable for the moment.
 */
class SrReporter
{
private:
        using string = std::string;
public:
        /**
         *  \brief SrReporter constrcutor.
         *  \param server Cumulocity server URL (no trailing slash).
         *  \param xid eXternal ID of the registered SmartREST template.
         *  \param auth authentication token from SrAgent.
         *  \param out reference to the SrAgent egress queue.
         *  \param in reference to the SrAgent ingress queue.
         *  \param cap capacity of the request buffer.
         */
        SrReporter(const string &server, const string &xid, const string &auth,
                   SrQueue<SrNews> &out, SrQueue<SrOpBatch> &in,
                   uint16_t cap=1000):
                http(server + "/s", "", auth), out(out),
                in(in), xid(xid), _cap(cap), sleeping(false) {}
        virtual ~SrReporter() {}

        /**
         *  \brief Get the current capacity of the request buffer.
         *
         *  capacity indicates the maximum number of messages (with SR_PRIO_BUF
         *  bit set) that will be buffered when the network is down. When
         *  the network is down for longer time, and the number of messages
         *  with SR_PRIO_BUF bit set exceeds the defined capacity, older
         *  messages will be discarded.

         *  \note Message buffering is intended for messages with higher demand
         *  for reliability because of temporary network error, e.g., alarms.
         *  Abuse of SR_PRIO_BUF bit by set it for all messages will not only
         *  downgrade agent's performance, but also real important messages
         *  will often be discarded.
         *
         *  \note The actual buffered messages will often be less than capacity
         *  because messages for setting XIDs will also take slots. Therefore,
         *  it's sensible not to use many SmartREST templates, consequently,
         *  less XIDs will take slots.
         */
        uint16_t capacity() const {return _cap;}
        /**
         *  \brief Set the capacity of the request buffer.
         *
         *  If the new capacity is smaller than already buffered requests, these
         *  requests will NOT be discarded.
         *
         *  \param cap new buffer capacity.
         */
        void setCapacity(uint16_t cap) {_cap = cap;}
        /**
         *  \brief Start the SrReporter thread.
         */
        int start();
        /**
         *  \brief Check if the SrReporter is sleeping.
         *  \return true if sleeping, false otherwise.
         */
        bool isSleeping() const {return sleeping;}
        /**
         *  \brief Put the SrReporter to sleep.
         *
         *  SrNews with SR_PRIO_BUF bit set will be buffered, other messages
         *  will be discarded. Sleeping an already slept SrReporter has no
         *  effect.
         */
        void sleep() {
                sleeping = true;
                srNotice("reporter: slept.");
        }
        /**
         *  \brief Resume a sleeping SrReporter. Resume a non-sleeping
         *  SrReporter has no effect.
         */
        void resume() {
                sleeping = false;
                srNotice("reporter: resumed.");
        }
        /**
         *  \brief Set file location for file backed message buffering.
         *
         *  By default, message buffering flag SR_PRIO_BUF is only implemented
         *  in memory, therefore, it doesn't survive a sudden outage or reboot.
         *  For devices with higher demand for sending reliability, can use
         *  this function to have a file backed message buffering.
         *
         *  File size is indirectly controlled by capacity() of the request
         *  buffer. File size is approximately capacity() * average message
         *  size.
         *
         *  \note Use file backed buffering incurs a substantial file I/O
         *  operations, especially when many messages are sent with SR_PRIO_BUF
         *  bit set.
         *
         *  \param filename file name for the buffering file.
         */
        void setBufFile(const string &filename) {fn = filename;};

protected:
        /**
         *  \brief pthread routine function.
         *  \param arg pointer to a SrReporter instance.
         */
        static void *func(void *arg);

private:
        std::deque<string> buffer;
        SrNetHttp http;
        pthread_t tid;
        SrQueue<SrNews> &out;
        SrQueue<SrOpBatch> &in;
        const string &xid;
        string fn;
        uint16_t _cap;
        bool sleeping;
};

#endif /* SRREPORTER_H */
