#include "srtimer.h"

bool operator<=(const timespec &l, const timespec &r)
{
    return l.tv_sec == r.tv_sec ? l.tv_nsec <= r.tv_nsec : l.tv_sec <= r.tv_sec;
}
