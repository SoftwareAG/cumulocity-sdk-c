#ifndef SRTYPES_H
#define SRTYPES_H
#include <string>


struct SrNews
{
        SrNews(uint8_t prio = 0): prio(prio) {}
        SrNews(const std::string &s, uint8_t prio = 0): data(s), prio(prio) {}
        std::string data;
        uint8_t prio;
};


struct SrOpBatch
{
        SrOpBatch() {}
        SrOpBatch(const std::string &s): data(s) {}
        std::string data;
};


// template<typename T1, typename T2>
// union MixPointer { T1 fptr; T2 functor; };

#endif /* SRTYPES_H */
