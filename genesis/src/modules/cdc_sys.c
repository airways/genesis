/*
// ColdMUD was created and is copyright 1993, 1994 by Greg Hudson
//
// Genesis is a derivitive work, and is copyright 1995 by Brandon Gillespie.
// Full details and copyright information can be found in the file doc/CREDITS
//
// File: modules/cdc_sys.c
// ---
// System Object Module
*/

#include <sys/time.h>          /* getrusage()  25-Jan-95 BJG */
#include <sys/resource.h>      /* getrusage()  25-Jan-95 BJG */

#include "config.h"
#include "defs.h"
#include "y.tab.h"
#include "cdc_types.h"
#include "execute.h"

#ifdef HAVE_GETRUSAGE
int getrusage(int who, struct rusage * r_usage);
#endif

void op_next_dbref(void) {

    if (!func_init_0())
        return;

    push_dbref(db_top);
}

void op_load(void) {

    if (!func_init_0())
        return;

    push_int(1);
}

void op_status(void) {
#ifdef HAVE_GETRUSAGE
    struct rusage r;
#endif
    list_t *status;
    data_t *d;
    int x;

    if (!func_init_0())
        return;

#define __LLENGTH__ 18

    status = list_new(__LLENGTH__);
    d = list_empty_spaces(status, __LLENGTH__);
    for (x=0; x < __LLENGTH__; x++)
        d[x].type = INTEGER;

#ifndef HAVE_GETRUSAGE

    for (x=0; x < __LLENGTH__; x++)
        d[x].u.val = -1;

#else

    getrusage(RUSAGE_SELF, &r);
    d[0].u.val = (int) r.ru_utime.tv_sec; /* user time used (seconds) */
    d[1].u.val = (int) r.ru_utime.tv_usec;/* user time used (microseconds) */
    d[2].u.val = (int) r.ru_stime.tv_sec; /* system time used (seconds) */
    d[3].u.val = (int) r.ru_stime.tv_usec;/* system time used (microseconds) */
    d[4].u.val = (int) r.ru_maxrss;
    d[7].u.val = (int) r.ru_idrss;       /* integral unshared data size */
    d[8].u.val = (int) r.ru_minflt;      /* page reclaims */
    d[9].u.val = (int) r.ru_majflt;      /* page faults */
    d[10].u.val = (int) r.ru_nswap;       /* swaps */
    d[11].u.val = (int) r.ru_inblock;     /* block input operations */
    d[12].u.val = (int) r.ru_oublock;     /* block output operations */
    d[13].u.val = (int) r.ru_msgsnd;      /* messages sent */
    d[14].u.val = (int) r.ru_msgrcv;      /* messages received */
    d[15].u.val = (int) r.ru_nsignals;    /* signals received */
    d[16].u.val = (int) r.ru_nvcsw;       /* voluntary context switches */
    d[17].u.val = (int) r.ru_nivcsw;      /* involuntary context switches */

#endif

#undef __LLENGTH__

    push_list(status);
    list_discard(status);
}

void op_version(void) {
    list_t *version;
    data_t *d;

    /* Take no arguments. */
    if (!func_init_0())
        return;

    /* Construct a list of the version numbers and push it. */
    version = list_new(3);
    d = list_empty_spaces(version, 3);
    d[0].type = d[1].type = d[2].type = INTEGER;
    d[0].u.val = VERSION_MAJOR;
    d[1].u.val = VERSION_MINOR;
    d[2].u.val = VERSION_PATCH;
    push_list(version);
    list_discard(version);
}

