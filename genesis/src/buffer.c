/*
// ColdMUD was created and is copyright 1993, 1994 by Greg Hudson
//
// Genesis is a derivitive work, and is copyright 1995 by Brandon Gillespie.
// Full details and copyright information can be found in the file doc/CREDITS
//
// File: buffer.c
// ---
// Routines for ColdC buffer manipulation.
*/

#include <ctype.h>
#include "config.h"
#include "defs.h"
#include "y.tab.h"
#include "buffer.h"
#include "memory.h"
#include "util.h"

#define BUFALLOC(len)		(Buffer *)emalloc(sizeof(Buffer) + (len) - 1)
#define BUFREALLOC(buf, len)	(Buffer *)erealloc(buf, sizeof(Buffer) + (len) - 1)

internal Buffer *prepare_to_modify(Buffer *buf);

Buffer *buffer_new(int len) {
    Buffer *buf;

    buf = BUFALLOC(len);
    buf->len = len;
    buf->refs = 1;
    return buf;
}

Buffer *buffer_dup(Buffer *buf) {
    buf->refs++;
    return buf;
}

void buffer_discard(Buffer *buf) {
    buf->refs--;
    if (!buf->refs)
	efree(buf);
}

Buffer *buffer_append(Buffer *buf1, Buffer *buf2) {
    if (!buf2->len)
	return buf1;
    buf1 = prepare_to_modify(buf1);
    buf1 = BUFREALLOC(buf1, buf1->len + buf2->len);
    MEMCPY(buf1->s + buf1->len, buf2->s, buf2->len);
    buf1->len += buf2->len;
    return buf1;
}

int buffer_retrieve(Buffer *buf, int pos) {
    return buf->s[pos];
}

Buffer *buffer_replace(Buffer *buf, int pos, unsigned int c) {
    if (buf->s[pos] == c)
	return buf;
    buf = prepare_to_modify(buf);
    buf->s[pos] = OCTET_VALUE(c);
    return buf;
}

Buffer *buffer_add(Buffer *buf, unsigned int c) {
    buf = prepare_to_modify(buf);
    buf = BUFREALLOC(buf, buf->len + 1);
    buf->s[buf->len] = OCTET_VALUE(c);
    buf->len++;
    return buf;
}

Buffer *buffer_resize(Buffer *buf, int len) {
    if (len == buf->len)
	return buf;
    buf = prepare_to_modify(buf);
    buf = BUFREALLOC(buf, len);
    buf->len = len;
    return buf;
}

Buffer *buffer_tail(Buffer *buf, int pos) {
    Buffer *outbuf;

    if (pos == 1)
	return buf;

    outbuf = prepare_to_modify(buf);
    outbuf->len = buf->len - (pos-1);
    MEMCPY(outbuf->s, buf->s+(pos-1), outbuf->len);
    outbuf = BUFREALLOC(outbuf,outbuf->len);
    return(outbuf);
}

string_t * buffer_to_string(Buffer * buf) {
    string_t * str, * out;
    unsigned char * string_start, *p, *q;
    char * s;
    size_t len;

#define SEPCHAR '\n'
#define SEPLEN 1

    out = string_new(0);
    string_start = p = buf->s;
    while (p + SEPLEN <= buf->s + buf->len) {
        p = (unsigned char *) memchr(p, SEPCHAR, (buf->s + buf->len) - p);
        if (!p)
            break;
        str = string_new(p - string_start);
        s = str->s;
        for (q = string_start; q < p; q++) {
            if (isprint(*q))
                *s++ = *q;
        }
        *s = 0;
        str->len = s - str->s;
        out = string_add(out, str);
        out = string_add_chars(out, "\\n", 2);
        string_discard(str);
        string_start = p = p + SEPLEN;
    }

    len = (buf->s + buf->len) - string_start;
    s = (char *) tmalloc(sizeof(char) + len);
    MEMCPY(s, string_start, len);
    out = string_add_chars(out, s, len);
    tfree(s, len);

#undef SEPCHAR
#undef SEPLEN

    return out;
}

/* If sep (separator buffer) is NULL, separate by newlines. */
list_t *buffer_to_strings(Buffer *buf, Buffer *sep)
{
    data_t d;
    string_t *str;
    list_t *result;
    unsigned char sepchar, *string_start, *p, *q;
    char *s;
    int seplen;
    Buffer *end;

    sepchar = (sep) ? *sep->s : '\n';
    seplen = (sep) ? sep->len : 1;
    result = list_new(0);
    string_start = p = buf->s;
    while (p + seplen <= buf->s + buf->len) {
	/* Look for sepchar staring from p. */
	p = (unsigned char *)memchr(p, sepchar, 
				    (buf->s + buf->len) - (p + seplen - 1));
	if (!p)
	    break;

	/* Keep going if we don't match all of the separator. */
	if (sep && MEMCMP(p + 1, sep->s + 1, seplen - 1) != 0) {
	    p++;
	    continue;
	}

	/* We found a separator.  Copy the printable characters in the
	 * intervening text into a string. */
	str = string_new(p - string_start);
	s = str->s;
	for (q = string_start; q < p; q++) {
	    if (isprint(*q))
		*s++ = *q;
	}
	*s = 0;
	str->len = s - str->s;

	d.type = STRING;
	d.u.str = str;
	result = list_add(result, &d);
	string_discard(str);

	string_start = p = p + seplen;
    }

    /* Add the remainder characters to the list as a buffer. */
    end = buffer_new(buf->s + buf->len - string_start);
    MEMCPY(end->s, string_start, buf->s + buf->len - string_start);
    d.type = BUFFER;
    d.u.buffer = end;
    result = list_add(result, &d);
    buffer_discard(end);

    return result;
}

Buffer *buffer_from_string(string_t * string) {
    Buffer * buf;
    int      new;

    buf = buffer_new(string_length(string));
    new = parse_strcpy(buf->s, string_chars(string), string_length(string));

    if (string_length(string) - new)
        buf = buffer_resize(buf, new);

    return buf;
}

Buffer *buffer_from_strings(list_t * string_list, Buffer * sep) {
    data_t * string_data;
    Buffer *buf;
    int num_strings, i, len, pos;
    unsigned char *s;

    string_data = list_first(string_list);
    num_strings = list_length(string_list);

    /* Find length of finished buffer. */
    len = 0;
    for (i = 0; i < num_strings; i++)
        len += string_length(string_data[i].u.str) + ((sep) ? sep->len : 2);

    /* Make a buffer and copy the strings into it. */
    buf = buffer_new(len);
    pos = 0;
    for (i = 0; i < num_strings; i++) {
        s = string_chars(string_data[i].u.str);
        len = string_length(string_data[i].u.str);
        MEMCPY(buf->s + pos, s, len);
        pos += len;
        if (sep) {
            MEMCPY(buf->s + pos, sep->s, sep->len);
            pos += sep->len;
        } else {
            buf->s[pos++] = '\r';
            buf->s[pos++] = '\n';
        }
    }

    return buf;
}

static Buffer *prepare_to_modify(Buffer *buf)
{
    Buffer *cnew;

    if (buf->refs == 1)
	return buf;

    /* Make a new buffer with the same contents as the old one. */
    buf->refs--;
    cnew = buffer_new(buf->len);
    MEMCPY(cnew->s, buf->s, buf->len);
    return cnew;
}

