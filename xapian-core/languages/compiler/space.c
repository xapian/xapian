
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>    /* for printf */
#include <stdlib.h>   /* malloc, free */
#include <string.h>   /* memcpy */

#include "header.h"

#define HEAD (2 * sizeof(int))
#define EXTENDER 40

// Threshold in bytes above which reallocations double the size.
#define EXPONENTIAL_GROWTH_THRESHOLD 512

/*  This module provides a simple mechanism for arbitrary length writable
    strings, called 'blocks'. They are 'symbol *' items rather than 'char *'
    items however.

    The calls are:

        symbol * b = create_b(n);
            - create an empty block b with room for n symbols
        b = reserve_b(b, n);
            - ensure that the capacity of b is at least n (b may change)
        b2 = copy_b(b)
            - copy block b into b2
        lose_b(b);
            - lose block b
        b = add_to_b(b, p, n);
            - add the n symbols at address p to the end of the data in b
        SIZE(b)
            - is the number of symbols in b

    For example:

        symbol * b = create_b(0);
        for (symbol i = 'A'; i <= 'Z'; i++) {
            add_symbol_to_b(b, i);
        }

    After running the above code b contains:

        { (symbol)'A', (symbol)'B', ..., (symbol)'Z' }
*/

/*  For a block b, SIZE(b) is the number of symbols so far written into it,
    CAPACITY(b) the total number it can contain, so SIZE(b) <= CAPACITY(b).

    NB In the compiler, blocks do NOT have 1 extra character over the promised
    capacity - use ensure_nul_b() and ensure_nul_s() to zero terminate.
*/

extern symbol * create_b(int n) {
    symbol * p = (symbol *) (HEAD + (char *) MALLOC(HEAD + (n + 1) * sizeof(symbol)));
    CAPACITY(p) = n;
    SET_SIZE(p, 0);
    return p;
}

extern void report_b(FILE * out, const symbol * p) {
    for (int i = 0; i < SIZE(p); i++) {
        if (p[i] > 255) {
            printf("In report_b, can't convert p[%d] to char because it's 0x%02x\n", i, (int)p[i]);
            exit(1);
        }
        putc(p[i], out);
    }
}

extern void output_str(FILE * outfile, struct str * str) {
    report_s(outfile, str_data(str));
}

extern void lose_b(symbol * p) {
    if (p == NULL) return;
    FREE((char *) p - HEAD);
}

extern symbol * reserve_b_(symbol * p, int n) {
    const int EXP_THRESHOLD = EXPONENTIAL_GROWTH_THRESHOLD / sizeof(symbol);
    int new_size = n + EXTENDER;
    if (new_size > EXP_THRESHOLD) {
        // Switch to exponential growth for large strings.
        int c = (CAPACITY(p) | (EXP_THRESHOLD * 2 - 1)) + 1;
        while (c < new_size) c *= 2;
        new_size = c;
    }
    symbol * q = create_b(new_size);
    memcpy(q, p, SIZE(p) * sizeof(symbol));
    SET_SIZE(q, SIZE(p));
    lose_b(p);
    return q;
}

extern symbol * resize_b(symbol * p, int n) {
    int size = SIZE(p);
    if (n > size) {
        p = reserve_b(p, n);
        memset(p + size, 0, (n - size) * sizeof(symbol));
    }
    SET_SIZE(p, n);
    return p;
}

extern symbol * add_to_b(symbol * p, const symbol * q, int n) {
    p = reserve_b(p, SIZE(p) + n);
    memcpy(p + SIZE(p), q, n * sizeof(symbol));
    ADD_TO_SIZE(p, n);
    return p;
}

extern symbol * copy_b(const symbol * p) {
    int n = SIZE(p);
    symbol * q = create_b(n);
    add_to_b(q, p, n);
    return q;
}

int space_count = 0;

static void * xmalloc(size_t n) {
    void * result = malloc(n);
    if (result == NULL) {
        fprintf(stderr, "Failed to allocate %lu bytes\n", (unsigned long)n);
        exit(1);
    }
    return result;
}

extern void * check_malloc(size_t n) {
    space_count++;
    return xmalloc(n);
}

extern void check_free(void * p) {
    if (p) space_count--;
    free(p);
}

