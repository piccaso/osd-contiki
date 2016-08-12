/*
 * Timezone parsing
 */

/**
 * \file
 *         Definitions for timezone parsing
 *      
 * \author
 *         Ralf Schlatterbeck <rsc@tux.runtux.com>
 */

#ifndef tzparse_h
#define tzparse_h

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Rule for DST switching
 */
struct tzrule {
    int             r_type;         /* type of rule--see below */
    int             r_day;          /* day number of rule */
    int             r_week;         /* week number of rule */
    int             r_mon;          /* month number of rule */
    long            r_time;         /* transition time of rule */
};

#define JULIAN_DAY              0       /* Jn - Julian day */
#define DAY_OF_YEAR             1       /* n - day of year */
#define MONTH_NTH_DAY_OF_WEEK   2       /* Mm.n.d - month, week, day of week */

/*
 * Info about timezone offset handling.
 * We get at least a dstname and stdoffset, if no daylight saving is in
 * effect, dstname is NULL and no rule is filled in.
 */
struct tzoffset_info {
    const char *stdname;
    const char *dstname;
    long stdoffset;
    long dstoffset;
    struct tzrule start;
    struct tzrule end;
    char namebuf [TZ_MAX_CHARS];
};

int tzparse (const char *name, struct tzoffset_info *tzo);

#ifdef __cplusplus
}
#endif

#endif // tzparse_h
