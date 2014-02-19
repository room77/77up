#ifndef _PUBLIC_UTIL_TIME_DEFS_H_
#define _PUBLIC_UTIL_TIME_DEFS_H_

// number of seconds in a day, an hour, a minute and a second
const int ONE_DAY = 86400;
const int ONE_HOUR = 3600;
const int ONE_MINUTE = 60;
const int ONE_SECOND = 1;

// for struct tm
#define YEAR(tm)      ((tm).tm_year + 1900)
#define MONTH(tm)     ((tm).tm_mon + 1)
#define DAY(tm)       ((tm).tm_mday)
#define DAYOFWEEK(tm) ((tm).tm_wday)
#define HOUR(tm)      ((tm).tm_hour)
#define MINUTE(tm)    ((tm).tm_min)
#define SECOND(tm)    ((tm).tm_sec)

#endif  // _PUBLIC_UTIL_TIME_DEFS_H_
