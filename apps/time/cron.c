/**
 * \file
 *      cron: Cron-like functionality
 * \author
 *      Ralf Schlatterbeck <rsc@runtux.com>
 *
 * \brief cron-like command scheduler
 *
 * Inspired by vixie's cron by Paul Vixie which is
 * Copyright 1988,1990,1993,1994 by Paul Vixie
 * 
 * Distribute freely, except: don't remove my name from the source or
 * documentation (don't take credit for my work), mark your changes (don't
 * get me blamed for your possible bugs), don't alter or remove this
 * notice.  May be sold if buildable source is provided to buyer.  No
 * warrantee of any kind, express or implied, is included with this
 * software; use at your own risk, responsibility for damages (if any) to
 * anyone resulting from the use of this software rests entirely with the
 * user.
 *
 * Changes to make this work on a microcontroller by Ralf Schlatterbeck
 * In fact this is mostly a rewrite but keeps central algorithms and
 * some data structures of the original.
 * The syntax is simplified, we don't support the @ notation (e.g.
 * @hourly) and named entities (e.g. weekday names instead of numbers).
 * Furthermore we can't send email and don't support environment
 * variables. The called commands are functions registered via a
 * registration mechanism before runtime.
 * Copyright 2016 Ralf Schlatterbeck, distribute with the conditions
 * given above.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include "contiki.h"
#include "cron.h"
#include "xtime.h"

#undef DEBUG

static struct cron_entry   cron_entries  [MAX_CRON_ENTRIES];
static struct cron_cmd     cron_commands [MAX_CRON_COMMANDS];
static size_t cron_registered_entries  = 0;
static size_t cron_registered_commands = 0;
static struct cron_entry *entry_freelist = NULL;
static struct cron_entry *entry_list     = NULL;

typedef int64_t minute_t;
static xtime_t start_time;
static minute_t cron_clock_time;

/* Register a new cron command
 * A command consists of a name (which must be unique, uniqueness is not
 * enforced currently as registrations are done once at initialization
 * time), a function to be called and a parameter.
 */
int cron_register_command
    (const char *name, void (*function)(void *), void * parameter)
{
    if (cron_registered_commands >= MAX_CRON_COMMANDS) {
        return -1;
    }
    cron_commands [cron_registered_commands].name      = name;
    cron_commands [cron_registered_commands].function  = function;
    cron_commands [cron_registered_commands].parameter = parameter;
    cron_registered_commands++;
    return 0;
}

/*
 * Allocate a new crontab entry.
 * This allocates from the freelist and returns NULL if no more entries
 * are available.
 * Implementation note: If cron_registered_entries is 0 we first
 * initialize the entry_freelist and the entry_list.
 */
struct cron_entry *allocate_cron_entry (void)
{
    size_t n;
    struct cron_entry *e;
    if (cron_registered_entries == 0) {
        entry_list = NULL;
        entry_freelist = &cron_entries [0];
        for (n=0; n < (MAX_CRON_ENTRIES - 1); n++) {
            cron_entries [n].next = &cron_entries [n+1];
        }
        cron_entries [MAX_CRON_ENTRIES - 1].next = NULL;
    }
    if (cron_registered_entries >= MAX_CRON_ENTRIES) {
        return NULL;
    }
    e = entry_freelist;
    entry_freelist = e->next;
    e->next = entry_list;
    entry_list = e;
    cron_registered_entries++;
    return e;
}

void free_cron_entry (struct cron_entry *obsolete)
{
    struct cron_entry *e;
    assert (cron_registered_entries);
    if (entry_list == obsolete) {
        entry_list = obsolete->next;
        obsolete->next = entry_freelist;
        entry_freelist = obsolete;
        cron_registered_entries--;
        return;
    }
    for (e=entry_list; e; e=e->next) {
        if (e->next == obsolete) {
            e->next = obsolete->next;
            obsolete->next = entry_freelist;
            entry_freelist = obsolete;
            cron_registered_entries--;
            break;
        }
    }
    assert (e != NULL);
}

struct cron_entry *get_cron_entry (size_t idx)
{
    if (idx >= MAX_CRON_ENTRIES) {
        return NULL;
    }
    return &cron_entries [idx];
}

/* Set the number in bits, return 0 on success -1 on error */
static int set_element (bitstr_t *bits, int low, int high, int n)
{
    if (n < low || n > high) {
        return -1;
    }
    bit_set (bits, (n - low));
    return 0;
}

static const char *get_number (int *np, int high, const char *s)
{
    char *endptr;
    long l = strtol (s, &endptr, 10);
    if (endptr == s || errno == ERANGE || l > high) {
        return NULL;
    }
    *np = l;
    return endptr;
}

