#include <assert.h>
#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strlen */
#include "header.h"

/* Generator functions common to multiple target languages. */

extern struct generator * create_generator(struct analyser * a, struct options * o) {
    NEW(generator, g);
    *g = (struct generator){0};
    g->analyser = a;
    g->options = o;
    g->failure_label = -1;
    g->varname_prefix = "v_";
    g->margin_indent = "    ";
    return g;
}

extern void close_generator(struct generator * g) {
    FREE(g);
}

extern int just_return_on_fail(struct generator * g) {
    return g->failure_label == x_return && str_len(g->failure_str) == 0;
}

extern int tailcallable(struct generator * g, struct node * p) {
    return just_return_on_fail(g) &&
           p->right && p->right->type == c_functionend;
}

// Write a C-style relational operator (also used by some other languages).
extern void write_c_relop(struct generator * g, int relop) {
    switch (relop) {
        case c_eq: write_string(g, " == "); break;
        case c_ne: write_string(g, " != "); break;
        case c_gt: write_string(g, " > "); break;
        case c_ge: write_string(g, " >= "); break;
        case c_lt: write_string(g, " < "); break;
        case c_le: write_string(g, " <= "); break;
        default:
            fprintf(stderr, "Unexpected type #%d in write_c_relop\n", relop);
            exit(1);
    }
}

static void write_comment_literalstring(struct generator * g, const symbol *s,
                                        const char * end) {
    if (end) {
        // Check if the literal string contains the target language end comment
        // string.  Don't try to be clever here as real-world literal strings
        // are unlikely to contain even partial matches.
        int end_len = (int)strlen(end);
        if (end_len <= SIZE(s)) {
            for (int i = 0; i <= SIZE(s) - end_len; ++i) {
                for (int j = 0; j < end_len; ++j) {
                    if (s[i + j] != end[j]) goto next_outer;
                }
                write_string(g, "<literal string>");
                return;
next_outer: ;
            }
        }
    }

    symbol ch_max = 0xa0;
    if (g->options->encoding == ENC_SINGLEBYTE) {
        ch_max = 0xff;
    }

    int i = 0;
    write_char(g, '\'');
    while (i < SIZE(s)) {
        int ch;
        if (g->options->encoding == ENC_UTF8) {
            i += get_utf8(s + i, &ch);
        } else {
            ch = s[i++];
        }
        if (ch == '\'' || ch == '{') {
            write_char(g, '{');
            write_char(g, ch);
            write_char(g, '}');
        } else if (ch < 32 ||
                   (ch >= 127 && ch <= ch_max) ||
                   ch == '\\' ||
                   ch >= 0x590) {
            // Encode characters which are problematic if emitted literally
            // using Snowball-style `{U+xx}`:
            //
            // * Control characters.
            //
            // * For ENC_SINGLEBYTE we encode all non-ASCII to avoid invalid
            //   UTF-8 in comments (which clang warns about for C/C++ with
            //   option `-pedantic` or `-Winvalid-utf8`).
            //
            // * `\`: In Java, `\u000a` in a comment is interpreted as a
            //   newline and so exits the comment, while `\uq` gives
            //   compilation error `illegal unicode escape`.  Since `\` is
            //   unusual in Snowball literal strings we take the simple
            //   approach of escaping it for all target languages.
            //
            // * Anything >= 0x590 as a crude way to avoid LTR characters
            //   affecting the rendering of source character order in confusing
            //   ways.
            write_string(g, "{U+");
            write_hex(g, ch);
            write_char(g, '}');
        } else {
            write_wchar_as_utf8(g, ch);
        }
    }
    write_char(g, '\'');
}

