/*
 * Definitions for cron
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
 * Changes to make this work on a microcontroller by Ralf Schlatterbeck
 * In fact this is mostly a rewrite but keeps central algorithms of the
 * original.
 * Copyright 2016 Ralf Schlatterbeck, distribute with the conditions
 * given above.
 */

#ifndef _cron_h_
#define _cron_h_

#include "bitstring.h"

#define SECONDS_PER_MINUTE 60

#define FIRST_MINUTE    0
#define LAST_MINUTE     59
#define MINUTE_COUNT    (LAST_MINUTE - FIRST_MINUTE + 1)

#define FIRST_HOUR      0
#define LAST_HOUR       23
#define HOUR_COUNT      (LAST_HOUR - FIRST_HOUR + 1)

#define FIRST_DOM       1
#define LAST_DOM        31
#define DOM_COUNT       (LAST_DOM - FIRST_DOM + 1)

#define FIRST_MONTH     1
#define LAST_MONTH      12
#define MONTH_COUNT     (LAST_MONTH - FIRST_MONTH + 1)

/* note on DOW: 0 and 7 are both Sunday, for compatibility reasons. */
#define FIRST_DOW       0
#define LAST_DOW        7
#define DOW_COUNT       (LAST_DOW - FIRST_DOW + 1)

#ifndef MAX_CRON_ENTRIES
#define MAX_CRON_ENTRIES 5
#endif

#ifndef MAX_CRON_COMMANDS
#define MAX_CRON_COMMANDS 5
#endif

struct cron_cmd {
    const char *name;
    void      (*function)(void *);
    void       *parameter;
};

struct cron_entry {
    struct cron_entry *next;
    struct cron_cmd   *cmd;
    bitstr_t           bit_decl(minute, MINUTE_COUNT);
    bitstr_t           bit_decl(hour,   HOUR_COUNT);
    bitstr_t           bit_decl(dom,    DOM_COUNT);
    bitstr_t           bit_decl(month,  MONTH_COUNT);
    bitstr_t           bit_decl(dow,    DOW_COUNT);
    int                flags;

#define DOM_STAR        0x01
#define DOW_STAR        0x02
#define WHEN_REBOOT     0x04
#define MIN_STAR        0x08
#define HR_STAR         0x10
#define VALID           0x20
};

extern int parse_crontab_line
    (const char *line, struct cron_entry *e, const char **err);
extern int cron_register_command
    (const char *name, void (*function)(void *), void * parameter);

extern struct cron_entry *allocate_cron_entry (void);
extern struct cron_entry *get_cron_entry (size_t idx);
extern void               free_cron_entry (struct cron_entry *e);
extern void               cron (void);

#endif /* _cron_h_ */
