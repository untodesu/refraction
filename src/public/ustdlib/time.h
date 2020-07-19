#ifndef USTDLIB_TIME_H
#define USTDLIB_TIME_H
#include <stdint.h>

namespace ustd
{
    class Time {
    public:
        static const uint64_t unixTime(void);
        static const double unixTimeDouble(void);
    };

    class Clock {
    private:
        double start_time;

    public:
        Clock(void);
        const double getElapsedTime(void) const;
        const double restart(void);
    };
}

#endif
