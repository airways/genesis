/*
// Full copyright information is available in the file ../doc/CREDITS
*/

#include "defs.h"

#include <time.h>
#include <string.h>
#include <sys/time.h>    /* for mtime() */

#include "operators.h"
#include "execute.h"
#include "util.h"
#include "opcodes.h"

#ifdef HAVE_GETTIMEOFDAY
#if defined(sys_ultrix) || defined(sys_solaris)
Int gettimeofday (struct timeval *tp, struct timezone *tzp);
#endif
#endif

void func_time(void) {
    /* Take no arguments. */
    if (!func_init_0())
	return;

    push_int(time(NULL));
}

void func_localtime(void) {
    struct tm * tms;
    cData * d;
    cList * l;
    time_t t;
    cData *args;
    Int     nargs;

    if (!func_init_0_or_1(&args, &nargs, INTEGER))
	return;

    if (nargs) {
        t = (time_t) args[0].u.val;
        pop(1);
    } else
        time(&t);

    tms = localtime(&t);

    l = list_new(11);
    d = list_empty_spaces(l, 11);

    d[0].type=INTEGER;
    d[0].u.val = (cNum) t;
    d[1].type=INTEGER;
    d[1].u.val = tms->tm_sec;
    d[2].type=INTEGER;
    d[2].u.val = tms->tm_min;
    d[3].type=INTEGER;
    d[3].u.val = tms->tm_hour;
    d[4].type=INTEGER;
    d[4].u.val = tms->tm_mday;
    d[5].type=INTEGER;
    d[5].u.val = tms->tm_mon;
    d[6].type=INTEGER;
    d[6].u.val = tms->tm_year;
    d[7].type=INTEGER;
    d[7].u.val = tms->tm_wday;
    d[8].type=INTEGER;
    d[8].u.val = tms->tm_yday;
    d[9].type=INTEGER;
    d[9].u.val = tms->tm_isdst;
    d[10].type = STRING;
    d[10].u.str= string_dup(str_tzname);

    push_list(l);
    list_discard(l);
}

void func_mtime(void) {
#ifdef HAVE_GETTIMEOFDAY
    struct timeval tp;
#endif

    if (!func_init_0())
        return;

#ifdef HAVE_GETTIMEOFDAY
    /* usec is microseconds */
    gettimeofday(&tp, NULL);

    push_int((cNum) tp.tv_usec);
#else
    push_int(-1);
#endif
}

void func_ctime(void) {
    cData *args;
    Int num_args;
    time_t tval;
    char *timestr;
    cStr *str;

    /* Take an optional integer argument. */
    if (!func_init_0_or_1(&args, &num_args, INTEGER))
	return;

    tval = (num_args) ? args[0].u.val : time(NULL);
    timestr = ctime(&tval);
    str = string_from_chars(timestr, 24);

    pop(num_args);
    push_string(str);
    string_discard(str);
}

void func_bind_function(void) {
    cData * args;
    Int      opcode;

    /* accept a symbol and objnum */
    if (!func_init_2(&args, SYMBOL, OBJNUM))
        return;

    opcode = find_function(ident_name(args[0].u.symbol));

    if (opcode == -1) {
        cthrow(perm_id, "Attempt to bind function which does not exist.");
        return;
    }

    op_table[opcode].binding = args[1].u.objnum;

    pop(2);
    push_int(1);
}

void func_unbind_function(void) {
    cData *args;
    Int   opcode;

    /* accept a symbol */
    if (!func_init_1(&args, SYMBOL))
        return;

    opcode = find_function(ident_name(args[0].u.symbol));

    if (opcode == -1) {
        cthrow(perm_id, "Attempt to unbind function which does not exist.");
        return;
    }

    op_table[opcode].binding = INV_OBJNUM;

    pop(1);
    push_int(1);
}