static const char *get_range (bitstr_t *bits, int low, int high, const char *s)
{
    int i;
    int n1, n2, n3;

    if (*s == '*') {
        /* '*' means "first-last" but can be modified by /step */
        n1 = low;
        n2 = high;
        if (*++s == '\0') {
            return NULL;
        }
    } else {
        s = get_number (&n1, high, s);
        if (s == NULL || *s == '\0') {
            return NULL;
        }
        if (*s != '-') {
            if (set_element (bits, low, high, n1) < 0) {
                return NULL;
            }
            return s;
        } else {
            if (*++s == '\0') {
                return NULL;
            }
            s = get_number (&n2, high, s);
            if (*s == '\0') {
                return NULL;
            }
        }
    }
    if (*s == '/') {
        if (*++s == '\0') {
            return NULL;
        }
        s = get_number (&n3, high, s);
        if (*s == '\0') {
            return NULL;
        }
    } else {
        n3 = 1;
    }
    /* Explicit check for sane values */
    if (n1 < low || n1 > high || n2 < low || n2 > high) {
        return NULL;
    }
    for (i=n1; i<=n2; i+=n3) {
        if (set_element (bits, low, high, i) < 0) {
            return NULL;
        }
    }
    return s;
}

static const char *get_list (bitstr_t *bits, int low, int high, const char *s)
{
    int done = 0;
    bit_nclear (bits, 0, (high - low + 1));
    while (!done) {
        s = get_range (bits, low, high, s);
        if (NULL == s) {
            return NULL;
        }
        if (*s == ',') {
            s++;
        } else {
            done = 1;
        }
    }
    /* Skip white space */
    while (s && isspace (*s)) {
        s++;
    }
    return s;
}

/*
 * Parse a single crontab entry on a single line.
 * We get the line via CoAP.
 * Returns 0 if parsed successfully, -1 on error.
 * On error the err pointer is set to the error message.
 */
int parse_crontab_line
    (const char *line, struct cron_entry *e, const char **err)
{
    size_t n;
    const char *s = line;
    size_t cmd_len = 0;

    e->flags &= ~VALID;
    if (*s == '*') {
        e->flags |= MIN_STAR;
    }
    s = get_list (e->minute, FIRST_MINUTE, LAST_MINUTE, s);
    if (NULL == s || *s == '\0') {
        *err = "minute";
        return -1;
    }
    if (*s == '*') {
        e->flags |= HR_STAR;
    }
    s = get_list (e->hour, FIRST_HOUR, LAST_HOUR, s);
    if (NULL == s || *s == '\0') {
        *err = "hour";
        return -1;
    }
    if (*s == '*') {
        e->flags |= DOM_STAR;
    }
    s = get_list (e->dom, FIRST_DOM, LAST_DOM, s);
    if (NULL == s || *s == '\0') {
        *err = "dom";
        return -1;
    }
    s = get_list (e->month, FIRST_MONTH, LAST_MONTH, s);
    if (NULL == s || *s == '\0') {
        *err = "month";
        return -1;
    }
    if (*s == '*') {
        e->flags |= DOW_STAR;
    }
    s = get_list (e->dow, FIRST_DOW, LAST_DOW, s);
    if (NULL == s || *s == '\0') {
        *err = "dow";
        return -1;
    }
    /* Make sundays equivalent */
    if (bit_test (e->dow, 0) || bit_test (e->dow, 7)) {
        bit_set (e->dow, 0);
        bit_set (e->dow, 7);
    }
    /* strip whitespace at *end* of command
     * by getting length without whitespace
     */
    for (n=0; s [n]; n++) {
        if (!isspace (s [n])) {
            cmd_len = n + 1;
        }
    }
    for (n=0; n<cron_registered_commands; n++) {
        if (  cmd_len == strlen (cron_commands [n].name)
           && !strncmp (cron_commands [n].name, s, cmd_len)
           )
        {
            e->cmd = &cron_commands [n];
            break;
        }
    }
    if (n == cron_registered_commands) {
        *err = "command";
        return -1;
    }
    e->flags |= VALID;
    return 0;
}

static void set_time (void)
{
    struct xtm *tm;
    struct xtimeval tv;
    xgettimeofday (&tv, NULL);
    start_time = tv.tv_sec;
    tm = xlocaltime (&start_time);
    /* We adjust the time to GMT so we can catch DST changes */
    cron_clock_time = (start_time + tm->tm_gmtoff) / (xtime_t)SECONDS_PER_MINUTE;
}