extern int checked_snprintf(char * str, size_t size,
                            const char * format, ...) {
    va_list ap;
    va_start(ap, format);
    int r = vsnprintf(str, size, format, ap);
    va_end(ap);
    // Some pre-C99 snprintf implementations return -1 if the buffer is too
    // small so cast to unsigned for a simpler test (we require C99, but better
    // to be robust if we encounter a C99 compiler with pre-C99 quirks in its
    // runtime library).
    if ((unsigned)r >= size) {
        fprintf(stderr, "snprintf(buf, %zu, \"%s\", ...) would overflow\n",
                size, format);
        exit(1);
    }
    return r;
}

/* Convert a block to a zero terminated string. */
extern char * b_to_sz(const symbol * p) {
    int n = SIZE(p);
    char * s = (char *)xmalloc(n + 1);
    for (int i = 0; i < n; i++) {
        if (p[i] > 255) {
            printf("In b_to_s, can't convert p[%d] to char because it's 0x%02x\n", i, (int)p[i]);
            exit(1);
        }
        s[i] = (char)p[i];
    }
    s[n] = 0;
    return s;
}

/* Add a single symbol to a block. If p = 0 the block is created. */
extern symbol * add_symbol_to_b(symbol * p, symbol ch) {
    if (p == NULL) {
        p = create_b(1);
    } else {
        p = reserve_b(p, SIZE(p) + 1);
    }
    p[SIZE(p)] = ch;
    ADD_TO_SIZE(p, 1);
    return p;
}

extern byte * create_s(int n) {
    byte * p = (byte *) (HEAD + (byte *) MALLOC(HEAD + (n + 1)));
    CAPACITY(p) = n;
    SET_SIZE(p, 0);
    return p;
}

extern byte * create_s_from_sz(const char * s) {
    int n = (int)strlen(s);
    byte * p = create_s(n);
    memcpy(p, s, n + 1);
    SET_SIZE(p, n);
    return p;
}

extern byte * create_s_from_data(const char * s, int n) {
    byte * p = create_s(n);
    memcpy(p, s, n);
    SET_SIZE(p, n);
    return p;
}

extern void report_s(FILE * out, const byte * p) {
    fwrite(p, 1, SIZE(p), out);
}

extern void lose_s(byte * p) {
    if (p == NULL) return;
    FREE((byte *) p - HEAD);
}

extern byte * reserve_s_(byte * p, int n) {
    const int EXP_THRESHOLD = EXPONENTIAL_GROWTH_THRESHOLD / sizeof(byte);
    n += EXTENDER;
    if (n > EXP_THRESHOLD) {
        // Switch to exponential growth for large strings.
        int c = (CAPACITY(p) | (EXP_THRESHOLD * 2 - 1)) + 1;
        while (c < n) c *= 2;
        n = c;
    }
    byte * q = create_s(n);
    memcpy(q, p, SIZE(p));
    SET_SIZE(q, SIZE(p));
    lose_s(p);
    return q;
}

extern byte * resize_s(byte * p, int n) {
    int size = SIZE(p);
    if (n > size) {
        p = reserve_s(p, n);
        memset(p + size, 0, n - size);
    }
    SET_SIZE(p, n);
    return p;
}

extern byte * copy_s(const byte * p) {
    return add_s_to_s(NULL, p);
}

/* Add a string with given length to a byte block. If p = 0 the
   block is created. */

extern byte * add_slen_to_s(byte * p, const char * s, int n) {
    if (p == NULL) {
        p = create_s(n);
    } else {
        p = reserve_s(p, SIZE(p) + n);
    }
    int k = SIZE(p);
    memcpy(p + k, s, n);
    SET_SIZE(p, k + n);
    return p;
}

/* Add a byte block to a byte block. If p = 0 the
   block is created. */

extern byte * add_s_to_s(byte * p, const byte * s) {
    return add_slen_to_s(p, (const char *)s, SIZE(s));
}

/* Add a zero terminated string to a byte block. If p = 0 the
   block is created. */

extern byte * add_sz_to_s(byte * p, const char * s) {
    return add_slen_to_s(p, s, (int)strlen(s));
}

/* Add a single character to a byte block. If p = 0 the
   block is created. */

extern byte * add_char_to_s(byte * p, char ch) {
    if (p == NULL) {
        p = create_s(1);
    } else {
        p = reserve_s(p, SIZE(p) + 1);
    }
    p[SIZE(p)] = ch;
    ADD_TO_SIZE(p, 1);
    return p;
}

/* The next section defines string handling capabilities in terms
   of the lower level byte block handling capabilities of space.c */
/* -------------------------------------------------------------*/

struct str {
    byte * data;
};

