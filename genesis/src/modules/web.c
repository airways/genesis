/*
// Full copyright information is available in the file ../doc/CREDITS
*/

/*
// RFC 1738:
//    
//   Many URL schemes reserve certain characters for a special meaning:
//   their appearance in the scheme-specific part of the URL has a
//   designated semantics. If the character corresponding to an octet is
//   reserved in a scheme, the octet must be encoded.  The characters ";",
//   "/", "?", ":", "@", "=" and "&" are the characters which may be
//   reserved for special meaning within a scheme. No other characters may
//   be reserved within a scheme.
//
//   [...]
//
//   Thus, only alphanumerics, the special characters "$-_.+!*'(),", and
//   reserved characters used for their reserved purposes may be used
//   unencoded within a URL.
//
//   valid ascii: 48-57 (0-9) 65-90 (A-Z) 97-122 (a-z)
*/

#define NATIVE_MODULE "$http"

#include "web.h"
#include "util.h"

/* valid ascii: 48-57 (0-9) 65-90 (A-Z) 97-122 (a-z) */

module_t web_module = {init_web, uninit_web};

/* we pre-define this for speed */
char * dec_2_hex[] = {
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL, (char) NULL, (char) NULL,
   (char) NULL, (char) NULL, (char) NULL,
   "%21", "%22", "%23", "%24", "%25", "%26", "%27", "%28", "%29", "%2a", "%2b",
   "%2c", "%2d", "%2e", "%2f", "%30", "%31", "%32", "%33", "%34", "%35", "%36",
   "%37", "%38", "%39", "%3a", "%3b", "%3c", "%3d", "%3e", "%3f", "%40", "%41",
   "%42", "%43", "%44", "%45", "%46", "%47", "%48", "%49", "%4a", "%4b", "%4c",
   "%4d", "%4e", "%4f", "%50", "%51", "%52", "%53", "%54", "%55", "%56", "%57",
   "%58", "%59", "%5a", "%5b", "%5c", "%5d", "%5e", "%5f", "%60", "%61", "%62",
   "%63", "%64", "%65", "%66", "%67", "%68", "%69", "%6a", "%6b", "%6c", "%6d",
   "%6e", "%6f", "%70", "%71", "%72", "%73", "%74", "%75", "%76", "%77", "%78",
   "%79", "%7a", "%7b", "%7c", "%7d", "%7e", (char) NULL
};

#define tohex(c) (dec_2_hex[(Int) c])


void init_web(Int argc, char ** argv) {
}

void uninit_web(void) {
}

INTERNAL char tochar(char h, char l) {
     char p;

     h = UCASE(h);
     l = UCASE(l);
     h -= '0';
     if (h > 9)
          h -= 7;
     l -= '0';
     if (l > 9)
          l -= 7;
     p = h * 16 + l;

     return p;
}

cStr * decode(cStr * str) {
    char * s = string_chars(str),
         * n = s,
           h,
           l;
    register Int len = string_length(str);

    for (; len > 0; len--, s++, n++) {
        switch (*s) {
            case '+':
                *n = ' ';
                break;
            case '%':
                h = *++s;
                l = *++s;
                len -= 2;
                *n = tochar(h, l);
                break;
            default:
                *n = *s;
        }
    }

    *n = (char) NULL;

    str->len = (n - str->s);

    return str;
}

cStr * encode(char * s) {
    cStr * str = string_new(strlen(s) * 2); /* this gives us a buffer
                                                   to expand into */

    for (;*s != (char) NULL; s++) {
        if ((Int) *s == 32)
            str = string_addc(str, '+');
        else if ((Int) *s > 32 && (Int) *s < 127) {
            if (((Int) *s >= 48 && (Int) *s <= 57) ||
                ((Int) *s >= 65 && (Int) *s <= 90) ||
                ((Int) *s >= 97 && (Int) *s <= 122))
                str = string_addc(str, *s);
            else
                str = string_add_chars(str, tohex(*s), 3);
        }
    }

    return str;
}

NATIVE_METHOD(decode) {
    cStr * str;

    INIT_1_ARG(STRING);

    str = string_dup(STR1);

    CLEAN_STACK();
    anticipate_assignment();

    /* decode should prep it, but its easier for us to do it */
#if 0
    str = string_prep(str, str->start, str->len);

    str = decode(str);

    RETURN_STRING(str);
#else
    RETURN_STRING(decode(string_prep(str, str->start, str->len)));

#endif
}

NATIVE_METHOD(encode) {
    cStr * str;

    INIT_1_ARG(STRING);

    str = encode(string_chars(STR1));

    CLEAN_RETURN_STRING(str);
}