static void find_jobs (minute_t vtime, int do_wild, int do_nonwild)
{
    xtime_t virtual_second = vtime * SECONDS_PER_MINUTE;
    struct xtm *tm = xgmtime (&virtual_second);
    int minute, hour, dom, month, dow;
    struct cron_entry *e;

    /* make 0-based values out of these so we can use them as indicies */
    minute = tm->tm_min -FIRST_MINUTE;
    hour = tm->tm_hour -FIRST_HOUR;
    dom = tm->tm_mday -FIRST_DOM;
    month = tm->tm_mon +1 /* 0..11 -> 1..12 */ -FIRST_MONTH;
    dow = tm->tm_wday -FIRST_DOW;
    #ifdef DEBUG
    printf ("%d %d %d %d %d\n", minute, hour, dom, month, dow);
    #endif

    /* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
     * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
     * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
     * is why we keep 'e->dow_star' and 'e->dom_star'.  yes, it's bizarre.
     * like many bizarre things, it's the standard.
     */
    for (e = entry_list; e; e = e->next) {
        #ifdef DEBUG
        if (e->flags & VALID) {
            printf ("Checking entry %s\n", e->cmd->name);
        }
        printf ("valid:  %d\n", (e->flags & VALID));
        printf ("minute: %d\n", (bit_test (e->minute, minute)));
        printf ("hour:   %d\n", (bit_test (e->hour,   hour)));
        printf ("month   %d\n", (bit_test (e->month,  month)));
        printf ("dom*    %d\n", (e->flags & DOM_STAR));
        printf ("dow*    %d\n", (e->flags & DOW_STAR));
        printf ("dow     %d\n", (bit_test (e->dow,  dow)));
        printf ("dom     %d\n", (bit_test (e->dom,  dom)));
        printf ("min*    %d\n", (e->flags & MIN_STAR));
        printf ("hr*     %d\n", (e->flags & HR_STAR));
        printf ("nonwild %d\n", (do_nonwild));
        #endif
        if (  (e->flags & VALID)
           && bit_test (e->minute, minute)
           && bit_test (e->hour, hour)
           && bit_test (e->month, month)
           && ( ((e->flags & DOM_STAR) || (e->flags & DOW_STAR))
              ? (bit_test (e->dow, dow) && bit_test (e->dom, dom))
              : (bit_test (e->dow, dow) || bit_test (e->dom, dom))
              )
           )
        {
            if (  (do_nonwild && !(e->flags & (MIN_STAR | HR_STAR)))
               || (do_wild    &&  (e->flags & (MIN_STAR | HR_STAR)))
               )
            {
                printf ("Cron: calling \"%s\"\n", e->cmd->name);
                e->cmd->function (e->cmd->parameter);
            }
        }
    }
}

/*
 * The cron callback function must be run regularly, at least once per
 * minute.
 * Implementation:
 * Clocks are in minutes since the epoch (time() / 60).
 * virtual_time is the time it *would* be if we are called promptly and
 * nobody ever changed the clock. It is monotonically increasing...
 * unless a timejump happens.
 * time_running is the time we were last called. We determine
 * initialization by checking time_running for -1.
 * cron_clock_time is the current time for this run.
 */
void cron (void)
{
    static minute_t time_running = -1;
    static minute_t virtual_time = -1;
    minute_t time_diff;

    /*
     * ... calculate how the current time differs from
     * our virtual clock. Classify the change into one
     * of 4 cases
     */
    set_time ();
    if (time_running == cron_clock_time) {
        #ifdef DEBUG
        printf ("time not reached\n");
        #endif
        return;
    }
    time_running = cron_clock_time;
    time_diff = time_running - virtual_time;
    #ifdef DEBUG
    printf ("time_diff: %ld\n", (long)time_diff);
    #endif
    /* shortcut for the most common case */
    if (time_diff == 1) {
        virtual_time = time_running;
        find_jobs (virtual_time, 1, 1);
    } else {
        int wakeup_kind = -1;
        if (time_diff > -(3*MINUTE_COUNT)) {
            wakeup_kind = 0;
        }
        if (time_diff > 0) {
            wakeup_kind = 1;
        }
        if (time_diff > 5) {
            wakeup_kind = 2;
        }
        if (time_diff > (3*MINUTE_COUNT)) {
            wakeup_kind = 3;
        }
        #ifdef DEBUG
        printf ("wakeup_kind: %d\n", wakeup_kind);
        #endif
        switch (wakeup_kind) {
        /* time_diff is a small positive number (wokeup late)
         * run jobs for each virtual minute until caught up.
         */
        case 1:
            do {
                virtual_time++;
                find_jobs (virtual_time, 1, 1);
            } while (virtual_time < time_running);
            break;
        /* time_diff is a medium-sized positive number, for example
         * because we went to DST. Run wildcard jobs once, then run any
         * fixed-time jobs that would otherwise be skipped. If we use up
         * our minute  (possible, if there are a lot of jobs to run) go
         * around the loop again so that wildcard jobs have a chance to
         * run, and we do our housekeeping.
         */
        case 2:
            /* run wildcard jobs for current minute */
            find_jobs (time_running, 1, 0);
            /* run fixed-time jobs for each minute missed */
            do {
                virtual_time++;
                find_jobs (virtual_time, 0, 1);
                set_time ();
            } while (  virtual_time < time_running
                    && cron_clock_time == time_running
                    );
            break;
        /* time_diff is a small or medium-sized negative num, eg.
         * because of DST ending. Just run the wildcard jobs. The
         * fixed-time jobs probably have already run, and should not be
         * repeated. virtual_time does not change until we are caught up.
         */
        case 0:
            find_jobs (time_running, 1, 0);
            break;
        /* Other: time has changed a *lot*, jump virtual time, and run
         * everything
         */
        default:
            virtual_time = time_running;
            find_jobs (time_running, 1, 1);
            break;
        }
    }
}