/* Create a new string. */
extern struct str * str_new(void) {
    struct str * output = (struct str *) xmalloc(sizeof(struct str));
    output->data = create_s(0);
    return output;
}

/* Delete a string. */
extern void str_delete(struct str * str) {
    lose_s(str->data);
    free(str);
}

/* Append a str to this str. */
extern void str_append(struct str * str, const struct str * add) {
    str->data = add_s_to_s(str->data, add->data);
}

/* Append a character to this str. */
extern void str_append_ch(struct str * str, char add) {
    str->data = add_char_to_s(str->data, add);
}

/* Append a low level byte block to a str. */
extern void str_append_s(struct str * str, const byte * q) {
    str->data = add_s_to_s(str->data, q);
}

/* Append a (char *, null terminated) string to a str. */
extern void str_append_string(struct str * str, const char * s) {
    str->data = add_sz_to_s(str->data, s);
}

/* Append an integer to a str. */
extern void str_append_int(struct str * str, int i) {
    // Most calls are for integers 0 to 9 (~72%).
    if (i >= 0 && i <= 9) {
        str_append_ch(str, '0' + i);
        return;
    }

    // Ensure there's enough space then snprintf() directly onto the end.
    int size = SIZE(str->data);
    int max_size = (CHAR_BIT * sizeof(int) + 5) / 3;
    str->data = reserve_s(str->data, size + max_size);
    int r = checked_snprintf((char*)str->data + size, max_size, "%d", i);
    ADD_TO_SIZE(str->data, r);
}

/* Append wide character to a string as UTF-8. */
extern void str_append_wchar_as_utf8(struct str * str, symbol ch) {
    if (ch < 0x80) {
        str_append_ch(str, (char)ch);
        return;
    }
    if (ch < 0x800) {
        str_append_ch(str, (ch >> 6) | 0xC0);
        str_append_ch(str, (ch & 0x3F) | 0x80);
        return;
    }
    str_append_ch(str, (ch >> 12) | 0xE0);
    str_append_ch(str, ((ch >> 6) & 0x3F) | 0x80);
    str_append_ch(str, (ch & 0x3F) | 0x80);
}

/* Clear a string */
extern void str_clear(struct str * str) {
    SET_SIZE(str->data, 0);
}

/* Set a string */
extern void str_assign(struct str * str, const char * s) {
    str_clear(str);
    str_append_string(str, s);
}

/* Copy a string. */
extern struct str * str_copy(const struct str * old) {
    struct str * newstr = str_new();
    str_append(newstr, old);
    return newstr;
}

/* Get the data stored in this str. */
extern byte * str_data(const struct str * str) {
    return str->data;
}

/* Get the length of the str. */
extern int str_len(const struct str * str) {
    return SIZE(str->data);
}

/* Get the last character of the str.
 *
 * Or -1 if the string is empty.
 */
extern int str_back(const struct str *str) {
    return SIZE(str->data) ? str->data[SIZE(str->data) - 1] : -1;
}

/* Remove the last character of the str.
 *
 * Or do nothing if the string is empty.
 */
extern void str_pop(const struct str *str) {
    if (SIZE(str->data)) ADD_TO_SIZE(str->data, -1);
}

/* Remove the last n characters of the str.
 *
 * Or make the string empty if its length is less than n.
 */
extern void str_pop_n(const struct str *str, int n) {
    if (SIZE(str->data) > n) {
        ADD_TO_SIZE(str->data, -n);
    } else {
        SET_SIZE(str->data, 0);
    }
}

extern int get_utf8(const symbol * p, int * slot) {
    int b0 = *p++;
    if (b0 < 0xC0) {   /* 1100 0000 */
        * slot = b0; return 1;
    }
    int b1 = *p++;
    if (b0 < 0xE0) {   /* 1110 0000 */
        * slot = (b0 & 0x1F) << 6 | (b1 & 0x3F); return 2;
    }
    * slot = (b0 & 0xF) << 12 | (b1 & 0x3F) << 6 | (*p & 0x3F); return 3;
}

extern int put_utf8(int ch, symbol * p) {
    if (ch < 0x80) {
        p[0] = ch; return 1;
    }
    if (ch < 0x800) {
        p[0] = (ch >> 6) | 0xC0;
        p[1] = (ch & 0x3F) | 0x80; return 2;
    }
    p[0] = (ch >> 12) | 0xE0;
    p[1] = ((ch >> 6) & 0x3F) | 0x80;
    p[2] = (ch & 0x3F) | 0x80; return 3;
}
