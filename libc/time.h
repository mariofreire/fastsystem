// Fast System Time
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

typedef int32_t time_t;

struct tm
{
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

time_t time(time_t *timer);
time_t mktime(struct tm *tm);

struct tm *gmtime(const time_t *timer);
struct tm *localtime(const time_t *timer);

struct tm *gmtime_r(const time_t *timer, struct tm *result);
struct tm *localtime_r(const time_t *timer, struct tm *result);

double difftime(time_t time1, time_t time0);

char *asctime(const struct tm *tm);
char *ctime(const time_t *timer);

#endif
