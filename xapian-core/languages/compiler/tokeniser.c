
#include <stdio.h>   /* stderr etc */
#include <stdlib.h>  /* malloc free */
#include <string.h>  /* strlen */
#include <ctype.h>   /* isalpha etc */
#include "header.h"

struct system_word {
    int s_size;      /* size of system word */
    const byte * s;  /* pointer to the system word */
    int code;        /* its internal code */
};


/* ASCII collating assumed in syswords.h */

#include "syswords.h"

#define INITIAL_INPUT_BUFFER_SIZE 8192

static int hex_to_num(int ch);

static int smaller(int a, int b) { return a < b ? a : b; }

extern byte * get_input(const char * filename) {
    FILE * input = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "rb");
    if (input == NULL) { return NULL; }
    byte * u = NULL;
    int size = fseek(input, 0, SEEK_END) == 0 ? ftell(input) : -1;
    if (size >= 0 && fseek(input, 0, SEEK_SET) == 0) {
        u = create_s(size);
        if (fread(u, size, 1, input) != 1) {
            fprintf(stderr, "%s: Read error\n", filename);
            exit(1);
        }
    } else {
        // Unseekable stream, e.g. piped stdin.
        size = 0;
        u = create_s(INITIAL_INPUT_BUFFER_SIZE);
        while (true) {
            int s = CAPACITY(u) - size;
            int r = fread(u + size, 1, s, input);
            if (r < 0) {
                fprintf(stderr, "%s: Read error\n", filename);
                exit(1);
            }
            size += r;
            if (r < s) break;
            u = increase_capacity_s(u, size);
        }
    }
    if (input != stdin) fclose(input);
    SET_SIZE(u, size);
    return u;
}

static void error(struct tokeniser * t, const char * s1, byte * p, int n, const char * s2) {
    if (t->error_count == 20) { fprintf(stderr, "... etc\n"); exit(1); }
    fprintf(stderr, "%s:%d: ", t->file, t->line_number);
    if (s1) fprintf(stderr, "%s", s1);
    if (p) {
        int i;
        for (i = 0; i < n; i++) fprintf(stderr, "%c", p[i]);
    }
    if (s2) fprintf(stderr, "%s", s2);
    fprintf(stderr, "\n");
    t->error_count++;
}

static void error1(struct tokeniser * t, const char * s) {
    error(t, s, NULL, 0, NULL);
}

static void error2(struct tokeniser * t, const char * s) {
    error(t, "unexpected end of text after ", NULL, 0, s);
}

static int compare_words(int m, const byte * p, int n, const byte * q) {
    if (m != n) return m - n;
    return memcmp(p, q, n);
}

static int find_word(int n, const byte * p) {
    int i = 0; int j = vocab->code;
    do {
        int k = i + (j - i)/2;
        const struct system_word * w = vocab + k;
        int diff = compare_words(n, p, w->s_size, w->s);
        if (diff == 0) return w->code;
        if (diff < 0) j = k; else i = k;
    } while (j - i != 1);
    return -1;
}

static int white_space(struct tokeniser * t, int ch) {
    switch (ch) {
        case '\n':
            t->line_number++;
            /* fall through */
        case '\r':
        case '\t':
        case ' ':
            return true;
    }
    return false;
}

static symbol * find_in_m(struct tokeniser * t, int n, byte * p) {
    struct m_pair * q;
    for (q = t->m_pairs; q; q = q->next) {
        byte * name = q->name;
        if (n == SIZE(name) && memcmp(name, p, n) == 0) return q->value;
    }
    return NULL;
}