static void write_comment_AE(struct generator * g, struct node * p) {
    switch (p->type) {
        case c_name:
            write_s(g, p->name->s);
            break;
        case c_number:
            write_int(g, p->number);
            break;
        case c_cursor:
        case c_len:
        case c_lenof:
        case c_limit:
        case c_maxint:
        case c_minint:
        case c_size:
        case c_sizeof:
            write_string(g, name_of_token(p->type));
            if (p->name) {
                write_char(g, ' ');
                write_s(g, p->name->s);
            }
            break;
        case c_neg:
            write_char(g, '-');
            write_comment_AE(g, p->right);
            break;
        case c_multiply:
        case c_plus:
        case c_minus:
        case c_divide:
            write_char(g, '(');
            write_comment_AE(g, p->left);
            write_char(g, ' ');
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->right);
            write_char(g, ')');
            break;
        default:
            fprintf(stderr, "Unexpected type #%d in write_comment_AE\n", p->type);
            exit(1);
    }
}

void write_comment_content(struct generator * g, struct node * p,
                           const char * end) {
    switch (p->type) {
        case c_assign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
            if (p->name) {
                write_char(g, '$');
                write_s(g, p->name->s);
                write_char(g, ' ');
            }
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->AE);
            break;
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
            write_string(g, "$(");
            write_comment_AE(g, p->left);
            write_char(g, ' ');
            write_string(g, name_of_token(p->type));
            write_char(g, ' ');
            write_comment_AE(g, p->AE);
            write_char(g, ')');
            break;
        case c_define:
            if (p->mode == m_forward) {
                write_string(g, "forwardmode define ");
            } else {
                write_string(g, "backwardmode define ");
            }
            write_s(g, p->name->s);
            break;
        case c_literalstring:
            write_comment_literalstring(g, p->literalstring, end);
            break;
        case c_call:
        case c_grouping:
        case c_name:
            write_s(g, p->name->s);
            break;
        default:
            write_string(g, name_of_token(p->type));
            if (p->name) {
                write_char(g, ' ');
                write_s(g, p->name->s);
            } else if (p->literalstring) {
                write_char(g, ' ');
                write_comment_literalstring(g, p->literalstring, end);
            }
    }
    write_string(g, ", line ");
    write_int(g, p->line_number);
}

void write_generated_comment_content(struct generator * g) {
    // Report only the leafname of the Snowball source file to make output
    // reproducible even if an absolute path to the source file is specified.
    write_string(g, "Generated from ");
    const char * leaf = g->analyser->tokeniser->file;
    const char * p = strrchr(leaf, '/');
    if (p) leaf = p + 1;
    p = strrchr(leaf, '\\');
    if (p) leaf = p + 1;
    write_string(g, leaf);
    write_string(g, " by Snowball " SNOWBALL_VERSION " - https://snowballstem.org/");
}

void write_start_comment(struct generator * g,
                         const char * comment_start,
                         const char * comment_end) {
    write_margin(g);
    write_string(g, comment_start);
    write_generated_comment_content(g);
    if (comment_end) {
        write_string(g, comment_end);
    }
    write_newline(g);
    write_newline(g);
}

extern struct str * vars_newname(struct generator * g) {
    struct str * output;
    g->var_number++;
    output = str_new();
    str_append_string(output, g->varname_prefix);
    str_append_int(output, g->var_number);
    return output;
}

extern void write_margin(struct generator * g) {
    for (int i = 0; i < g->margin; i++) write_string(g, g->margin_indent);
}

/* K_needed() tests to see if we really need to keep c. Not true when the
   command does not touch the cursor (and in backwardmode, also does not
   change the limit by inserting, deleting, or replacing text in the string).
   This and repeat_score() could be elaborated almost indefinitely.
*/

static int K_needed_(struct node * p, int call_depth);

