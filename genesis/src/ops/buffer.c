/*
// ColdMUD was created and is copyright 1993, 1994 by Greg Hudson
//
// Genesis is a derivitive work, and is copyright 1995 by Brandon Gillespie.
// Full details and copyright information can be found in the file doc/CREDITS
//
// File: ops/buffer.c
// ---
// Buffer manipulation functions
*/

#include "config.h"
#include "defs.h"
#include "cdc_types.h"
#include "operators.h"
#include "execute.h"

COLDC_FUNC(bufgraft) {
    if (!func_init_0())
        return;
    push_int(0);
}

COLDC_FUNC(buflen) {
    data_t * args;
    int len;

    if (!func_init_1(&args, BUFFER))
        return;

    len = buffer_len(_BUF(ARG1));

    pop(1);
    push_int(len);
}

COLDC_FUNC(buf_replace) {
    data_t * args;
    int pos;

    if (!func_init_3(&args, BUFFER, INTEGER, INTEGER))
        return;

    pos = _INT(ARG2) - 1;
    if (pos < 0)
	THROW((range_id, "Position (%d) is less than one.", pos + 1))
    else if (pos >= buffer_len(_BUF(ARG1)))
	THROW((range_id, "Position (%d) is greater than buffer length (%d).",
	      pos + 1, buffer_len(_BUF(ARG1))))

    _BUF(ARG1) = buffer_replace(_BUF(ARG1), pos, _INT(ARG3));

    pop(2);
}

COLDC_FUNC(subbuf) {
    data_t *args;
    int start, len, nargs, blen;

    if (!func_init_2_or_3(&args, &nargs, BUFFER, INTEGER, INTEGER))
	return;

    blen = args[0].u.buffer->len;
    start = args[1].u.val - 1;
    len = (nargs == 3) ? args[2].u.val : blen - start;

    if (start < 0)
        THROW((range_id, "Start (%d) is less than one.", start + 1))
    else if (len < 0)
        THROW((range_id, "Length (%d) is less than zero.", len))
    else if (start + len > blen)
        THROW((range_id,
              "The subrange extends to %d, past the end of the buffer (%d).",
              start + len, blen))

    anticipate_assignment();
    args[0].u.buffer = buffer_subrange(args[0].u.buffer, start, len);
    pop(nargs - 1);
}

COLDC_FUNC(buf_to_str) {
    data_t *args;
    string_t * str;

    if (!func_init_1(&args, BUFFER))
	return;

    str = buffer_to_string(args[0].u.buffer);

    pop(1);
    push_string(str);
    string_discard(str);
}

COLDC_FUNC(buf_to_strings) {
    data_t *args;
    int num_args;
    list_t *list;
    Buffer *sep;

    if (!func_init_1_or_2(&args, &num_args, BUFFER, BUFFER))
	return;

    sep = (num_args == 2) ? args[1].u.buffer : NULL;
    list = buffer_to_strings(args[0].u.buffer, sep);

    pop(num_args);
    push_list(list);
    list_discard(list);
}

COLDC_FUNC(str_to_buf) {
    data_t *args;
    Buffer *buf;

    if (!func_init_1(&args, STRING))
	return;
    buf = buffer_from_string(args[0].u.str);
    pop(1);
    push_buffer(buf);
    buffer_discard(buf);
}


COLDC_FUNC(strings_to_buf) {
    data_t *args, *d;
    int num_args, i;
    Buffer *buf, *sep;
    list_t *list;

    if (!func_init_1_or_2(&args, &num_args, LIST, BUFFER))
	return;

    list = args[0].u.list;
    sep = (num_args == 2) ? args[1].u.buffer : NULL;

    for (d = list_first(list), i=0; d; d = list_next(list, d),i++) {
	if (d->type != STRING)
            THROW((type_id, "List element %d (%D) not a string.", i + 1, d))
    }

    buf = buffer_from_strings(list, sep);

    pop(num_args);
    push_buffer(buf);
    buffer_discard(buf);
}

