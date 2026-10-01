// Fast System Time
// Author: Mario Freire
// Version 0.1
// Copyright (C) 2024 DSP Interactive.

#include <stdarg.h>
#include "fskrnl.h"
#include "enum.h"

#define TIME_SECONDS_PER_MINUTE 60
#define TIME_SECONDS_PER_HOUR   3600
#define TIME_SECONDS_PER_DAY    86400

static struct tm time_tm;
static char time_buffer[26];

static int is_leap_year(int year)
{
    if ((year % 4) != 0)
        return 0;

    if ((year % 100) != 0)
        return 1;

    if ((year % 400) != 0)
        return 0;

    return 1;
}

static int days_in_year(int year)
{
    return is_leap_year(year) ? 366 : 365;
}

static int days_in_month(int year, int month)
{
    static const int days[12] =
    {
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };

    if (month == 1 && is_leap_year(year))
        return 29;

    return days[month];
}

static int32_t days_from_year(int year)
{
    int32_t y;

    y = year - 1;

    return y * 365
         + y / 4
         - y / 100
         + y / 400;
}

static int32_t days_from_1970(int year, int month, int day)
{
    int32_t days;
    int i;

    days = days_from_year(year);
    days -= days_from_year(1970);

    for (i = 0; i < month; i++)
        days += days_in_month(year, i);

    days += day - 1;

    return days;
}

struct tm *gmtime_r(const time_t *timer, struct tm *result)
{
    int32_t seconds;
    int32_t days;
    int32_t day_seconds;
    int year;
    int month;

    if (timer == 0 || result == 0)
        return 0;

    seconds = *timer;

    days = seconds / TIME_SECONDS_PER_DAY;
    day_seconds = seconds % TIME_SECONDS_PER_DAY;

    if (day_seconds < 0)
    {
        day_seconds += TIME_SECONDS_PER_DAY;
        days--;
    }

    result->tm_hour =
        day_seconds / TIME_SECONDS_PER_HOUR;

    day_seconds %= TIME_SECONDS_PER_HOUR;

    result->tm_min =
        day_seconds / TIME_SECONDS_PER_MINUTE;

    result->tm_sec =
        day_seconds % TIME_SECONDS_PER_MINUTE;

    result->tm_wday = (days + 4) % 7;

    if (result->tm_wday < 0)
        result->tm_wday += 7;

    year = 1970;

    if (days >= 0)
    {
        while (days >= days_in_year(year))
        {
            days -= days_in_year(year);
            year++;
        }
    }
    else
    {
        while (days < 0)
        {
            year--;
            days += days_in_year(year);
        }
    }

    result->tm_year = year - 1900;
    result->tm_yday = days;

    month = 0;

    while (days >= days_in_month(year, month))
    {
        days -= days_in_month(year, month);
        month++;
    }

    result->tm_mon = month;
    result->tm_mday = days + 1;
    result->tm_isdst = 0;

    return result;
}

struct tm *gmtime(const time_t *timer)
{
    return gmtime_r(timer, &time_tm);
}

struct tm *localtime_r(const time_t *timer, struct tm *result)
{
    return gmtime_r(timer, result);
}

struct tm *localtime(const time_t *timer)
{
    return localtime_r(timer, &time_tm);
}

time_t mktime(struct tm *tm)
{
    int32_t days;
    int32_t seconds;
    int year;
    int month;
    int i;

    if (tm == 0)
        return (time_t)-1;

    year = tm->tm_year + 1900;
    month = tm->tm_mon;

    while (month < 0)
    {
        month += 12;
        year--;
    }

    while (month >= 12)
    {
        month -= 12;
        year++;
    }

    days = days_from_1970(
        year,
        month,
        tm->tm_mday
    );

    seconds =
        days * TIME_SECONDS_PER_DAY +
        tm->tm_hour * TIME_SECONDS_PER_HOUR +
        tm->tm_min * TIME_SECONDS_PER_MINUTE +
        tm->tm_sec;

    tm->tm_year = year - 1900;
    tm->tm_mon = month;

    tm->tm_wday = (days + 4) % 7;

    if (tm->tm_wday < 0)
        tm->tm_wday += 7;

    tm->tm_yday = 0;

    for (i = 0; i < month; i++)
        tm->tm_yday += days_in_month(year, i);

    tm->tm_yday += tm->tm_mday - 1;
    tm->tm_isdst = 0;

    return (time_t)seconds;
}

time_t time(time_t *timer)
{
    time_t current;
    current = 0;
    if (timer != 0)
        *timer = current;
    return current;
}

double difftime(time_t time1, time_t time0)
{
    return (double)(time1 - time0);
}

char *asctime(const struct tm *tm)
{
    static const char *weekdays[7] =
    {
        "Sun",
        "Mon",
        "Tue",
        "Wed",
        "Thu",
        "Fri",
        "Sat"
    };

    static const char *months[12] =
    {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    };

    int year;

    if (tm == 0)
        return 0;

    if (tm->tm_wday < 0 || tm->tm_wday > 6)
        return 0;

    if (tm->tm_mon < 0 || tm->tm_mon > 11)
        return 0;

    year = tm->tm_year + 1900;

    time_buffer[0] = weekdays[tm->tm_wday][0];
    time_buffer[1] = weekdays[tm->tm_wday][1];
    time_buffer[2] = weekdays[tm->tm_wday][2];
    time_buffer[3] = ' ';

    time_buffer[4] = months[tm->tm_mon][0];
    time_buffer[5] = months[tm->tm_mon][1];
    time_buffer[6] = months[tm->tm_mon][2];
    time_buffer[7] = ' ';

    time_buffer[8] = '0' + tm->tm_mday / 10;
    time_buffer[9] = '0' + tm->tm_mday % 10;
    time_buffer[10] = ' ';

    time_buffer[11] = '0' + tm->tm_hour / 10;
    time_buffer[12] = '0' + tm->tm_hour % 10;
    time_buffer[13] = ':';

    time_buffer[14] = '0' + tm->tm_min / 10;
    time_buffer[15] = '0' + tm->tm_min % 10;
    time_buffer[16] = ':';

    time_buffer[17] = '0' + tm->tm_sec / 10;
    time_buffer[18] = '0' + tm->tm_sec % 10;
    time_buffer[19] = ' ';

    time_buffer[20] = '0' + (year / 1000) % 10;
    time_buffer[21] = '0' + (year / 100) % 10;
    time_buffer[22] = '0' + (year / 10) % 10;
    time_buffer[23] = '0' + year % 10;

    time_buffer[24] = '\n';
    time_buffer[25] = '\0';

    return time_buffer;
}

char *ctime(const time_t *timer)
{
    struct tm *tm;

    tm = localtime(timer);

    if (tm == 0)
        return 0;

    return asctime(tm);
}