static int K_needed_node(struct node * p, int call_depth) {
    switch (p->type) {
        case c_assignto:
        case c_do:
        case c_dollar:
        case c_leftslice:
        case c_rightslice:
        case c_assign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
        case c_sliceto:
        case c_booltest:
        case c_not_booltest:
        case c_set:
        case c_unset:
        case c_true:
        case c_false:
        case c_debug:
        case c_functionend:
            // Doesn't change the cursor or always restores it.
            break;

        case c_stringassign:
            // Doesn't change the cursor in forwards mode; in backwards
            // mode the cursor and forwards limit move in step.
            break;

        case c_attach:
            // Cursor modified in backwardmode.
            if (p->mode == m_backward) return true;
            break;

        case c_insert:
            // Cursor modified in forwards mode.
            if (p->mode == m_forward) return true;
            break;

        case c_call:
            /* Recursive functions aren't typical in snowball programs, so
             * make the pessimistic assumption that keep is needed if we
             * hit a generous limit on recursion.  It's not likely to make
             * a difference to any real world program, but means we won't
             * recurse until we run out of stack for pathological cases.
             */
            if (call_depth >= 100) return true;
            if (K_needed_(p->name->definition->left, call_depth + 1))
                return true;
            break;

        case c_bra:
        case c_loop:
        case c_fail:
            if (K_needed_(p->left, call_depth)) return true;
            break;

        case c_backwards:
        case c_reverse:
        case c_test:
            if (p->possible_signals != 1) return true;
            // Restores cursor on t and the subcommand can't fail.
            break;

        default: return true;
    }
    return false;
}

static int K_needed_(struct node * p, int call_depth) {
    while (p) {
        if (K_needed_node(p, call_depth)) return true;
        p = p->right;
    }
    return false;
}

extern int K_needed(struct node * p) {
    return K_needed_(p, 0);
}

static int K_needed_node_on_f_(struct node * p, int call_depth) {
    switch (p->type) {
        case c_assignto:
        case c_do:
        case c_dollar:
        case c_leftslice:
        case c_rightslice:
        case c_assign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
        case c_sliceto:
        case c_booltest:
        case c_not_booltest:
        case c_set:
        case c_unset:
        case c_true:
        case c_false:
        case c_debug:
        case c_functionend:
            // Doesn't change the cursor or always restores it.
            break;

        case c_grouping:
        case c_literalstring:
        case c_name:
        case c_non:
        case c_hop:
        case c_next:
        case c_substring:
        case c_tomark:
            // Doesn't modify the cursor on failure.
            break;

        case c_repeat:
        case c_slicefrom:
        case c_tolimit:
        case c_attach:
        case c_insert:
            // Can't fail, so can't modify the cursor on failure.
            break;

        case c_goto:
        case c_try:
            // Restores the cursor on failure.
            break;

        case c_gopast:
            // Restores the cursor on failure if repeat_restore() is true.
            if (!repeat_restore(p->left)) return true;
            break;

        case c_call:
            /* Recursive functions aren't typical in snowball programs, so
             * make the pessimistic assumption that keep is needed if we
             * hit a generous limit on recursion.  It's not likely to make
             * a difference to any real world program, but means we won't
             * recurse until we run out of stack for pathological cases.
             */
            if (call_depth >= 100) return true;
            if (K_needed_(p->name->definition->left, call_depth + 1))
                return true;
            break;

        case c_bra:
        case c_loop:
        case c_fail:
            if (K_needed_(p->left, call_depth)) return true;
            break;

        case c_backwards:
        case c_reverse:
        case c_test:
            if (p->possible_signals != 1) return true;
            // Restores cursor on t and the subcommand can't fail.
            break;

        default:
            // FIXME: Can we handle c_or c_and c_among c_atleast c_setlimit
            // better?
            return true;
    }
    return false;
}

extern int K_needed_node_on_f(struct node * p) {
    return K_needed_node_on_f_(p, 0);
}

// Like K_needed(), but for the sub-node chain of c_or.  We only restore on
// signal f, and the cursor only needs to be restored between nodes so we don't
// need to check the final node in the chain.
extern int K_needed_for_or(struct node * p) {
    while (p->right) {
        if (K_needed_node_on_f(p)) return true;
        p = p->right;
    }
    return false;
}

// Like K_needed(), but for the sub-node chain of c_and.  The cursor only needs
// to be restored between nodes so we don't need to check the final node in the
// chain.
extern int K_needed_for_and(struct node * p) {
    while (p->right) {
        if (K_needed_node(p, 0)) return true;
        p = p->right;
    }
    return false;
}

