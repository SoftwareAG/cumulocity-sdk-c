#ifndef SRTYPES_H
#define SRTYPES_H
#include <string>

/**
 *  \class SrNews srtypes.h
 *  \brief Data type represents a SmartREST request (measurement, alarm, etc.).
 */
struct SrNews
{
        SrNews(uint8_t prio = 0): prio(prio) {}
        SrNews(const std::string &s, uint8_t prio = 0): data(s), prio(prio) {}
        /**
         *  \brief the request to send to Cumulocity.
         */
        std::string data;
        /**
         *  \brief request priority.
         *
         *  0 means no buffering, request will be discarded if the SrReporter
         *  can not send it after multiple trials. 1 means buffering, request
         *  will be buffered and retried later if the SrReporter thread fail to
         *  send the request.
         *  Note buffering still does not mean 100% guarantee, as the buffer
         *  of the SrReporter has an explicit capacity. Buffered old requests
         *  will be discarded if the capacity is exhausted.
         */
        uint8_t prio;
};


/**
 *  \class SrOpBatch srtypes.h
 *  \brief Data type represents a SmartREST response, i.e., a batch of
 *  multiple messages.
 */
struct SrOpBatch
{
        SrOpBatch() {}
        SrOpBatch(const std::string &s): data(s) {}
        /**
         *  \brief buffer contains the response.
         */
        std::string data;
};

#endif /* SRTYPES_H */
