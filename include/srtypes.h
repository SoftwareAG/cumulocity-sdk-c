#ifndef SRTYPES_H
#define SRTYPES_H
#include <string>

#define SR_PRIO_BUF 1
#define SR_PRIO_XID 2

/**
 *  \class SrNews
 *  \brief Data type represents a SmartREST request (measurement, alarm, etc.).
 */
struct SrNews
{
        SrNews(uint8_t prio = 0): prio(prio) {}
        SrNews(const std::string &s, uint8_t prio = 0): data(s), prio(prio) {}
        /**
         *  \brief The request to send to Cumulocity.
         */
        std::string data;
        /**
         *  \brief request priority.
         *
         *  0 means no buffering, request will be discarded if the SrReporter
         *  can not send it after multiple trials. 1 means buffering, request
         *  will be buffered and retried later if the SrReporter thread fails
         *  to send the request.
         *
         *  \note Buffering still does not mean 100% guarantee, as the buffer
         *  of the SrReporter has an explicit capacity. Buffered old requests
         *  will be discarded if the capacity is exhausted.
         */
        uint8_t prio;
};


/**
 *  \class SrOpBatch
 *  \brief Data type represents a SmartREST response, i.e., a batch of
 *  multiple messages.
 */
struct SrOpBatch
{
        SrOpBatch() {}
        SrOpBatch(const std::string &s): data(s) {}
        /**
         *  \brief Buffer contains the response.
         */
        std::string data;
};

#endif /* SRTYPES_H */