static int repeat_score(struct node * p, int call_depth) {
    int score = 0;
    while (p) {
        switch (p->type) {
            case c_dollar:
            case c_leftslice:
            case c_rightslice:
            case c_assign:
            case c_plusassign:
            case c_minusassign:
            case c_multiplyassign:
            case c_divideassign:
            case c_eq:
            case c_ne:
            case c_gt:
            case c_ge:
            case c_lt:
            case c_le:
            case c_sliceto:   /* case c_not: must not be included here! */
            case c_booltest:
            case c_not_booltest:
            case c_set:
            case c_unset:
            case c_true:
            case c_false:
            case c_debug:
            case c_functionend:
                break;

            case c_call:
                /* Recursive functions aren't typical in snowball programs, so
                 * make the pessimistic assumption that repeat requires cursor
                 * reinstatement if we hit a generous limit on recursion.  It's
                 * not likely to make a difference to any real world program,
                 * but means we won't recurse until we run out of stack for
                 * pathological cases.
                 */
                if (call_depth >= 100) {
                    return 2;
                }
                score += repeat_score(p->name->definition->left, call_depth + 1);
                if (score >= 2)
                    return score;
                break;

            case c_bra:
                score += repeat_score(p->left, call_depth);
                if (score >= 2)
                    return score;
                break;

            case c_name:
            case c_literalstring:
            case c_next:
            case c_grouping:
            case c_non:
#if 0
            // These could be here if the target-language helpers all preserved
            // the cursor on failure:
            case c_goto_grouping:
            case c_gopast_grouping:
            case c_goto_non:
            case c_gopast_non:
#endif
            case c_hop:
                if (++score >= 2)
                    return score;
                break;

            default:
                return 2;
        }
        p = p->right;
    }
    return score;
}

/* tests if an expression requires cursor reinstatement in a repeat */
extern int repeat_restore(struct node * p) {
    return repeat_score(p, 0) >= 2;
}

extern bool
amongvar_needed(struct node * p)
{
    if (!p) return false;
    if (p->among && p->among->amongvar_needed) return true;
    return amongvar_needed(p->left) ||
           amongvar_needed(p->right) ||
           amongvar_needed(p->aux);
}

/* Language-independent write routines for simple entities */

static void write_hexdigit(struct generator * g, unsigned i) {
    str_append_ch(g->outbuf, "0123456789ABCDEF"[i & 0xF]); /* hexchar */
}

extern void write_hex4(struct generator * g, unsigned ch) {
    for (int i = 12; i >= 0; i -= 4) write_hexdigit(g, ch >> i);
}

extern void write_hex(struct generator * g, unsigned i) {
    if (i >> 4) write_hex(g, i >> 4);
    write_hexdigit(g, i); /* hex integer */
}

extern void write_octal3(struct generator * g, unsigned n) {
    assert(n < 256);
    write_char(g, '0' + ((n >> 6) & 0x03));
    write_char(g, '0' + ((n >> 3) & 0x07));
    write_char(g, '0' + (n & 0x07));
}

extern void write_char(struct generator * g, int ch) {
    str_append_ch(g->outbuf, ch); /* character */
}

extern void write_newline(struct generator * g) {
    /* Avoid generating trailing whitespace. */
    while (true) {
        int ch = str_back(g->outbuf);
        if (ch != ' ' && ch != '\t') break;
        str_pop(g->outbuf);
    }
    str_append_ch(g->outbuf, '\n'); /* newline */
    g->line_count++;
}

extern void write_string(struct generator * g, const char * s) {
    str_append_string(g->outbuf, s);
}

extern void write_wchar_as_utf8(struct generator * g, symbol ch) {
    str_append_wchar_as_utf8(g->outbuf, ch);
}

extern void write_int(struct generator * g, int i) {
    str_append_int(g->outbuf, i);
}

// Write an integer, left-padded to width 3 with spaces.
extern void wi3(struct generator * g, int i) {
    if (i < 100) write_char(g, ' ');
    if (i < 10)  write_char(g, ' ');
    write_int(g, i);
}

extern void write_s(struct generator * g, const byte * s) {
    str_append_s(g->outbuf, s);
}

extern void write_str(struct generator * g, struct str * str) {
    str_append(g->outbuf, str);
}