static int read_literal_string(struct tokeniser * t, int c) {
    byte * p = t->p;
    SET_SIZE(t->b, 0);
    while (true) {
        if (c >= SIZE(p) || p[c] == '\n') {
            error1(t, "string literal not terminated");
            return c;
        }
        int ch = p[c];
        c++;
        if (ch == t->m_start) {
            /* Inside insert characters. */
            int c0 = c;
            int newlines = false; /* no newlines as yet */
            int all_whitespace = true; /* no printing chars as yet */
            while (true) {
                if (c >= SIZE(p) || (p[c] == '\n' && !all_whitespace)) {
                    error1(t, "string literal not terminated");
                    return c;
                }
                ch = p[c];
                if (ch == '\n') {
                    newlines = true;
                }
                c++;
                if (ch == t->m_end) break;
                if (!white_space(t, ch)) all_whitespace = false;
            }
            if (!newlines) {
                int n = c - c0 - 1;    /* macro size */
                int firstch = p[c0];
                symbol * q = find_in_m(t, n, p + c0);
                if (q == NULL) {
                    if (n == 1 && (firstch == '\'' || firstch == t->m_start))
                        t->b = add_symbol_to_b(t->b, p[c0]);
                    else if (n >= 3 && firstch == 'U' && p[c0 + 1] == '+') {
                        int codepoint = 0;
                        if (t->uplusmode == UPLUS_DEFINED) {
                            /* See if found with xxxx upper-cased. */
                            byte * uc = create_s(n);
                            int i;
                            for (i = 0; i != n; ++i) {
                                uc[i] = toupper(p[c0 + i]);
                            }
                            q = find_in_m(t, n, uc);
                            lose_s(uc);
                            if (q != NULL) {
                                t->b = add_to_b(t->b, q, SIZE(q));
                                continue;
                            }
                            error1(t, "Some U+xxxx stringdefs seen but not this one");
                        } else {
                            t->uplusmode = UPLUS_UNICODE;
                        }
                        for (int x = c0 + 2; x != c - 1; ++x) {
                            int hex = hex_to_num(p[x]);
                            if (hex < 0) {
                                error1(t, "Bad hex digit following U+");
                                break;
                            }
                            codepoint = (codepoint << 4) | hex;
                        }
                        if (t->encoding == ENC_UTF8) {
                            if (codepoint < 0 || codepoint > 0x01ffff) {
                                error1(t, "character values exceed 0x01ffff");
                            }
                            /* Ensure there's enough space for a max length
                             * UTF-8 sequence. */
                            int b_size = SIZE(t->b);
                            if (CAPACITY(t->b) < b_size + 3) {
                                t->b = increase_capacity_b(t->b, 3);
                            }
                            SET_SIZE(t->b, b_size + put_utf8(codepoint, t->b + b_size));
                        } else {
                            if (t->encoding == ENC_SINGLEBYTE) {
                                /* Only ISO-8859-1 is handled this way - for
                                 * other single-byte character sets you need
                                 * to stringdef all the U+xxxx codes you use
                                 * like - e.g.:
                                 *
                                 * stringdef U+0171   hex 'FB'
                                 */
                                if (codepoint < 0 || codepoint > 0xff) {
                                    error1(t, "character values exceed 256");
                                }
                            } else {
                                if (codepoint < 0 || codepoint > 0xffff) {
                                    error1(t, "character values exceed 64K");
                                }
                            }
                            t->b = add_symbol_to_b(t->b, (symbol)codepoint);
                        }
                    } else {
                        error(t, "string macro '", p + c0, n, "' undeclared");
                    }
                } else {
                    t->b = add_to_b(t->b, q, SIZE(q));
                }
            }
        } else {
            if (ch == '\'') return c;
            if (ch < 0 || ch >= 0x80) {
                if (t->encoding != ENC_WIDECHARS) {
                    /* We don't really want people using non-ASCII literal
                     * strings, but historically it's worked for single-byte
                     * and UTF-8 if the source encoding matches what the
                     * generated stemmer works in and it seems unfair to just
                     * suddenly make this a hard error.
                     */
                    fprintf(stderr,
                            "%s:%d: warning: Non-ASCII literal strings aren't "
                            "portable - use stringdef instead\n",
                            t->file, t->line_number);
                } else {
                    error1(t, "Non-ASCII literal strings aren't "
                              "portable - use stringdef instead");
                }
            }
            t->b = add_symbol_to_b(t->b, p[c - 1]);
        }
    }
}

static int next_token(struct tokeniser * t) {
    byte * p = t->p;
    int c = t->c;
    int code = -1;
    while (true) {
        if (c >= SIZE(p)) { t->c = c; return -1; }
        int ch = p[c];
        if (white_space(t, ch)) { c++; continue; }
        if (isalpha(ch)) {
            int c0 = c;
            while (c < SIZE(p) && (isalnum(p[c]) || p[c] == '_')) c++;
            code = find_word(c - c0, p + c0);
            if (code < 0 || t->token_disabled[code]) {
                SET_SIZE(t->s, 0);
                t->s = add_slen_to_s(t->s, (const char*)p + c0, c - c0);
                code = c_name;
            }
        } else if (isdigit(ch)) {
            int value = ch - '0';
            while (++c < SIZE(p) && isdigit(p[c])) {
                value = 10 * value + (p[c] - '0');
            }
            t->number = value;
            code = c_number;
        } else if (ch == '\'') {
            c = read_literal_string(t, c + 1);
            code = c_literalstring;
        } else {
            int lim = smaller(2, SIZE(p) - c);
            int i;
            for (i = lim; i > 0; i--) {
                code = find_word(i, p + c);
                if (code >= 0) { c += i; break; }
            }
        }
        if (code >= 0) {
            t->c = c;
            return code;
        }
        error(t, "'", p + c, 1, "' unknown");
        c++;
        continue;
    }
}

