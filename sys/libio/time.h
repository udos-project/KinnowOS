#ifndef __LIBIO_TIME_H__
#define __LIBIO_TIME_H__ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#define CLOCKS_PER_SEC 32767

struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

struct itimeval {
    struct timeval it_interval;
    struct timeval it_value;
};

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

struct timespec {
    time_t tv_sec;
    long tv_nsec;
};

struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};

#ifdef __cplusplus
}
#endif

#endif
