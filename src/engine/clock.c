#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#include <stdlib.h>
#endif
#include "clock.h"

int64_t CLOCK_now()
{
    int64_t time_ms;
#ifdef _WIN32
    time_ms = GetTickCount();
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    time_ms = (now.tv_sec * 1000) + (now.tv_usec / 1000);
#endif
    return time_ms;
}

int64_t CLOCK_time_passed(const int64_t start_time_ms)
{
    int64_t time_passed_ms;
    time_passed_ms = CLOCK_now() - start_time_ms;
#ifdef _WIN32
    if(time_passed_ms < 0) {
        /* Prevent wrap-around */
        time_passed_ms += 0x100000000;
    }
#endif
    return time_passed_ms;
}