static int next_char(struct tokeniser * t) {
    if (t->c >= SIZE(t->p)) return -1;
    return t->p[t->c++];
}

static int next_real_char(struct tokeniser * t) {
    while (true) {
        int ch = next_char(t);
        if (!white_space(t, ch)) return ch;
    }
}

static void read_chars(struct tokeniser * t) {
    int ch = next_real_char(t);
    if (ch < 0) { error2(t, "stringdef"); return; }
    int c0 = t->c-1;
    while (true) {
        ch = next_char(t);
        if (white_space(t, ch) || ch < 0) break;
    }
    SET_SIZE(t->s, 0);
    t->s = add_slen_to_s(t->s, (const char*)t->p + c0, t->c - c0 - 1);
}

static int decimal_to_num(int ch) {
    if ('0' <= ch && ch <= '9') return ch - '0';
    return -1;
}

static int hex_to_num(int ch) {
    if ('0' <= ch && ch <= '9') return ch - '0';
    if ('a' <= ch && ch <= 'f') return ch - 'a' + 10;
    if ('A' <= ch && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

static void convert_numeric_string(struct tokeniser * t, symbol * p, int base) {
    int c = 0; int d = 0;
    while (true) {
        while (c < SIZE(p) && p[c] == ' ') c++;
        if (c == SIZE(p)) break;

        int number = 0;
        while (c != SIZE(p)) {
            int ch = p[c];
            if (ch == ' ') break;
            if (base == 10) {
                ch = decimal_to_num(ch);
                if (ch < 0) {
                    error1(t, "decimal string contains non-digits");
                    return;
                }
            } else {
                ch = hex_to_num(ch);
                if (ch < 0) {
                    error1(t, "hex string contains non-hex characters");
                    return;
                }
            }
            number = base * number + ch;
            c++;
        }
        if (t->encoding == ENC_SINGLEBYTE) {
            if (number < 0 || number > 0xff) {
                error1(t, "character values exceed 256");
                return;
            }
        } else {
            if (number < 0 || number > 0xffff) {
                error1(t, "character values exceed 64K");
                return;
            }
        }
        if (t->encoding == ENC_UTF8)
            d += put_utf8(number, p + d);
        else
            p[d++] = number;
    }
    SET_SIZE(p, d);
}

extern int read_token(struct tokeniser * t) {
    if (t->token_held) {
        t->token_held = false;
        return t->token;
    }
    t->token_reported_as_unexpected = false;
    byte * p = t->p;
    while (true) {
        int code = next_token(t);
        switch (code) {
            case c_comment1: /*  slash-slash comment */
                while (t->c < SIZE(p) && p[t->c] != '\n') t->c++;
                continue;
            case c_comment2: { /* slash-star comment */
                // Scan for a '*' stopping one before the end since we need a
                // '/' to follow it to close the comment.
                int size_less_one = SIZE(p) - 1;
                int c = t->c;
                while (true) {
                    if (c >= size_less_one) {
                        error1(t, "/* comment not terminated");
                        t->token = -1;
                        return -1;
                    }
                    if (p[c] == '\n') {
                        t->line_number++;
                    } else if (p[c] == '*' && p[c + 1] == '/') {
                        // Found '*/' to end of comment.
                        t->c = c + 2;
                        break;
                    }
                    ++c;
                }
                continue;
            }
            case c_stringescapes: {
                int ch1 = next_real_char(t);
                int ch2 = next_real_char(t);
                if (ch2 < 0) {
                    error2(t, "stringescapes");
                    continue;
                }
                if (ch1 == '\'') {
                    error1(t, "first stringescape cannot be '");
                    continue;
                }
                t->m_start = ch1;
                t->m_end = ch2;
                continue;
            }
            case c_stringdef: {
                int base = 0;
                read_chars(t);
                code = read_token(t);
                if (code == c_hex) { base = 16; code = read_token(t); } else
                if (code == c_decimal) { base = 10; code = read_token(t); }
                if (code != c_literalstring) {
                    error1(t, "string omitted after stringdef");
                    continue;
                }
                if (base > 0) convert_numeric_string(t, t->b, base);

                NEW(m_pair, q);
                q->next = t->m_pairs;
                q->name = copy_s(t->s);
                q->value = copy_b(t->b);
                t->m_pairs = q;
                if (t->uplusmode != UPLUS_DEFINED &&
                    (SIZE(t->s) >= 3 && t->s[0] == 'U' && t->s[1] == '+')) {
                    if (t->uplusmode == UPLUS_UNICODE) {
                        error1(t, "U+xxxx already used with implicit meaning");
                    } else {
                        t->uplusmode = UPLUS_DEFINED;
                    }
                }
                continue;
            }
            case c_get: {
                code = read_token(t);
                if (code != c_literalstring) {
                    error1(t, "string omitted after get"); continue;
                }
                t->get_depth++;
                if (t->get_depth > 10) {
                    error1(t, "get directives go 10 deep. Looping?");
                    exit(1);
                }

                NEW(input, q);
                char * file = b_to_sz(t->b);
                int file_owned = 1;
                byte * u = get_input(file);
                if (u == NULL) {
                    struct include * r;
                    for (r = t->includes; r; r = r->next) {
                        byte * s = copy_s(r->s);
                        s = add_sz_to_s(s, file);
                        s[SIZE(s)] = 0;
                        if (file_owned > 0) {
                            free(file);
                        } else {
                            lose_s((byte *)file);
                        }
                        file = (char*)s;
                        file_owned = -1;
                        u = get_input(file);
                        if (u != NULL) break;
                    }
                }
                if (u == NULL) {
                    error(t, "Can't get '", (byte *)file, strlen(file), "'");
                    exit(1);
                }
                memmove(q, t, sizeof(struct input));
                t->next = q;
                t->p = u;
                t->c = 0;
                t->file = file;
                t->file_owned = file_owned;
                t->line_number = 1;

                p = t->p;
                continue;
            }
            case -1:
                if (t->next) {
                    lose_s(p);

                    struct input * q = t->next;
                    memmove(t, q, sizeof(struct input)); p = t->p;
                    FREE(q);

                    t->get_depth--;
                    continue;
                }
                /* fall through */
            default:
                t->previous_token = t->token;
                t->token = code;
                return code;
        }
    }
}

extern int peek_token(struct tokeniser * t) {
    int token = read_token(t);
    t->token_held = true;
    return token;
}

extern const char * name_of_token(int code) {
    for (int i = 1; i < vocab->code; i++)
        if ((vocab + i)->code == code) return (const char *)(vocab + i)->s;
    switch (code) {
        case c_mathassign:   return "=";
        case c_name:         return "name";
        case c_number:       return "number";
        case c_literalstring:return "literal";
        case c_neg:          return "neg";
        case c_grouping:     return "grouping";
        case c_call:         return "call";
        case c_booltest:     return "Boolean test";
        case c_functionend:  return "Function end";
        case c_goto_grouping:
                             return "goto grouping";
        case c_gopast_grouping:
                             return "gopast grouping";
        case c_goto_non:     return "goto non";
        case c_gopast_non:   return "gopast non";
        case c_not_booltest: return "Inverted boolean test";
        case -2:             return "start of text";
        case -1:             return "end of text";
        default:             return "?";
    }
}

extern void disable_token(struct tokeniser * t, int code) {
    t->token_disabled[code] = 1;
}

extern struct tokeniser * create_tokeniser(byte * p, char * file) {
    NEW(tokeniser, t);
    *t = (struct tokeniser){0};
    t->p = p;
    t->file = file;
    t->line_number = 1;
    t->b = create_b(0);
    t->s = create_s(0);
    t->m_start = -1;
    t->token = -2;
    t->previous_token = -2;
    t->uplusmode = UPLUS_NONE;
    return t;
}

extern void close_tokeniser(struct tokeniser * t) {
    lose_b(t->b);
    lose_s(t->s);
    {
        struct m_pair * q = t->m_pairs;
        while (q) {
            struct m_pair * q_next = q->next;
            lose_s(q->name);
            lose_b(q->value);
            FREE(q);
            q = q_next;
        }
    }
    {
        struct input * q = t->next;
        while (q) {
            struct input * q_next = q->next;
            FREE(q);
            q = q_next;
        }
    }
    if (t->file_owned > 0) {
        free(t->file);
    } else if (t->file_owned < 0) {
        lose_s((byte *)t->file);
    }
    FREE(t);
}
