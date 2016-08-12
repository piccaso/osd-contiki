/**
 * \addgroup Time related functions
 *
 * @{
 */
#include "contiki.h"
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "contiki-lib.h"
#include "xtime.h"
#include "tzparse.h"

#define SECSPERMIN      60
#define MINSPERHOUR     60
#define HOURSPERDAY     24
#define DAYSPERWEEK     7
#define DAYSPERNYEAR    365
#define DAYSPERLYEAR    366
#define SECSPERHOUR     (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY      ((long) SECSPERHOUR * HOURSPERDAY)
#define MONSPERYEAR     12

static const int mon_lengths[2][MONSPERYEAR] =
{ { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
, { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

/*
 * Static timezone information
 */
static struct tzoffset_info localtime_tzoffset;

/* Used for gmtime and localtime, according to manpage on linux the
 * internal value may be overwritten "by subsequent calls to any of the
 * date  and  time  functions".
 */
static struct xtm tm;

/*
 * Internal variables to manage offset of utc from the contiki clock
 * and timezone offset from utc in minutes.
 * For now we don't manage the sub-second offset -- time of setting the
 * clock and the precisiont of the clock in use don't warrant this
 * effort.
 * The last_seconds is used to check if we had a seconds overflow,
 * although this happens only every 136 years :-)
 */
static xtime_t  clock_offset;
static uint32_t last_seconds;

static xtime_t
transtime (xtime_t janfirst, int year, const struct tzrule *rp, long offset);

# define LEAP_YEAR(_year) \
    ((_year % 4) == 0 && (_year % 100 != 0 || _year % 400 == 0))
# define YDAYS(_year) (LEAP_YEAR(year) ? 366 : 365)

struct xtm *xgmtime_r (const xtime_t *timep, struct xtm *ptm)
{
    unsigned int year;
    int days, month, month_len;
    xtime_t t = *timep;
    ptm->tm_sec = t % 60;
    t /= 60;
    ptm->tm_min = t % 60;
    t /= 60;
    ptm->tm_hour = t % 24;
    t /= 24;
    ptm->tm_wday = (t+4) % 7;
    year = 1970;
    days =  0;
    while ((days += YDAYS (year)) <= t)
    {
        year++;
    }
    ptm->tm_year = year - 1900;
    days -= YDAYS (year);
    t    -= days;
    ptm->tm_yday = t;
    for (month=0; month<12; month++)
    {
        if (month == 1)
        {
            month_len = LEAP_YEAR (year) ? 29 : 28;
        }
        else
        {
            int m = month;
            if (m >= 7)
            {
                m -= 1;
            }
            m &= 1;
            month_len = m ? 30 : 31;
        }
        if (t >= month_len)
        {
            t -= month_len;
        }
        else
        {
            break;
        }
    }
    ptm->tm_mon    = month;
    ptm->tm_mday   = t + 1;
    ptm->tm_isdst  = 0;
    ptm->tm_gmtoff = 0;
    ptm->tm_zone   = "UTC";
    return ptm;
}

struct xtm *xgmtime (const xtime_t *timep)
{
    return xgmtime_r (timep, &tm);
}

/*
 * Compute is_dst flag of given timestamp
 */
static int is_dst (const xtime_t *timep, const struct tzoffset_info *tzo)
{
    xtime_t janfirst = 0;
    xtime_t starttime, endtime;
    int year = 1970;
    int lastdst = 0;
    if (tzo->dstname == NULL) {
        return 0;
    }

    for (year = 1970; janfirst < *timep; year++) {
        starttime = transtime (janfirst, year, &tzo->start, tzo->stdoffset);
        endtime   = transtime (janfirst, year, &tzo->end,   tzo->dstoffset);
        if (starttime <= *timep && endtime <= *timep) {
            lastdst = (starttime > endtime);
        } else if (starttime > *timep && endtime <= *timep) {
            return 0;
        } else if (starttime <= *timep && endtime > *timep) {
            return 1;
        } else if (starttime > *timep && endtime > *timep) {
            return lastdst;
        }
        janfirst += YDAYS (year) * SECSPERDAY;
    }
    return lastdst;
}

struct xtm *xlocaltime_r (const xtime_t *timep, struct xtm *ptm)
{
    const struct tzoffset_info *tzo = &localtime_tzoffset;
    int isdst = 0;
    long offset = 0;
    xtime_t t = *timep;

    if (tzo->stdname == NULL) {
        set_tz (DEFAULT_TIMEZONE);
    }
    isdst  = is_dst (timep, tzo);
    offset = isdst ? tzo->dstoffset : tzo->stdoffset;
    t -= offset;
    xgmtime_r (&t, ptm);
    ptm->tm_isdst  = isdst;
    ptm->tm_gmtoff = -offset;
    ptm->tm_zone   = isdst ? tzo->dstname : tzo->stdname;
    return ptm;
}

struct xtm *xlocaltime (const xtime_t *timep)
{
    return xlocaltime_r (timep, &tm);
}

/**
 * \brief Get time in seconds and microseconds
 * xgettimeofday will return the clock time as the microseconds part
 * while xsettimeofday will *ignore* the microseconds part (for now).
 * Note that the contiki clock interface is broken anyway, we can't read
 * seconds and sub-seconds atomically. We try to work around this by
 * repeatedly reading seconds, sub-seconds, seconds until first and
 * second read of seconds match.
 */
int xgettimeofday (struct xtimeval *tv, struct timezone *tz)
{
    uint32_t cs;
    if (tv) {
        int i;
        /* Limit tries to get the same second twice to two */
        for (i=0; i<2; i++) {
            cs = clock_seconds ();
            if (cs < last_seconds) {
                clock_offset += 0xFFFFFFFFL;
                clock_offset ++;
            }
            last_seconds = cs;
            tv->tv_sec = cs + clock_offset;
            tv->tv_usec = ((xtime_t)(clock_time () % CLOCK_SECOND))
                        * 1000000L / CLOCK_SECOND;
            if (cs == clock_seconds ()) {
                break;
            }
        }
    }
    if (tz) {
        const struct tzoffset_info *tzo = &localtime_tzoffset;
        tz->tz_dsttime     = is_dst (&tv->tv_sec, tzo);
        if (tz->tz_dsttime) {
            tz->tz_minuteswest = -tzo->dstoffset / 60;
        } else {
            tz->tz_minuteswest = -tzo->stdoffset / 60;
        }
    }
    return 0;
}

/**
 * \brief Set time in seconds, microseconds ignored for now
 */
int xsettimeofday (const struct xtimeval *tv, const struct timezone *tz)
{
    /* Don't allow setting timezone */
    if (tz) {
        errno = ERANGE;
        return -1;
    }
    if (tv) {
        uint32_t cs;
        cs = clock_seconds ();
        clock_offset = tv->tv_sec - cs;
    }
    return 0;
}

/*
 * Save timezone names into reserved string buffer and fill in the names
 * into the given tzoffset_info.
 * Return -1 on error, 0 for success.
 */
static int save_tznames
    ( const char *stdname, const char *dstname
    , size_t stdlen, size_t dstlen
    , struct tzoffset_info *tzo
    )
{
    size_t len = stdlen;
    if (stdname == NULL) {
        return -1;
    }
    if (dstname != NULL) {
        len += dstlen;
    }
    if (len + 2 > sizeof (tzo->namebuf)) {
        return -1;
    }
    tzo->stdname = tzo->namebuf;
    strncpy (tzo->namebuf, stdname, stdlen);
    tzo->namebuf [stdlen] = '\0';
    if (dstlen) {
        strncpy (tzo->namebuf + stdlen + 1, dstname, dstlen);
        tzo->namebuf [stdlen + 1 + dstlen] = '\0';
        tzo->dstname = tzo->namebuf + stdlen + 1;
    } else {
        tzo->dstname = NULL;
    }
    return 0;
}

/*
 * Utility functions for timezone string parsing (POSIX section 8)
 * Code adapted from OpenBSD localtime.c 1.57 2015/12/12 21:25:44
 * which is in the public domain.
 */

/*
 * The DST rules to use if TZ has no rules:
 * We default to US rules as of 1999-08-17.
 * POSIX 1003.1 section 8.1.1 says that the default DST rules are
 * implementation dependent; for historical reasons, US rules are a
 * common default.
 */
#ifndef TZDEFRULESTRING
#define TZDEFRULESTRING ",M4.1.0,M10.5.0"
#endif

/*
 * Given the Epoch-relative time of January 1, 00:00:00 UTC, in a year, the
 * year, a rule, and the offset from UTC at the time that rule takes effect,
 * calculate the Epoch-relative time that rule takes effect.
 */

static xtime_t
transtime(xtime_t janfirst, int year, const struct tzrule *rulep, long offset)
{
    int     leapyear;
    xtime_t value;
    int     i;
    int     d, m1, yy0, yy1, yy2, dow;

    value = 0;
    leapyear = LEAP_YEAR (year);
    switch (rulep->r_type) {

    case JULIAN_DAY:
        /*
         * Jn - Julian day, 1 == January 1, 60 == March 1 even in leap
         * years.
         * In non-leap years, or if the day number is 59 or less, just
         * add SECSPERDAY times the day number-1 to the time of
         * January 1, midnight, to get the day.
         */
        value = janfirst + (rulep->r_day - 1) * SECSPERDAY;
        if (leapyear && rulep->r_day >= 60) {
            value += SECSPERDAY;
        }
        break;

    case DAY_OF_YEAR:
        /*
         * n - day of year.
         * Just add SECSPERDAY times the day number to the time of
         * January 1, midnight, to get the day.
         */
        value = janfirst + rulep->r_day * SECSPERDAY;
        break;

    case MONTH_NTH_DAY_OF_WEEK:
        /*
         * Mm.n.d - nth "dth day" of month m.
         */
        value = janfirst;
        for (i = 0; i < rulep->r_mon - 1; ++i) {
            value += mon_lengths [leapyear][i] * SECSPERDAY;
        }

        /*
        ** Use Zeller's Congruence to get day-of-week of first day of
        ** month.
        */
        m1  = (rulep->r_mon + 9) % 12 + 1;
        yy0 = (rulep->r_mon <= 2) ? (year - 1) : year;
        yy1 = yy0 / 100;
        yy2 = yy0 % 100;
        dow = ((26 * m1 - 2) / 10 + 1 + yy2 + yy2 / 4 + yy1 / 4 - 2 * yy1) % 7;
        if (dow < 0) {
            dow += DAYSPERWEEK;
        }

        /*
        ** "dow" is the day-of-week of the first day of the month. Get
        ** the day-of-month (zero-origin) of the first "dow" day of the
        ** month.
        */
        d = rulep->r_day - dow;
        if (d < 0) {
            d += DAYSPERWEEK;
        }
        for (i = 1; i < rulep->r_week; ++i) {
            if (d + DAYSPERWEEK >= mon_lengths[leapyear][rulep->r_mon - 1]) {
                break;
            }
            d += DAYSPERWEEK;
        }

        /*
        ** "d" is the day-of-month (zero-origin) of the day we want.
        */
        value += d * SECSPERDAY;
        break;
    }

    /*
    ** "value" is the Epoch-relative time of 00:00:00 UTC on the day in
    ** question. To get the Epoch-relative time of the specified local
    ** time on that day, add the transition time and the current offset
    ** from UTC.
    */
    return value + rulep->r_time + offset;
}

/*
 * Given a pointer into a time zone string, scan until a character that is not
 * a valid character in a zone name is found. Return a pointer to that
 * character.
 */

static const char *getzname (const char *s)
{
    char c;
    while ((c = *s) != '\0' && !isdigit(c) && c != ',' && c != '-' && c != '+'){
        ++s;
    }
    return s;
}

/*
 * Given a pointer into an extended time zone string, scan until the ending
 * delimiter of the zone name is located. Return a pointer to the delimiter.
 *
 * As with getzname above, the legal character set is actually quite
 * restricted, with other characters producing undefined results.
 * We don't do any checking here; checking is done later in common-case code.
 */

static const char *getqzname (const char *strp, const int delim)
{
    int c;
    while ((c = *strp) != '\0' && c != delim) {
            ++strp;
    }
    return strp;
}

/*
 * Given a pointer into a time zone string, extract a number from that string.
 * Check that the number is within a specified range; if it is not, return
 * NULL.
 * Otherwise, return a pointer to the first character not part of the number.
 */

static const char *getnum (const char *strp, int *nump, int min, int max)
{
    char    c;
    int     num;

    if (strp == NULL || !isdigit ((c = *strp))) {
        return NULL;
    }
    num = 0;
    do {
        num = num * 10 + (c - '0');
        if (num > max) {
            return NULL; /* illegal value */
        }
        c = *++strp;
    } while (isdigit (c));
    if (num < min) {
        return NULL; /* illegal value */
    }
    *nump = num;
    return strp;
}

/*
 * Given a pointer into a time zone string, extract a number of seconds,
 * in hh[:mm[:ss]] form, from the string.
 * If any error occurs, return NULL.
 * Otherwise, return a pointer to the first character not part of the number
 * of seconds.
 */

static const char *getsecs (const char *strp, long *secsp)
{
    int num;

    /*
     * `HOURSPERDAY * DAYSPERWEEK - 1' allows quasi-Posix rules like
     * "M10.4.6/26", which does not conform to Posix,
     * but which specifies the equivalent of
     * ``02:00 on the first Sunday on or after 23 Oct''.
     */
    strp = getnum (strp, &num, 0, HOURSPERDAY * DAYSPERWEEK - 1);
    if (strp == NULL) {
        return NULL;
    }
    *secsp = num * (long) SECSPERHOUR;
    if (*strp == ':') {
        ++strp;
        strp = getnum (strp, &num, 0, MINSPERHOUR - 1);
        if (strp == NULL) {
            return NULL;
        }
        *secsp += num * SECSPERMIN;
        if (*strp == ':') {
            ++strp;
            /* `SECSPERMIN' allows for leap seconds. */
            strp = getnum (strp, &num, 0, SECSPERMIN);
            if (strp == NULL) {
                return NULL;
            }
            *secsp += num;
        }
    }
    return strp;
}

/*
 * Given a pointer into a time zone string, extract an offset, in
 * [+-]hh[:mm[:ss]] form, from the string.
 * If any error occurs, return NULL.
 * Otherwise, return a pointer to the first character not part of the time.
 */

static const char *getoffset (const char *strp, long *offsetp)
{
    int neg = 0;

    if (*strp == '-') {
        neg = 1;
        ++strp;
    } else if (*strp == '+') {
        ++strp;
    }
    strp = getsecs (strp, offsetp);
    if (strp == NULL) {
        return NULL;            /* illegal time */
    }
    if (neg) {
        *offsetp = -*offsetp;
    }
    return strp;
}

/*
 * Parse (optionally extended) timezone name. Return pointer to
 * (undelimited) timezone name in tzn and length of same in len.
 * Return pointer to first character *after* name, NULL on error.
 * Factored from original tzparse function.
 */
static const char *
egettzname (const char *strp, size_t *len, const char **tzn)
{
    *tzn = strp;
    if (*strp == '<') {
        strp++;
        *tzn = strp;
        strp = getqzname (strp, '>');
        if (*strp != '>') {
            return NULL;
        }
        *len = strp - *tzn;
        strp++;
    } else {
        strp = getzname (strp);
        *len = strp - *tzn;
    }
    return strp;
}

/*
** Given a pointer into a time zone string, extract a rule in the form
** date[/time]. See POSIX section 8 for the format of "date" and "time".
** If a valid rule is not found, return NULL.
** Otherwise, return a pointer to the first character not part of the rule.
*/

static const char *getrule (const char *strp, struct tzrule *rulep)
{
    if (*strp == 'J') {
        /*
         * Julian day.
         */
        rulep->r_type = JULIAN_DAY;
        ++strp;
        strp = getnum (strp, &rulep->r_day, 1, DAYSPERNYEAR);
    } else if (*strp == 'M') {
        /*
         * Month, week, day.
         */
        rulep->r_type = MONTH_NTH_DAY_OF_WEEK;
        ++strp;
        strp = getnum (strp, &rulep->r_mon, 1, MONSPERYEAR);
        if (strp == NULL) {
            return NULL;
        }
        if (*strp++ != '.') {
            return NULL;
        }
        strp = getnum (strp, &rulep->r_week, 1, 5);
        if (strp == NULL) {
            return NULL;
        }
        if (*strp++ != '.') {
            return NULL;
        }
        strp = getnum (strp, &rulep->r_day, 0, DAYSPERWEEK - 1);
    } else if (isdigit (*strp)) {
        /*
         * Day of year.
         */
        rulep->r_type = DAY_OF_YEAR;
        strp = getnum (strp, &rulep->r_day, 0, DAYSPERLYEAR - 1);
    } else {
        return NULL; /* invalid format */
    }
    if (strp == NULL) {
        return NULL;
    }
    if (*strp == '/') {
        /*
         * Time specified.
         */
        ++strp;
        strp = getsecs (strp, &rulep->r_time);
    } else {
        rulep->r_time = 2 * SECSPERHOUR; /* default = 2:00:00 */
    }
    return strp;
}

/*
 * Parse POSIX section 8 TZ string.
 * We keep the misnomer "name" for the timezone string.
 */
int tzparse (const char *name, struct tzoffset_info *tzo)
{
    const char *stdname;
    const char *dstname;
    size_t      stdlen;
    size_t      dstlen;
    long        stdoffset;
    long        dstoffset;

    dstname = NULL;
    stdname = name;
    name = egettzname (name, &stdlen, &stdname);
    if (name == NULL || *name == '\0') {
        return -1;
    }
    name = getoffset (name, &stdoffset);
    if (name == NULL) {
        return -1;
    }
    if (*name != '\0') {
        name = egettzname (name, &dstlen, &dstname);
        if (name == NULL) {
            return -1;
        }
        if (*name != '\0' && *name != ',' && *name != ';') {
            name = getoffset (name, &dstoffset);
            if (name == NULL) {
                return -1;
            }
        } else {
            dstoffset = stdoffset - SECSPERHOUR;
        }
        if (*name == '\0') {
            name = TZDEFRULESTRING;
        }
        if (*name == ',' || *name == ';') {
            struct tzrule     start;
            struct tzrule     end;
            ++name;
            if ((name = getrule (name, &start)) == NULL) {
                return -1;
            }
            if (*name++ != ',') {
                return -1;
            }
            if ((name = getrule (name, &end)) == NULL) {
                return -1;
            }
            if (*name != '\0') {
                return -1;
            }
            if (save_tznames (stdname, dstname, stdlen, dstlen, tzo) != 0) {
                return -1;
            }
            tzo->start = start;
            tzo->end   = end;
            tzo->stdoffset = stdoffset;
            tzo->dstoffset = dstoffset;
        } else {
            return -1;
        }
    } else {
        /* only standard time, no DST */
        if (save_tznames (stdname, NULL, stdlen, 0, tzo) != 0) {
            return -1;
        }
        tzo->stdoffset = stdoffset;
        tzo->dstoffset = stdoffset;
    }
    return 0;
}

/*
 * Provide a single static timezone which is used by localtime et.al.
 */
int set_tz (const char *tzstring)
{
    return tzparse (tzstring, &localtime_tzoffset);
}

static size_t lensecs (long seconds)
{
    size_t len = 1;
    long secs = abs (seconds);
    if (seconds < 0) {
        len++;
    }
    if (secs / 3600 > 9) {
        len++;
    }
    if (secs % 3600) {
        len += 3;
        if (secs % 60) {
            len += 3;
        }
    }
    return len;
}

/*
 * Get length of string resulting from serializing rule.
 */
static size_t lenrule (const struct tzrule *rule)
{
    size_t len = 0;
    if (rule->r_type == JULIAN_DAY) {
        len++;
    }
    if (rule->r_type == JULIAN_DAY || rule->r_type == DAY_OF_YEAR) {
        len++;
        if (rule->r_day > 9) {
            len++;
            if (rule->r_day > 99) {
                len++;
            }
        }
    } else if (rule->r_type == MONTH_NTH_DAY_OF_WEEK) {
        len++;
        len++;
        if (rule->r_mon > 9) {
            len++;
        }
        len += 4; /* dots and week/day */
        if (rule->r_time != 7200) {
            len++;
            len += lensecs (rule->r_time);
        }
    }
    return len;
}

static int is_extended_name (const char *name)
{
    int i;
    for (i=0; name [i]; i++) {
        if (isdigit (name [i]) || name [i] == '+' || name [i] == '-') {
            return 1;
        }
    }
    return 0;
}

/*
 * Get length of timezone string resulting from serializing tzo.
 */
static size_t len_tz_r (const struct tzoffset_info *tzo)
{
    size_t len = 0;
    if (tzo->stdname == NULL) {
        return 0;
    }
    len = strlen (tzo->stdname);
    if (is_extended_name (tzo->stdname)) {
        len += 2;
    }
    len += lensecs (tzo->stdoffset);
    if (tzo->dstname) {
        len += strlen (tzo->dstname);
        if (is_extended_name (tzo->dstname)) {
            len += 2;
        }
        if (tzo->dstoffset - tzo->stdoffset != -3600) {
            len += lensecs (tzo->dstoffset);
        }
        len += 2; /* commas */
        len += lenrule (&tzo->start);
        len += lenrule (&tzo->end);
    }
    return len;
}

size_t len_tz (void)
{
    return len_tz_r (&localtime_tzoffset);
}

void appendsecs (char *buf, long minutes)
{
    char *p = buf + strlen (buf);
    long min = abs (minutes);
    if (minutes < 0) {
        *p++ = '-';
    }
    if (min % 3600 == 0) {
        sprintf (p, "%ld", min / 3600);
    } else if (min % 60 == 0) {
        sprintf (p, "%ld:%ld", min / 3600, (min / 60) % 60);
    } else {
        sprintf (p, "%ld:%ld:%ld", min / 3600, (min / 60) % 60, min % 60);
    }
}

void appendrule (char *buf, const struct tzrule *rule)
{
    char *p = buf + strlen (buf);
    if (rule->r_type == JULIAN_DAY) {
        sprintf (p, "J%d", rule->r_day);
    } else if (rule->r_type == DAY_OF_YEAR) {
        sprintf (p, "%d", rule->r_day);
    } else if (rule->r_type == MONTH_NTH_DAY_OF_WEEK) {
        sprintf (p, "M%d.%d.%d", rule->r_mon, rule->r_week, rule->r_day);
        p = buf + strlen (buf);
        if (rule->r_time != 7200) {
            *p++ = '/';
            *p = '\0';
            appendsecs (p, rule->r_time);
        }
    }
}

const char *get_tz (char *buf, size_t buflen)
{
    const struct tzoffset_info *tzo = &localtime_tzoffset;

    if (tzo->stdname == NULL || len_tz_r (tzo) > buflen) {
        return NULL;
    }
    if (is_extended_name (tzo->stdname)) {
        sprintf (buf, "<%s>", tzo->stdname);
    } else {
        strcpy (buf, tzo->stdname);
    }
    appendsecs (buf, tzo->stdoffset);
    if (tzo->dstname != NULL) {
        if (is_extended_name (tzo->dstname)) {
            sprintf (buf + strlen (buf), "<%s>", tzo->dstname);
        } else {
            strcat (buf, tzo->dstname);
        }
        if (tzo->dstoffset - tzo->stdoffset != -3600) {
            appendsecs (buf, tzo->dstoffset);
        }
        strcat (buf, ",");
        appendrule (buf, &tzo->start);
        strcat (buf, ",");
        appendrule (buf, &tzo->end);
    }
    return buf;
}


/** @} */
