#include <assert.h>
#include <stdio.h> /* for fprintf etc */
#include <stdlib.h> /* for exit */
#include <string.h> /* for strlen */
#include "header.h"

// C90 guarantees 31 significant initial characters in an internal identifier.
#define C_MAX_ID_LEN 31

/* Define this to get warning messages when optimisations can't be used. */
/* #define OPTIMISATION_WARNINGS */

/* prototype functions for recursive use: */

static void generate(struct generator * g, struct node * p);
static void w(struct generator * g, const char * s);
static void writef(struct generator * g, const char * s, struct node * p);

static bool can_error_(struct node * p);

// `can_error(routine_name)` returns true if the C implementation of
// `routine_name` can return -1.  -1 indicates an internal or system type error
// (e.g. memory allocation failed or the slice wasn't valid) for C (for C++
// we instead throw exceptions for such cases).
static bool can_error(struct name * n) {
    return can_error_(n->definition->left);
}

static bool can_error_(struct node * p) {
    while (p) {
        switch (p->type) {
            case c_sliceto:
            case c_assignto:
            case c_slicefrom:
            case c_attach:
            case c_insert:
            case c_stringassign:
                return true;
            case c_call:
                // Recursive functions are rare so just assume the worst.
                if (p->name->recursive) return true;
                if (can_error(p->name)) return true;
                break;
            case c_among: {
                struct among * x = p->among;
                if (x->unique_function_count == 0) break;
                for (int i = 0; i < x->literalstring_count; ++i) {
                    struct name * function = x->v[i].function;
                    if (function && can_error(function)) return true;
                }
                break;
            }
        }
        if (p->left && can_error_(p->left)) return true;
        if (p->aux && can_error_(p->aux)) return true;
        p = p->right;
    }
    return false;
}

/* Write routines for items from the syntax tree */

static void write_relop(struct generator * g, int relop) {
    write_c_relop(g, relop);
}

static void write_varname(struct generator * g, struct name * p) {
    if (p->type == t_external) {
        if (g->options->target_lang == LANG_CPLUSPLUS) {
            write_string(g, g->options->package);
            write_string(g, "::");
            write_s(g, g->options->name);
            write_string(g, "::");
        }
        if (g->options->externals_prefix) {
            write_string(g, g->options->externals_prefix);
        }
    } else {
        /* Name variables using their Snowball name prefixed by s_, b_ or i_
         * depending on their type.
         *
         * We use the same naming scheme for both global and local variables.
         */
        write_char(g, "sbirxg"[p->type]);
        write_char(g, '_');
        if (g->options->target_lang == LANG_C) {
            if (SIZE(p->s) > C_MAX_ID_LEN - 2) {
                // We aim to generate C90 code, and C90 only guarantees 31
                // significant initial characters in internal identifiers.
                // C99 raised this to 63, and modern implementations are
                // likely to have a high limit or not impose one, but it
                // is easy to generate an identifier based on the number
                // instead.  A Snowball name must start with a letter so
                // this can't collide.
                write_int(g, p->count);
                return;
            }
        } else {
            assert(g->options->target_lang == LANG_CPLUSPLUS);
            for (int i = SIZE(p->s) - 1; i > 0; --i) {
                if (p->s[i] == '_' && p->s[i - 1] == '_') {
                    // C++ reserves identifiers containing a double underscore
                    // so generate an identifier based on the number instead.
                    // A Snowball name must start with a letter so this can't
                    // collide.
                    write_int(g, p->count);
                    return;
                }
            }
        }
    }
    write_s(g, p->s);
}

/* Reference to variable, e.g. when assigning to or using in an expression. */
static void write_varref(struct generator * g, struct name * p) {
    if (p->type < t_routine && p->local_to == NULL) {
        write_string(g, "((SN_local *)z)->");
    }
    write_varname(g, p);
}

/* write character literal */
static void wlitch(struct generator * g, int ch) {
    if (32 <= ch && ch < 127) {
        write_char(g, '\'');
        if (ch == '\'' || ch == '\\') {
            write_char(g, '\\');
        }
        write_char(g, ch);
        write_char(g, '\'');
    } else {
        write_string(g, "0x");
        write_hex(g, ch);
    }
}

static void wlitarray(struct generator * g, const symbol * p) {  /* write literal array */
    write_string(g, "{ ");
    for (int i = 0; i < SIZE(p); i++) {
        if (i) write_string(g, ", ");
        wlitch(g, p[i]);
    }
    write_string(g, " }");
}

static void wlitref(struct generator * g, const symbol * p) {  /* write ref to literal array */
    if (SIZE(p) == 0) {
        write_char(g, '0');
    } else {
        struct str * s = g->outbuf;
        g->outbuf = g->declarations;
        write_string(g, "static const symbol s_"); write_int(g, g->literalstring_count); write_string(g, "[] = ");
        wlitarray(g, p);
        write_string(g, ";\n");
        g->outbuf = s;
        write_string(g, "s_"); write_int(g, g->literalstring_count);
        g->literalstring_count++;
    }
}

static void write_comment(struct generator * g, struct node * p) {
    if (!g->options->comments) return;
    write_margin(g);
    write_string(g, "/* ");
    write_comment_content(g, p, "*/");
    write_string(g, " */");
    write_newline(g);
}

static void write_block_start(struct generator * g) {
    w(g, "~M{~+~N");
}

static void write_block_end(struct generator * g) {
    if (g->line_labelled == g->line_count) {
        // Before C23, `;` is required between a label and the block end.
        w(g, "~M;~N");
    }
    w(g, "~-~M}~N");
}

static void write_savecursor(struct generator * g, struct node * p,
                             struct str * savevar) {
    g->B[0] = str_data(savevar);
    g->S[1] = "";
    if (p->mode != m_forward) g->S[1] = "z->l - ";
    writef(g, "~Mint ~B0 = ~S1z->c;~N", p);
}

static void append_restore_string(struct node * p, struct str * out, struct str * savevar) {
    str_append_string(out, "z->c = ");
    if (p->mode != m_forward) str_append_string(out, "z->l - ");
    str_append(out, savevar);
    str_append_ch(out, ';');
}

static void write_restorecursor(struct generator * g, struct node * p, struct str * savevar) {
    write_margin(g);
    append_restore_string(p, g->outbuf, savevar);
    write_newline(g);
}

static void winc(struct generator * g, struct node * p) {     /* increment c */
    write_string(g, p->mode == m_forward ? "z->c++;" :
                                 "z->c--;");
}

static void wsetl(struct generator * g, int n) {
    g->margin--;
    write_margin(g);
    write_string(g, "lab");
    write_int(g, n);
    write_char(g, ':');
    write_newline(g);
    g->line_labelled = g->line_count;
    g->margin++;
}

static void write_failure(struct generator * g) {
    if (str_len(g->failure_str) != 0) {
        write_string(g, "{ ");
        write_str(g, g->failure_str);
        write_char(g, ' ');
    }
    switch (g->failure_label) {
        case x_return:
            write_string(g, "return 0;");
            break;
        default:
            write_string(g, "goto lab");
            write_int(g, g->failure_label);
            write_char(g, ';');
            g->label_used = 1;
    }
    if (str_len(g->failure_str) != 0) write_string(g, " }");
}

/* if at limit fail */
static void write_check_limit(struct generator * g, struct node * p) {
    if (p->mode == m_forward) {
        write_string(g, "if (z->c >= z->l) ");
    } else {
        write_string(g, "if (z->c <= z->lb) ");
    }
    write_failure(g);
}

static void write_data_address(struct generator * g, struct node * p) {
    symbol * b = p->literalstring;
    if (b != NULL) {
        write_int(g, SIZE(b)); w(g, ", ");
        wlitref(g, b);
    } else {
        write_varref(g, p->name);
    }
}

/* Formatted write. */
static void writef(struct generator * g, const char * input, struct node * p) {
    int i = 0;

    while (input[i]) {
        int ch = input[i++];
        if (ch != '~') {
            write_char(g, ch);
            continue;
        }
        ch = input[i++];
        switch (ch) {
            case '~': write_char(g, '~'); continue;
            case 'f': write_failure(g); continue;
            case 'M': write_margin(g); continue;
            case 'N': write_newline(g); continue;
            case '{': write_block_start(g); continue;
            case '}': write_block_end(g); continue;
            case 'S': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->S) / sizeof(g->S[0]))) {
                    printf("Invalid escape sequence ~%c%c in writef(g, \"%s\", p)\n",
                           ch, input[i - 1], input);
                    exit(1);
                }
                write_string(g, g->S[j]);
                continue;
            }
            case 'B': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->B) / sizeof(g->B[0])))
                    goto invalid_escape2;
                write_s(g, g->B[j]);
                continue;
            }
            case 'F': { // Among function dispatcher.
                struct among * x = p->among;
                if (x->unique_function_count == 0) {
                    write_char(g, '0');
                    continue;
                }

                if (x->unique_function_count == 1) {
                    // Only one different function used in this among.
                    struct amongvec * v = x->v;
                    for (int j = 0; j < x->literalstring_count; j++) {
                        if (v[j].function) {
                            write_varref(g, v[j].function);
                            goto continue_outer_loop;
                        }
                    }
                    fprintf(stderr, "unique_function_count == 1 but no among functions\n");
                    exit(1);
continue_outer_loop:
                    continue;
                }

                w(g, "af_");
                write_int(g, x->number);
                continue;
            }
            case 'I':
            case 'J':
            case 'c': {
                int j = input[i++] - '0';
                if (j < 0 || j > (int)(sizeof(g->I) / sizeof(g->I[0])))
                    goto invalid_escape2;
                if (ch == 'I')
                    write_int(g, g->I[j]);
                else if (ch == 'J')
                    wi3(g, g->I[j]);
                else
                    wlitch(g, g->I[j]);
                continue;
            }
            case 'V':
                write_varref(g, p->name);
                continue;
            case 'W':
                write_varname(g, p->name);
                continue;
            case 'L':
                wlitref(g, p->literalstring);
                continue;
            case 's':
                write_int(g, SIZE(p->literalstring));
                continue;
            case 'a': write_data_address(g, p); continue;
            case '+': g->margin++; continue;
            case '-': g->margin--; continue;
            case 'n': write_s(g, g->options->name); continue;
            case '$': /* insert_s, insert_v etc */
                write_char(g, p->literalstring == NULL ? 'v' : 's');
                continue;
            case 'p':
                if (g->options->externals_prefix)
                    write_string(g, g->options->externals_prefix);
                continue;
            default:
                printf("Invalid escape sequence ~%c in writef(g, \"%s\", p)\n",
                       ch, input);
                exit(1);
            invalid_escape2:
                printf("Invalid escape sequence ~%c%c in writef(g, \"%s\", p)\n",
                       ch, input[i - 1], input);
                exit(1);
        }
    }
}

static void w(struct generator * g, const char * s) {
    writef(g, s, NULL);
}

/* Write out a statement with additional code to propagate a negative return
 * value which indicates an error.
 *
 * When generating C++, such errors throw exceptions so we don't need to
 * check for negative return values.
 */
static void write_propagating_error(struct generator * g, const char * s,
                                    int keep_c,
                                    struct node *p) {
    bool error_possible = true;
    if (g->options->target_lang == LANG_CPLUSPLUS) {
        error_possible = false;
    } else if (p->type == c_call && !can_error(p->name)) {
        error_possible = false;
    }
    if (!error_possible) {
        if (keep_c) {
            write_block_start(g);
            w(g, "~Mint saved_c = z->c;~N");
        }
        write_margin(g);
        writef(g, s, p);
        w(g, ";~N");
        if (keep_c) {
            w(g, "~Mz->c = saved_c;~N");
            write_block_end(g);
        }
    } else {
        write_block_start(g);
        if (keep_c) {
            w(g, "~Mint saved_c = z->c;~N");
        }
        w(g, "~Mint ret = ");
        writef(g, s, p);
        w(g, ";~N");
        if (keep_c) {
            w(g, "~Mz->c = saved_c;~N");
        }
        w(g, "~Mif (ret < 0) return ret;~N");
        write_block_end(g);
    }
}

static void generate_AE(struct generator * g, struct node * p) {
    const char * s;
    switch (p->type) {
        case c_name:
            write_varref(g, p->name);
            break;
        case c_number:
            write_int(g, p->number);
            break;
        case c_maxint:
            write_string(g, "INT_MAX");
            break;
        case c_minint:
            write_string(g, "INT_MIN");
            break;
        case c_neg:
            write_char(g, '-');
            generate_AE(g, p->right);
            break;
        case c_multiply:
            s = " * ";
            goto label0;
        case c_divide:
            s = " / ";
            goto label0;
        case c_plus:
            s = " + ";
            goto label0;
        case c_minus:
            s = " - ";
        label0:
            write_char(g, '(');
            generate_AE(g, p->left);
            write_string(g, s);
            generate_AE(g, p->right);
            write_char(g, ')');
            break;
        case c_cursor:
            w(g, "z->c");
            break;
        case c_limit:
            w(g, p->mode == m_forward ? "z->l" : "z->lb");
            break;
        case c_len:
            if (g->options->encoding == ENC_UTF8) {
                w(g, "len_utf8(z->p)");
                break;
            }
            /* FALLTHRU */
        case c_size:
            w(g, "SIZE(z->p)");
            break;
        case c_lenof:
            if (g->options->encoding == ENC_UTF8) {
                writef(g, "len_utf8(~V)", p);
                break;
            }
            /* FALLTHRU */
        case c_sizeof:
            writef(g, "SIZE(~V)", p);
            break;
    }
}

static void generate_bra(struct generator * g, struct node * p) {
    write_comment(g, p);
    p = p->left;
    while (p) {
        generate(g, p);
        p = p->right;
    }
}

static void generate_and(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed_for_and(p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);

    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    p = p->left;
    while (p) {
        generate(g, p);
        if (savevar && p->right != NULL) write_restorecursor(g, p, savevar);
        p = p->right;
    }

    if (savevar) {
        write_block_end(g);
        str_delete(savevar);
    }
}

static void generate_or(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed_for_or(p->left)) {
        savevar = vars_newname(g);
    }

    int used = g->label_used;
    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    write_comment(g, p);
    w(g, "~Mdo {~N~+");

    if (savevar) {
        write_savecursor(g, p, savevar);
    }

    p = p->left;
    str_clear(g->failure_str);

    if (p == NULL) {
        /* p should never be NULL after an or: there should be at least two
         * sub nodes. */
        fprintf(stderr, "Error: \"or\" node without children nodes.");
        exit(1);
    }
    while (p->right != NULL) {
        int label = new_label(g);
        g->failure_label = label;
        g->label_used = 0;
        generate(g, p);
        w(g, "~Mbreak;~N");

        if (g->label_used)
            wsetl(g, g->failure_label);
        if (savevar && K_needed_node_on_f(p)) {
            write_restorecursor(g, p, savevar);
        }
        p = p->right;
    }

    g->label_used = used;
    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    generate(g, p);

    write_block_end(g);
    if (str_back(g->outbuf) == '\n') {
        str_pop(g->outbuf);
    }
    w(g, " while (0);~N");

    if (savevar) {
        str_delete(savevar);
    }
}

static void generate_backwards(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~Mz->lb = z->c; z->c = z->l;~N", p);
    generate(g, p->left);
    w(g, "~Mz->c = z->lb;~N");
}

static void generate_not(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed_node_on_f(p->left)) {
        savevar = vars_newname(g);
    }

    int used = g->label_used;
    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    write_comment(g, p);
    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    int label = new_label(g);
    g->failure_label = label;
    str_clear(g->failure_str);
    g->label_used = 0;
    generate(g, p->left);

    int l = g->failure_label;
    int u = g->label_used;

    g->label_used = used;
    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    writef(g, "~M~f~N", p);
    if (u)
        wsetl(g, l);

    if (savevar) {
        write_restorecursor(g, p, savevar);
        write_block_end(g);
        str_delete(savevar);
    }
}

static void generate_try(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);
    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    int label = new_label(g);
    g->failure_label = label;
    str_clear(g->failure_str);
    g->label_used = 0;
    if (savevar) {
        append_restore_string(p, g->failure_str, savevar);
    }

    generate(g, p->left);

    if (g->label_used)
        wsetl(g, g->failure_label);

    if (savevar) {
        write_block_end(g);
        str_delete(savevar);
    }
}

static void generate_set(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->target_lang == LANG_CPLUSPLUS) {
        writef(g, "~M~V = true;~N", p);
    } else {
        writef(g, "~M~V = 1;~N", p);
    }
}

static void generate_unset(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->target_lang == LANG_CPLUSPLUS) {
        writef(g, "~M~V = false;~N", p);
    } else {
        writef(g, "~M~V = 0;~N", p);
    }
}

static void generate_fail(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate(g, p->left);
    writef(g, "~M~f~N", p);
}

/* generate_test() also implements 'reverse' */
static void generate_test(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);

    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    generate(g, p->left);

    if (savevar) {
        write_restorecursor(g, p, savevar);
        write_block_end(g);
        str_delete(savevar);
    }
}

static void generate_do(struct generator * g, struct node * p) {
    struct str * savevar = NULL;
    if (K_needed(p->left)) {
        savevar = vars_newname(g);
    }

    write_comment(g, p);
    if (savevar) {
        write_block_start(g);
        write_savecursor(g, p, savevar);
    }

    if (p->left->type == c_call) {
        /* Optimise do <call> */
        write_comment(g, p->left);
        write_propagating_error(g, "~V(z)", false, p->left);
    } else {
        int label = new_label(g);
        g->failure_label = label;
        g->label_used = 0;
        str_clear(g->failure_str);

        generate(g, p->left);

        if (g->label_used)
            wsetl(g, g->failure_label);
    }

    if (savevar) {
        write_restorecursor(g, p, savevar);
        write_block_end(g);
        str_delete(savevar);
    }
}

static void generate_next(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->encoding == ENC_UTF8) {
        if (p->mode == m_forward)
            w(g, "~{~Mint ret = skip_utf8(z->p, z->c, z->l, 1");
        else
            w(g, "~{~Mint ret = skip_b_utf8(z->p, z->c, z->lb, 1");
        writef(g, ");~N"
              "~Mif (ret < 0) ~f~N"
              "~Mz->c = ret;~N"
              "~}", p);
    } else {
        write_margin(g);
        write_check_limit(g, p);
        write_newline(g);
        write_margin(g);
        winc(g, p);
        write_newline(g);
    }
}

static void generate_GO_grouping(struct generator * g, struct node * p, int is_goto, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "in" : "out";
    g->S[2] = g->options->encoding == ENC_UTF8 ? "_U" : "";
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    if (is_goto) {
        writef(g, "~Mif (~S1_grouping~S0~S2(z, ~V, ~I0, ~I1, 1) < 0) ~f~N", p);
    } else {
        writef(g, "~{"
              "~Mint ret = ~S1_grouping~S0~S2(z, ~V, ~I0, ~I1, 1);~N"
              "~Mif (ret < 0) ~f~N", p);

        if (p->mode == m_forward)
            w(g, "~Mz->c += ret;~N");
        else
            w(g, "~Mz->c -= ret;~N");
        w(g, "~}");
    }
}

static void generate_GO(struct generator * g, struct node * p, int is_goto) {
    write_comment(g, p);

    int used = g->label_used;
    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    w(g, "~Mwhile (1) {~N~+");

    struct str * savevar = NULL;
    if (is_goto || repeat_restore(p->left)) {
        savevar = vars_newname(g);
        write_savecursor(g, p, savevar);
    }

    int label = new_label(g);
    g->failure_label = label;
    g->label_used = 0;
    str_clear(g->failure_str);
    generate(g, p->left);

    /* include for goto; omit for gopast */
    if (is_goto) write_restorecursor(g, p, savevar);
    w(g, "~Mbreak;~N");

    if (g->label_used)
        wsetl(g, g->failure_label);
    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    g->label_used = used;
    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    generate_next(g, p);

    w(g, "~}");
}

static void generate_loop(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->target_lang == LANG_C) {
        w(g, "~{~Mint i; for (i = ");
    } else {
        w(g, "~Mfor (int i = ");
    }
    generate_AE(g, p->AE);
    writef(g, "; i > 0; i--) {~N~+", p);

    generate(g, p->left);

    w(g, "~}");
    if (g->options->target_lang == LANG_C) {
        w(g, "~}");
    }
}

static void generate_repeat_or_atleast(struct generator * g, struct node * p, struct str * loopvar) {
    writef(g, "~Mwhile (1) {~+~N", p);

    struct str * savevar = NULL;
    if (repeat_restore(p->left)) {
        savevar = vars_newname(g);
        write_savecursor(g, p, savevar);
    }

    int label = new_label(g);
    g->failure_label = label;
    g->label_used = 0;
    str_clear(g->failure_str);

    generate(g, p->left);

    if (loopvar != NULL) {
        g->B[0] = str_data(loopvar);
        w(g, "~M~B0--;~N");
    }

    w(g, "~Mcontinue;~N");

    if (g->label_used)
        wsetl(g, g->failure_label);

    if (savevar) {
        write_restorecursor(g, p, savevar);
        str_delete(savevar);
    }

    w(g, "~Mbreak;~N~}");
}

static void generate_repeat(struct generator * g, struct node * p) {
    write_comment(g, p);
    generate_repeat_or_atleast(g, p, NULL);
}

static void generate_atleast(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct str * loopvar = vars_newname(g);
    g->B[0] = str_data(loopvar);
    w(g, "~{~Mint ~B0 = ");
    generate_AE(g, p->AE);
    w(g, ";~N");
    {
        int used = g->label_used;
        int a0 = g->failure_label;
        struct str * a1 = str_copy(g->failure_str);

        generate_repeat_or_atleast(g, p, loopvar);

        g->label_used = used;
        g->failure_label = a0;
        str_delete(g->failure_str);
        g->failure_str = a1;
    }
    g->B[0] = str_data(loopvar);
    writef(g, "~Mif (~B0 > 0) ~f~N", p);
    w(g, "~}");
    str_delete(loopvar);
}

static void generate_tomark(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? ">" : "<";

    w(g, "~Mif (z->c ~S0 "); generate_AE(g, p->AE); writef(g, ") ~f~N", p);
    w(g, "~Mz->c = "); generate_AE(g, p->AE); writef(g, ";~N", p);
}

static void generate_hop(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (g->options->encoding == ENC_UTF8) {
        g->S[0] = p->mode == m_forward ? "" : "_b";
        g->S[1] = p->mode == m_forward ? "z->l" : "z->lb";
        w(g, "~{");
        if (p->AE->type == c_number) {
            // Constant distance hop.
            //
            // No need to check for negative hop as that's converted to false by
            // the analyser.
            g->I[0] = p->AE->number;
            w(g, "~Mint ret = skip~S0_utf8(z->p, z->c, ~S1, ~I0);~N");
        } else {
            w(g, "~Mint ae = ");
            generate_AE(g, p->AE);
            w(g, ";~N");
            w(g, "~Mint ret = ae >= 0 ? skip~S0_utf8(z->p, z->c, ~S1, ae) : -1;~N");
        }
        w(g, "~Mif (ret < 0) ~f~N");
        w(g, "~Mz->c = ret;~N");
        w(g, "~}");
    } else {
        // Fixed-width characters.
        g->S[0] = p->mode == m_forward ? "+" : "-";
        if (p->AE->type == c_number) {
            // Constant distance hop.
            //
            // No need to check for negative hop as that's converted to false by
            // the analyser.
            g->I[0] = p->AE->number;
            if (p->mode == m_forward) {
                writef(g, "~Mif (z->c ~S0 ~I0 > z->l) ~f~N", p);
            } else {
                writef(g, "~Mif (z->c ~S0 ~I0 < z->lb) ~f~N", p);
            }
            writef(g, "~Mz->c ~S0= ~I0;~N", p);
        } else {
            w(g, "~{~Mint ret = z->c ~S0 ");
            generate_AE(g, p->AE);
            writef(g, ";~N", p);
            if (p->mode == m_forward) {
                writef(g, "~Mif (ret > z->l || ret < z->c) ~f~N", p);
            } else {
                writef(g, "~Mif (ret < z->lb || ret > z->c) ~f~N", p);
            }
            writef(g, "~Mz->c = ret;~N"
                      "~}", p);
        }
    }
}

static void generate_tolimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "b";
    writef(g, "~Mz->c = z->l~S0;~N", p);
}

static void generate_leftslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "bra" : "ket";
    writef(g, "~Mz->~S0 = z->c;~N", p);
}

static void generate_rightslice(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "ket" : "bra";
    writef(g, "~Mz->~S0 = z->c;~N", p);
}

static void generate_assignto(struct generator * g, struct node * p) {
    write_comment(g, p);
    write_propagating_error(g, "assign_to(z, &~V)", false, p);
}

static void generate_sliceto(struct generator * g, struct node * p) {
    write_comment(g, p);
    write_propagating_error(g, "slice_to(z, &~V)", false, p);
}

static void generate_insert(struct generator * g, struct node * p, int style) {
    write_comment(g, p);
    int keep_c = style == c_attach;
    if (p->mode == m_backward) keep_c = !keep_c;
    write_propagating_error(g, "insert_~$(z, z->c, z->c, ~a)", keep_c, p);
}

static void generate_stringassign(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->mode == m_forward) {
        /* like 'attach' */
        write_propagating_error(g, "insert_~$(z, z->c, z->l, ~a)", true, p);
    } else {
        write_propagating_error(g, "insert_~$(z, z->lb, z->c, ~a)", false, p);
    }
}

static void generate_slicefrom(struct generator * g, struct node * p) {
    write_comment(g, p);
    if (p->literalstring && SIZE(p->literalstring) == 0) {
        write_propagating_error(g, "slice_del(z)", false, p);
        return;
    }
    write_propagating_error(g, "slice_from_~$(z, ~a)", false, p);
}

static void generate_setlimit(struct generator * g, struct node * p) {
    write_comment(g, p);
    struct str * varname = vars_newname(g);

    bool extra_block = false;
    if (p->left && p->left->type == c_tomark) {
        /* Special case for:
         *
         *   setlimit tomark AE for C
         *
         * All uses of setlimit in the current stemmers we ship follow this
         * pattern, and by special-casing we can avoid having to save and
         * restore c.
         */
        struct node * q = p->left;
        write_comment(g, q);
        assert(q->right == NULL);

        g->B[0] = str_data(varname);
        writef(g, "~{~Mint ~B0;~N", p);

        g->S[0] = q->mode == m_forward ? ">" : "<";
        w(g, "~Mif (z->c ~S0 "); generate_AE(g, q->AE); writef(g, ") ~f~N", q);

        g->B[0] = str_data(varname);
        if (p->mode == m_forward) {
            w(g, "~M~B0 = z->l; z->l = ");
            generate_AE(g, q->AE);
            w(g, "; ~B0 -= z->l;~N");
        } else {
            w(g, "~M~B0 = z->lb; z->lb = ");
            generate_AE(g, q->AE);
            w(g, ";~N");
        }

        if (p->mode == m_forward) {
            str_assign(g->failure_str, "z->l += ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        } else {
            str_assign(g->failure_str, "z->lb = ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        }
    } else {
        write_block_start(g);
        extra_block = true;
        struct str * savevar = vars_newname(g);
        write_savecursor(g, p, savevar);

        generate(g, p->left);

        g->B[0] = str_data(varname);
        if (p->mode == m_forward) {
            w(g, "~{~Mint ~B0 = z->l - z->c; z->l = z->c;~N");
        } else {
            w(g, "~{~Mint ~B0 = z->lb; z->lb = z->c;~N");
        }
        write_restorecursor(g, p, savevar);

        if (p->mode == m_forward) {
            str_assign(g->failure_str, "z->l += ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        } else {
            str_assign(g->failure_str, "z->lb = ");
            str_append(g->failure_str, varname);
            str_append_ch(g->failure_str, ';');
        }
        str_delete(savevar);
    }

    generate(g, p->aux);

    write_margin(g);
    write_str(g, g->failure_str);
    w(g, "~N"
      "~}");
    if (extra_block) {
        write_block_end(g);
    }

    str_delete(varname);
}

/* dollar sets snowball up to operate on a string variable as if it were the
 * current string */
static void generate_dollar(struct generator * g, struct node * p) {
    write_comment(g, p);

    int used = g->label_used;
    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);
    int label = new_label(g);
    g->failure_label = label;
    g->label_used = 0;
    str_clear(g->failure_str);

    struct str * savevar = vars_newname(g);
    g->B[0] = str_data(savevar);

    // We only want to save and restore SN_env, not the variables.
    writef(g, "~{~Mstruct SN_env en~B0 = *z;~N", p);
    if (p->left->possible_signals == -1) {
        /* Assume failure. */
        w(g, "~Mint ~B0_f = 1;~N");
    }
    writef(g, "~Mz->p = ~V;~N"
              "~Mz->lb = z->c = 0;~N"
              "~Mz->l = SIZE(z->p);~N", p);

    generate(g, p->left);

    if (p->left->possible_signals == -1) {
        /* Mark success. */
        g->B[0] = str_data(savevar);
        w(g, "~M~B0_f = 0;~N");
    }

    if (g->label_used)
        wsetl(g, g->failure_label);

    g->label_used = used;
    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;

    g->B[0] = str_data(savevar);
    writef(g, "~M~V = z->p;~N"
              "~M*z = en~B0;~N", p);

    if (p->left->possible_signals == 0) {
        // p->left always signals f.
        w(g, "~M~f~N");
    } else if (p->left->possible_signals == -1) {
        w(g, "~Mif (~B0_f) ~f~N");
    }
    w(g, "~}");

    str_delete(savevar);
}

static void generate_integer_assign(struct generator * g, struct node * p, const char * s) {
    write_comment(g, p);
    g->S[0] = s;
    writef(g, "~M~V ~S0 ", p);
    generate_AE(g, p->AE);
    w(g, ";~N");
}

static void generate_integer_test(struct generator * g, struct node * p) {
    write_comment(g, p);
    int relop = p->type;
    int optimise_to_return = tailcallable(g, p);
    if (optimise_to_return) {
        w(g, "~Mreturn ");
        p->right = NULL;
    } else {
        w(g, "~Mif (");
        // We want the inverse of the snowball test here.
        relop ^= 1;
    }
    generate_AE(g, p->left);
    write_relop(g, relop);
    generate_AE(g, p->AE);
    if (optimise_to_return) {
        writef(g, ";~N", p);
    } else {
        writef(g, ") ~f~N", p);
    }
}

static void generate_call(struct generator * g, struct node * p) {
    int signals = p->name->definition->possible_signals;
    write_comment(g, p);
    if (tailcallable(g, p)) {
        /* Tail call. */
        writef(g, "~Mreturn ~V(z);~N", p);
        p->right = NULL;
        return;
    }
    if (just_return_on_fail(g) && signals == 0) {
        /* Always fails. */
        writef(g, "~Mreturn ~V(z);~N", p);
        return;
    }
    if (just_return_on_fail(g)) {
        write_block_start(g);
        writef(g, "~Mint ret = ~V(z);~N", p);
        if (g->options->target_lang == LANG_CPLUSPLUS ||
            !can_error(p->name)) {
            writef(g, "~Mif (ret == 0) return ret;~N", p);
        } else {
            /* For C, we need to propagate both failures and runtime errors so
             * we do a combined test for better optimisation and clearer
             * generated code. */
            writef(g, "~Mif (ret <= 0) return ret;~N", p);
        }
        write_block_end(g);
    } else {
        if (signals == 1) {
            /* Always succeeds - just need to handle runtime errors. */
            write_propagating_error(g, "~V(z)", false, p);
        } else if (signals == 0) {
            /* Always fails. */
            write_propagating_error(g, "~V(z)", false, p);
            writef(g, "~M~f~N", p);
        } else {
            if (g->options->target_lang == LANG_CPLUSPLUS ||
                !can_error(p->name)) {
                writef(g, "~Mif (!~V(z)) ~f~N", p);
            } else {
                write_block_start(g);
                writef(g, "~Mint ret = ~V(z);~N", p);
                writef(g, "~Mif (ret == 0) ~f~N", p);
                writef(g, "~Mif (ret < 0) return ret;~N", p);
                write_block_end(g);
            }
        }
    }
}

static void generate_grouping(struct generator * g, struct node * p, int complement) {
    write_comment(g, p);

    struct grouping * q = p->name->grouping;
    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->S[1] = complement ? "out" : "in";
    g->S[2] = g->options->encoding == ENC_UTF8 ? "_U" : "";
    g->I[0] = q->smallest_ch;
    g->I[1] = q->largest_ch;
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn !~S1_grouping~S0~S2(z, ~V, ~I0, ~I1, 0);~N", p);
        p->right = NULL;
    } else {
        writef(g, "~Mif (~S1_grouping~S0~S2(z, ~V, ~I0, ~I1, 0)) ~f~N", p);
    }
}

static void generate_namedstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->S[0] = p->mode == m_forward ? "" : "_b";
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn eq_v~S0(z, ~V);~N", p);
        p->right = NULL;
    } else {
        writef(g, "~Mif (!(eq_v~S0(z, ~V))) ~f~N", p);
    }
}

static void generate_literalstring(struct generator * g, struct node * p) {
    write_comment(g, p);
    symbol * b = p->literalstring;
    if (SIZE(b) == 1) {
        /* It's quite common to compare with a single character literal string,
         * so just inline the simpler code for this case rather than making a
         * function call.  In UTF-8 mode, only do this for the ASCII subset,
         * since multi-byte characters are more complex to test against.
         */
        if (g->options->encoding == ENC_UTF8 && *b >= 128) {
            printf("single byte %d\n", *b);
            exit(1);
        }
        g->I[0] = *b;
        if (p->mode == m_forward) {
            writef(g, "~Mif (z->c == z->l || z->p[z->c] != ~c0) ~f~N"
                  "~Mz->c++;~N", p);
        } else {
            writef(g, "~Mif (z->c <= z->lb || z->p[z->c - 1] != ~c0) ~f~N"
                  "~Mz->c--;~N", p);
        }
        return;
    }

    g->S[0] = p->mode == m_forward ? "" : "_b";
    if (tailcallable(g, p)) {
        writef(g, "~Mreturn eq_s~S0(z, ~s, ~L);~N", p);
        p->right = NULL;
    } else {
        writef(g, "~Mif (!(eq_s~S0(z, ~s, ~L))) ~f~N", p);
    }
}

static void generate_define(struct generator * g, struct node * p) {
    struct name * q = p->name;

    write_newline(g);
    write_comment(g, p);

    if (q->type == t_routine) {
        write_string(g, "static ");
    } else if (g->options->target_lang == LANG_C) {
        write_string(g, "extern ");
    }
    writef(g, "int ~V(struct SN_env * z) {~N~+", p);

    if (amongvar_needed(p->left)) {
        w(g, "~Mint among_var;~N");
    }

    /* Declare localised variables. */
    for (struct name * name = g->analyser->names; name; name = name->next) {
        if (name->local_to == q) {
            switch (name->type) {
                case t_string:
                    assert(0);
                    break;
                case t_integer:
                    w(g, "~Mint ");
                    write_varname(g, name);
                    w(g, ";~N");
                    break;
                case t_boolean:
                    if (g->options->target_lang == LANG_CPLUSPLUS) {
                        w(g, "~Mbool ");
                    } else {
                        w(g, "~Mint ");
                    }
                    write_varname(g, name);
                    w(g, ";~N");
                    break;
            }
        }
    }

    g->next_label = 0;
    g->var_number = 0;

    str_clear(g->failure_str);
    g->failure_label = x_return;
    g->label_used = 0;

    /* Generate function body. */
    generate(g, p->left);
    if (p->left->right) {
        assert(p->left->right->type == c_functionend);
        if (p->left->possible_signals) {
            generate(g, p->left->right);
        }
    }
    w(g, "~}");
}

static void generate_functionend(struct generator * g, struct node * p) {
    (void)p;
    w(g, "~Mreturn 1;~N");
}

static void generate_substring(struct generator * g, struct node * p) {
    write_comment(g, p);

    struct among * x = p->among;
    int block = -1;
    unsigned int bitmap = 0;
    struct amongvec * among_cases = x->v;
    int empty_case = -1;
    int n_cases = 0;
    symbol cases[2];
    int shortest_size = x->shortest_size;

    g->S[0] = p->mode == m_forward ? "" : "_b";
    g->I[0] = x->number;
    g->I[1] = x->literalstring_count;

    /* In forward mode with non-ASCII UTF-8 characters, the first byte
     * of the string will often be the same, so instead look at the last
     * common byte position.
     *
     * In backward mode, we can't match if there are fewer characters before
     * the current position than the minimum length.
     */
    for (int c = 0; c < x->literalstring_count; ++c) {
        symbol ch;
        if (among_cases[c].size == 0) {
            empty_case = c;
            continue;
        }
        if (p->mode == m_forward) {
            ch = among_cases[c].b[shortest_size - 1];
        } else {
            ch = among_cases[c].b[among_cases[c].size - 1];
        }
        if (n_cases == 0) {
            block = ch >> 5;
        } else if (ch >> 5 != block) {
            block = -1;
            if (n_cases > 2) break;
        }
        if (block == -1) {
            if (n_cases > 0 && ch == cases[0]) continue;
            if (n_cases < 2) {
                cases[n_cases++] = ch;
            } else if (ch != cases[1]) {
                ++n_cases;
                break;
            }
        } else {
            if ((bitmap & (1u << (ch & 0x1f))) == 0) {
                bitmap |= 1u << (ch & 0x1f);
                if (n_cases < 2)
                    cases[n_cases] = ch;
                ++n_cases;
            }
        }
    }

    bool pre_check = (block != -1 || n_cases <= 2);
    if (g->options->coverage) {
        // Don't shortcut if generating coverage.
        pre_check = false;
    }
    if (pre_check) {
        char buf[64];
        g->I[2] = block;
        g->I[3] = bitmap;
        g->I[4] = shortest_size - 1;
        if (p->mode == m_forward) {
            checked_snprintf(buf, sizeof(buf),
                             "z->p[z->c + %d]", shortest_size - 1);
            g->S[1] = buf;
            if (shortest_size == 1) {
                writef(g, "~Mif (z->c >= z->l", p);
            } else {
                writef(g, "~Mif (z->c + ~I4 >= z->l", p);
            }
        } else {
            g->S[1] = "z->p[z->c - 1]";
            if (shortest_size == 1) {
                writef(g, "~Mif (z->c <= z->lb", p);
            } else {
                writef(g, "~Mif (z->c - ~I4 <= z->lb", p);
            }
        }
        assert(n_cases > 0);
        if (n_cases == 1) {
            g->I[4] = cases[0];
            writef(g, " || ~S1 != ~I4", p);
        } else if (n_cases == 2) {
            g->I[4] = cases[0];
            g->I[5] = cases[1];
            writef(g, " || (~S1 != ~I4 && ~S1 != ~I5)", p);
        } else {
            writef(g, " || ~S1 >> 5 != ~I2 || !((~I3 >> (~S1 & 0x1f)) & 1)", p);
        }
        write_string(g, ") ");
        if (empty_case != -1 && !among_cases[empty_case].function) {
            /* If the among includes the ungated empty string, it can never
             * fail so not matching the bitmap means we match the empty string.
             */
            g->I[4] = among_cases[empty_case].result;
            writef(g, "among_var = ~I4; else~N", p);
        } else {
            writef(g, "~f~N", p);
        }
    } else {
#ifdef OPTIMISATION_WARNINGS
        printf("Couldn't shortcut among %d\n", x->number);
#endif
    }

    if (x->amongvar_needed) {
        writef(g, "~Mamong_var = find_among~S0(z, a_~I0, ~I1, ~F);~N", p);
        if (!x->always_matches) {
            writef(g, "~Mif (!among_var) ~f~N", p);
        }
        return;
    }

    if (pre_check && !x->function_count) {
        // If all cases are one symbol long (so one byte of UTF-8, one
        // character long in fixed-width encodings) then we don't need to call
        // the helper and can just inc/dec the cursor by 1.
        if (x->longest_size == 1 && !x->always_matches) {
            write_margin(g);
            winc(g, p);
            write_newline(g);
            // Suppress generating table for this among.
            x->used = false;
            return;
        }
    }

    if (x->always_matches) {
        writef(g, "~Mfind_among~S0(z, a_~I0, ~I1, ~F);~N", p);
    } else if (x->command_count == 0 && tailcallable(g, p)) {
        writef(g, "~Mreturn find_among~S0(z, a_~I0, ~I1, ~F) != 0;~N", p);
        x->node->right = NULL;
    } else {
        writef(g, "~Mif (!find_among~S0(z, a_~I0, ~I1, ~F)) ~f~N", p);
    }
}

static void generate_among(struct generator * g, struct node * p) {
    struct among * x = p->among;

    if (x->substring == NULL) {
        generate_substring(g, p);
    } else {
        write_comment(g, p);
    }

    if (x->command_count == 1 && x->nocommand_count == 0) {
        /* Only one outcome ("no match" already handled). */
        generate(g, x->commands[0]);
    } else if (x->command_count > 0) {
        writef(g, "~Mswitch (among_var) {~N~+", p);
        for (int i = 1; i <= x->command_count; i++) {
            g->I[0] = i;
            w(g, "~Mcase ~I0:~N~+");
            generate(g, x->commands[i - 1]);
            w(g, "~Mbreak;~N~-");
        }
        w(g, "~}");
    }
}

static void generate_booltest(struct generator * g, struct node * p, int inverted) {
    write_comment(g, p);
    if (tailcallable(g, p)) {
        // Optimise at end of function.
        if (inverted) {
            writef(g, "~Mreturn !~V;~N", p);
        } else {
            writef(g, "~Mreturn ~V;~N", p);
        }
        p->right = NULL;
        return;
    }
    if (inverted) {
        writef(g, "~Mif (~V) ~f~N", p);
    } else {
        writef(g, "~Mif (!~V) ~f~N", p);
    }
}

static void generate_false(struct generator * g, struct node * p) {
    write_comment(g, p);
    writef(g, "~M~f~N", p);
}

static void generate_debug(struct generator * g, struct node * p) {
    write_comment(g, p);
    g->I[0] = g->debug_count++;
    g->I[1] = p->line_number;
    writef(g, "~Mdebug(z, ~I0, ~I1);~N", p);
}

static void generate(struct generator * g, struct node * p) {
    int used = g->label_used;
    int a0 = g->failure_label;
    struct str * a1 = str_copy(g->failure_str);

    switch (p->type) {
        case c_define:        generate_define(g, p); break;
        case c_bra:           generate_bra(g, p); break;
        case c_and:           generate_and(g, p); break;
        case c_or:            generate_or(g, p); break;
        case c_backwards:     generate_backwards(g, p); break;
        case c_not:           generate_not(g, p); break;
        case c_set:           generate_set(g, p); break;
        case c_unset:         generate_unset(g, p); break;
        case c_try:           generate_try(g, p); break;
        case c_fail:          generate_fail(g, p); break;
        case c_reverse:
        case c_test:          generate_test(g, p); break;
        case c_do:            generate_do(g, p); break;
        case c_goto:          generate_GO(g, p, 1); break;
        case c_gopast:        generate_GO(g, p, 0); break;
        case c_goto_grouping: generate_GO_grouping(g, p, 1, 0); break;
        case c_gopast_grouping:
                              generate_GO_grouping(g, p, 0, 0); break;
        case c_goto_non:      generate_GO_grouping(g, p, 1, 1); break;
        case c_gopast_non:    generate_GO_grouping(g, p, 0, 1); break;
        case c_repeat:        generate_repeat(g, p); break;
        case c_loop:          generate_loop(g, p); break;
        case c_atleast:       generate_atleast(g, p); break;
        case c_tomark:        generate_tomark(g, p); break;
        case c_hop:           generate_hop(g, p); break;
        case c_next:          generate_next(g, p); break;
        case c_tolimit:       generate_tolimit(g, p); break;
        case c_leftslice:     generate_leftslice(g, p); break;
        case c_rightslice:    generate_rightslice(g, p); break;
        case c_assignto:      generate_assignto(g, p); break;
        case c_sliceto:       generate_sliceto(g, p); break;
        case c_stringassign:  generate_stringassign(g, p); break;
        case c_insert:
        case c_attach:        generate_insert(g, p, p->type); break;
        case c_slicefrom:     generate_slicefrom(g, p); break;
        case c_setlimit:      generate_setlimit(g, p); break;
        case c_dollar:        generate_dollar(g, p); break;
        case c_assign:        generate_integer_assign(g, p, "="); break;
        case c_plusassign:    generate_integer_assign(g, p, "+="); break;
        case c_minusassign:   generate_integer_assign(g, p, "-="); break;
        case c_multiplyassign:generate_integer_assign(g, p, "*="); break;
        case c_divideassign:  generate_integer_assign(g, p, "/="); break;
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
            generate_integer_test(g, p);
            break;
        case c_call:          generate_call(g, p); break;
        case c_grouping:      generate_grouping(g, p, false); break;
        case c_non:           generate_grouping(g, p, true); break;
        case c_name:          generate_namedstring(g, p); break;
        case c_literalstring: generate_literalstring(g, p); break;
        case c_among:         generate_among(g, p); break;
        case c_substring:     generate_substring(g, p); break;
        case c_booltest:      generate_booltest(g, p, false); break;
        case c_not_booltest:  generate_booltest(g, p, true); break;
        case c_false:         generate_false(g, p); break;
        case c_true:          break;
        case c_debug:         generate_debug(g, p); break;
        case c_functionend:   generate_functionend(g, p); break;
        default: fprintf(stderr, "%d encountered\n", p->type);
                 exit(1);
    }

    if (g->failure_label != a0)
        g->label_used = used;

    g->failure_label = a0;
    str_delete(g->failure_str);
    g->failure_str = a1;
}

static void generate_head(struct generator * g) {
    struct options * o = g->options;
    if (o->cheader) {
        int quoted = (o->cheader[0] == '<' || o->cheader[0] == '"');
        w(g, "#include ");
        if (!quoted) write_char(g, '<');
        write_string(g, o->cheader);
        if (!quoted) write_char(g, '>');
        write_newline(g);
        write_newline(g);
    }

    if (o->target_lang == LANG_CPLUSPLUS) {
        w(g, "#define SNOWBALL_RUNTIME_THROW_EXCEPTIONS~N");
    }
    if (g->analyser->debug_used) {
        w(g, "#define SNOWBALL_DEBUG_COMMAND_USED~N");
    }

    w(g, "#include \"");
    write_s(g, o->output_leaf);
    w(g, ".h\"~N~N");

    if (g->analyser->int_limits_used) {
        w(g, "#include <limits.h>~N");
    }
    w(g, "#include <stddef.h>~N~N");

    if (o->target_lang == LANG_CPLUSPLUS) {
        w(g, "~Mtypedef ");
        write_string(g, o->package);
        w(g, "::~n::SN_local SN_local;~N~N");
        return;
    }

    w(g, "#include \"");
    if (o->runtime_path) {
        write_string(g, o->runtime_path);
        if (o->runtime_path[strlen(o->runtime_path) - 1] != '/')
            write_char(g, '/');
    }

    w(g, "snowball_runtime.h\"~N~N");

    if (g->analyser->variable_count > 0) {
        // Generate the struct SN_local definition, which embeds a struct
        // SN_env and also holds non-localised variables.  We group variables
        // by type to try to produce more efficient struct packing.
        w(g, "struct SN_local {~N~+"
             "~Mstruct SN_env z;~N");

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_integer) {
                w(g, "~Mint ");
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_boolean) {
                if (o->target_lang == LANG_CPLUSPLUS) {
                    w(g, "~Mbool ");
                } else {
                    w(g, "~Munsigned char ");
                }
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_string) {
                w(g, "~Msymbol * ");
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        w(g, "~-~M};~N~N");

        if (o->target_lang == LANG_C) {
            w(g, "typedef struct SN_local SN_local;~N~N");
        }
    }

    const char * vp = o->variables_prefix;
    if (vp) {
        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (q->local_to) continue;
            switch (q->type) {
                case t_string:
                    w(g, "extern const symbol * ");
                    write_string(g, vp);
                    write_s(g, q->s);
                    w(g, "(struct SN_env * z) {~N~+");
                    w(g, "~Msymbol * p = ");
                    write_varref(g, q);
                    w(g, ";~N"
                         "~Mp[SIZE(p)] = 0;~N"
                         "~Mreturn p;~N~-"
                         "}~N~N");
                    break;
                case t_integer:
                    w(g, "extern int ");
                    write_string(g, vp);
                    write_s(g, q->s);
                    w(g, "(struct SN_env * z) {~N~+"
                         "~Mreturn ");
                    write_varref(g, q);
                    w(g, ";~N~-"
                         "}~N~N");
                    break;
                case t_boolean:
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "extern bool ");
                    } else {
                        w(g, "extern int ");
                    }
                    write_string(g, vp);
                    write_s(g, q->s);
                    w(g, "(struct SN_env * z) {~N~+"
                         "~Mreturn ");
                    write_varref(g, q);
                    w(g, ";~N~-"
                         "}~N~N");
                    break;
            }
        }
    }
}

static void generate_routine_declarations(struct generator * g) {
    if (g->options->target_lang == LANG_C) {
        w(g, "#ifdef __cplusplus~N"
             "extern \"C\" {~N"
             "#endif~N");
        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (q->type == t_external) {
                w(g, "extern int ");
                write_varname(g, q);
                w(g, "(struct SN_env * z);~N");
            }
        }
        w(g, "#ifdef __cplusplus~N"
             "}~N"
             "#endif~N~N");
    }

    if (g->analyser->name_count[t_routine]) {
        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (q->type == t_routine) {
                w(g, "static int ");
                write_varname(g, q);
                w(g, "(struct SN_env * z);~N");
            }
        }
        write_newline(g);
    }
}

static void generate_among_table(struct generator * g, struct among * x) {
    write_newline(g);
    write_comment(g, x->node);

    struct amongvec * v = x->v;

    g->I[0] = x->number;
    for (int i = 0; i < x->literalstring_count; i++) {
        if (v[i].size) {
            g->I[1] = i;
            g->I[2] = v[i].size;
            w(g, "static const symbol s_~I0_~I1[~I2] = ");
            wlitarray(g, v[i].b);
            w(g, ";~N");
        }
    }

    g->I[1] = x->literalstring_count;
    if (g->options->coverage) {
        g->I[1] = g->I[1] * 2 + 1;
    }
    w(g, "~Mstatic const struct among a_~I0[~I1] = {~N");

    for (int i = 0; i < x->literalstring_count; i++) {
        if (i) w(g, ",~N");
        g->I[1] = i;
        g->I[2] = v[i].size;
        g->I[3] = (v[i].i >= 0 ? v[i].i - i : 0);
        g->I[4] = v[i].result;
        g->I[5] = v[i].function_index;

        if (g->options->comments) {
            w(g, "/*~J1 */ ");
        }
        w(g, "{ ~I2, ");
        if (v[i].size == 0) {
            w(g, "0,");
        } else {
            w(g, "s_~I0_~I1,");
        }
        w(g, " ~I3, ~I4, ~I5}");
    }
    if (g->options->coverage) {
        w(g, ",~N");
        g->S[1] = g->analyser->tokeniser->file;
        for (int i = 0; i < x->literalstring_count; i++) {
            if (g->options->comments) {
                w(g, "/* coverage */ ");
            }
            g->I[1] = x->v[i].line_number;
            g->I[2] = x->v[i].string_index;
            w(g, "{ ~I0, (const symbol*)\"~S1:~I1\", 0, ~I2, 0 },~N");
        }
        if (x->always_matches) {
            g->I[0] = -1;
        }
        if (g->options->comments) {
            w(g, "/* coverage */ ");
        }
        g->I[1] = x->node->line_number;
        w(g, "{ ~I0, (const symbol*)\"~S1:~I1\", 0, 0, 0 },~N");
    }
    w(g, "~N};~N");

    if (x->unique_function_count <= 1) return;

    w(g, "~N~Mstatic int af_~I0(struct SN_env * z) {~N~+");
    w(g, "~Mswitch (z->af) {~N~+");
    for (int n = 1; n <= x->unique_function_count; n++) {
        w(g, "~Mcase ");
        write_int(g, n);
        w(g, ": return ");
        for (int i = 0; i < x->literalstring_count; i++) {
            if (v[i].function_index == n) {
                write_varref(g, v[i].function);
                w(g, "(z);~N");
                break;
            }
        }
    }
    w(g, "~-~M}~N");
    w(g, "~Mreturn -1;~N");
    w(g, "~-~M}~N");
}

static void generate_amongs(struct generator * g) {
    struct str * s = g->outbuf;
    g->outbuf = g->declarations;
    for (struct among * x = g->analyser->amongs; x; x = x->next) {
        if (x->used) generate_among_table(g, x);
    }
    g->outbuf = s;
}

static void set_bit(symbol * b, int i) { b[i >> 3] |= 1 << (i & 7); }

static void generate_grouping_table(struct generator * g, struct grouping * q) {
    int range = q->largest_ch - q->smallest_ch + 1;
    int size = (range + 7) / 8;  /* assume 8 bits per symbol */
    symbol * b = q->b;
    symbol * map = create_b(size);

    for (int i = 0; i < size; i++) map[i] = 0;

    for (int i = 0; i < SIZE(b); i++) set_bit(map, b[i] - q->smallest_ch);

    w(g, "~Nstatic const unsigned char ");
    write_varname(g, q->name);
    w(g, "[] = { ");
    for (int i = 0; i < size; i++) {
        if (i) w(g, ", ");
        write_int(g, map[i]);
    }
    if (g->options->coverage) {
        int grouping_number = q->name->count;
        if (grouping_number > 255) grouping_number = 255;
        w(g, ", ");
        wlitch(g, grouping_number);

        char buf[1024];
        checked_snprintf(buf, sizeof(buf), "%s:%d: grouping %.*s",
                         g->analyser->tokeniser->file, q->line_number,
                         SIZE(q->name->s), q->name->s);

        for (const char * p = buf; *p; ++p) {
            w(g, ", ");
            wlitch(g, (int)*p);
        }
        w(g, ", '\\0'");
    }
    w(g, " };~N");

    lose_b(map);
}

static void generate_groupings(struct generator * g) {
    struct str * s = g->outbuf;
    g->outbuf = g->declarations;
    for (struct grouping * q = g->analyser->groupings; q; q = q->next) {
        generate_grouping_table(g, q);
    }
    g->outbuf = s;
}

static void generate_create(struct generator * g) {
    w(g, "~N"
         "extern struct SN_env * ~pcreate_env(void) {~N~+");

    if (g->analyser->variable_count == 0) {
        w(g, "~Mreturn SN_new_env(sizeof(struct SN_env));~N");
    } else if (g->analyser->name_count[t_string] == 0) {
        w(g, "~Mreturn SN_new_env(sizeof(SN_local));~N");
    } else {
        w(g, "~Mstruct SN_env * z = SN_new_env(sizeof(SN_local));~N"
             "~Mif (z) {~N~+");

        // SN_new_env() initialises the allocated size to all-zero-bits, so
        // assigning NULL here is only needed on platforms where NULL doesn't
        // have an all-zero-bits representation in memory.  The C standard
        // allows that, but there don't seem to be any current such platforms.
        // There doesn't seem an easy way to only enable this code when it is
        // useful though.
        //
        // To simplify handling a failure to allocate a string variable, if
        // there are multiple non-localised string variables we initialise them
        // all to NULL first, then try to allocate them in a second pass.  We
        // don't need to do this when there's only one because in that case we
        // can't have a partially successful allocation.
        if (g->analyser->name_count[t_string] > 1) {
            for (struct name * name = g->analyser->names; name; name = name->next) {
                if (!name->local_to && name->type == t_string) {
                    w(g, "~M");
                    write_varref(g, name);
                    w(g, " = NULL;~N");
                }
            }
            write_newline(g);
        }

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to) {
                switch (name->type) {
                    case t_string:
                        w(g, "~Mif ((");
                        write_varref(g, name);
                        w(g, " = create_s()) == NULL) {~N~+"
                             "~M~pclose_env(z);~N"
                             "~Mreturn NULL;~N~-"
                             "~M}~N");
                        break;
                }
            }
        }

        w(g, "~-~M}~N"
             "~Mreturn z;~N");
    }

    w(g, "~-}~N");
}

static void generate_close(struct generator * g) {
    // If there are no string variables then our close_env is just
    // SN_delete_env so we #define it to that in the header.
    if (g->analyser->name_count[t_string] == 0) return;

    w(g, "~Nextern void ~pclose_env(struct SN_env * z) {~N~+");
    w(g, "~Mif (!z) return;~N");

    for (struct name * name = g->analyser->names; name; name = name->next) {
        if (!name->local_to && name->type == t_string) {
            w(g, "~Mlose_s(");
            write_varref(g, name);
            w(g, ");~N");
        }
    }
    w(g, "~MSN_delete_env(z);~N"
         "~-}~N~N");
}

static void generate_header_file(struct generator * g) {
    struct options * o = g->options;
    if (o->hheader) {
        int quoted = (o->hheader[0] == '<' || o->hheader[0] == '"');
        w(g, "#include ");
        if (!quoted) write_char(g, '<');
        write_string(g, o->hheader);
        if (!quoted) write_char(g, '>');
        write_newline(g);
        write_newline(g);
    }

    if (o->target_lang == LANG_CPLUSPLUS) {
        w(g, "#define SNOWBALL_RUNTIME_THROW_EXCEPTIONS~N"
             "#include \"");
        if (o->runtime_path) {
            write_string(g, o->runtime_path);
            if (o->runtime_path[strlen(o->runtime_path) - 1] != '/')
                write_char(g, '/');
        }
        w(g, "snowball_runtime.h\"~N~N");

        w(g, "namespace ");
        write_string(g, o->package);
        w(g, " {~N~N");

        w(g, "class ~n : public ");
        write_string(g, o->parent_class_name);
        w(g, " {~N"
             "  public:~N~+");
    }

    if (o->target_lang == LANG_C) {
        w(g, "#ifdef __cplusplus~N"
             "extern \"C\" {~N"
             "#endif~N");            /* for C++ */

        w(g, "~N"
             "extern struct SN_env * ~pcreate_env(void);~N");

        if (g->analyser->name_count[t_string] == 0) {
            w(g, "#define ~pclose_env SN_delete_env~N");
        } else {
            w(g, "extern void ~pclose_env(struct SN_env * z);~N");
        }
        write_newline(g);
    }

    const char * vp = o->variables_prefix;
    if (vp) {
        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (q->local_to) continue;
            switch (q->type) {
                case t_string:
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "~Mconst symbol * ");
                    } else {
                        w(g, "extern const symbol * ");
                    }
                    write_string(g, vp);
                    write_s(g, q->s);
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "() {~N~+"
                             "~Mstruct SN_env * z = &(zlocal.z);~N"
                             "~Msymbol * p = ");
                        write_varref(g, q);
                        w(g, ";~N"
                             "~Mp[SIZE(p)] = 0;~N"
                             "~Mreturn p;~N~-"
                             "~M}~N~N");
                    } else {
                        w(g, "(struct SN_env * z);~N");
                    }
                    break;
                case t_integer:
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "~Mint ");
                    } else {
                        w(g, "extern int ");
                    }
                    write_string(g, vp);
                    write_s(g, q->s);
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "() {~N~+"
                             "~Mstruct SN_env * z = &(zlocal.z);~N"
                             "~Mreturn ");
                        write_varref(g, q);
                        w(g, ";~N~-"
                             "~M}~N~N");
                    } else {
                        w(g, "(struct SN_env * z);~N");
                    }
                    break;
                case t_boolean:
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "~Mbool ");
                    } else {
                        w(g, "extern int ");
                    }
                    write_string(g, vp);
                    write_s(g, q->s);
                    if (o->target_lang == LANG_CPLUSPLUS) {
                        w(g, "() {~N~+"
                             "~Mstruct SN_env * z = &(zlocal.z);~N"
                             "~Mreturn ");
                        write_varref(g, q);
                        w(g, ";~N~-"
                             "~M}~N~N");
                    } else {
                        w(g, "(struct SN_env * z);~N");
                    }
                    break;
            }
        }
    }

    if (o->target_lang == LANG_C) {
        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (q->type == t_external) {
                w(g, "extern int ");
                write_varname(g, q);
                w(g, "(struct SN_env * z);~N");
            }
        }

        w(g, "~N"
             "#ifdef __cplusplus~N"
             "}~N"
             "#endif~N");            /* for C++ */
    }

    if (o->target_lang == LANG_CPLUSPLUS) {
        // Generate the struct SN_local definition, which embeds a struct
        // SN_env and also holds any non-localised variables.  We group
        // variables by type to try to produce more efficient struct packing.
        w(g, "~Mstruct SN_local {~N~+"
             "~Mstruct SN_env z;~N");

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_integer) {
                w(g, "~Mint ");
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_boolean) {
                if (o->target_lang == LANG_CPLUSPLUS) {
                    w(g, "~Mbool ");
                } else {
                    w(g, "~Munsigned char ");
                }
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        for (struct name * name = g->analyser->names; name; name = name->next) {
            if (!name->local_to && name->type == t_string) {
                w(g, "~Msymbol * ");
                write_varname(g, name);
                w(g, ";~N");
            }
        }

        w(g, "~-~M};~N"
             "~N");

        w(g, "~-  private:~N~+"
             "~MSN_local zlocal = {};~N"
             "~N"
             "~Mvoid close_env() {~N~+"
             "~Mstruct SN_env * z = &(zlocal.z);~N");
        if (g->analyser->name_count[t_string] > 0) {
            for (struct name * name = g->analyser->names; name; name = name->next) {
                if (!name->local_to && name->type == t_string) {
                    w(g, "~Mlose_s(");
                    write_varref(g, name);
                    w(g, ");~N");
                }
            }
        }
        w(g, "~Mlose_s(z->p);~N"
             "~-~M}~N~N");

        for (struct name * q = g->analyser->names; q; q = q->next) {
            if (!q->local_to && q->type == t_external) {
                w(g, "~Mstatic int ");
                if (o->externals_prefix) {
                    write_string(g, o->externals_prefix);
                }
                write_s(g, q->s);
                w(g, "(struct SN_env * z);~N~N");
            }
        }

        w(g, "~-  public:~N~+"
             "~M~n() {~N~+"
             "~Mstruct SN_env * z = &(zlocal.z);~N"
             "~Mz->p = create_s();~N");
        if (g->analyser->name_count[t_string] > 0) {
            w(g, "~Mtry {~N~+");
            for (struct name * name = g->analyser->names; name; name = name->next) {
                if (!name->local_to && name->type == t_string) {
                    write_margin(g);
                    write_varref(g, name);
                    w(g, " = create_s();~N");
                }
            }
            w(g, "~-~M} catch (...) {~N~+"
                 "~Mclose_env();~N"
                 "~Mthrow;~N"
                 "~-~M}~N");
        }
        if (o->encoding == ENC_WIDECHARS) {
            g->S[0] = "std::wstring";
            g->S[1] = "wchar_t";
        } else {
            g->S[0] = "std::string";
            g->S[1] = "char";
        }
        w(g, "~-~M}~N~N"
             "~M~~~n() {~N~+"
             "~Mclose_env();~N"
             "~-~M}~N~N"
             "~M~S0 operator()(const ~S0& word) override {~N~+"
             "~Mstruct SN_env* z = &(zlocal.z);~N"
             "~Mconst symbol* s = reinterpret_cast<const symbol*>(word.data());~N"
             "~Mint s_size = word.size() > INT_MAX ? INT_MAX : word.size();~N"
             "~Mreplace_s(z, 0, z->l, s_size, s);~N"
             "~Mz->c = 0;~N"
             "~M");
        write_string(g, o->package);
        write_string(g, "::");
        write_s(g, o->name);
        write_string(g, "::");
        if (o->externals_prefix) {
            write_string(g, o->externals_prefix);
        }
        w(g, "stem(z);~N"
             "~Mreturn ~S0(reinterpret_cast<const ~S1*>(z->p), SIZE(z->p));~N"
             "~-~M}~N"
             "~N");

        // FIXME: Hard-coding this information here isn't great.
        int use_proper_noun_heuristic = false;
        switch (SIZE(o->output_leaf)) {
          case 5:
            if (memcmp(o->output_leaf, "dutch", 5) == 0)
                use_proper_noun_heuristic = true;
            break;
          case 6:
            if (memcmp(o->output_leaf, "danish", 6) == 0 ||
                memcmp(o->output_leaf, "french", 6) == 0 ||
                memcmp(o->output_leaf, "lovins", 6) == 0 ||
                memcmp(o->output_leaf, "porter", 6) == 0)
                use_proper_noun_heuristic = true;
            break;
          case 7:
            if (memcmp(o->output_leaf, "catalan", 7) == 0 ||
                memcmp(o->output_leaf, "english", 7) == 0 ||
                memcmp(o->output_leaf, "italian", 7) == 0 ||
                memcmp(o->output_leaf, "spanish", 7) == 0 ||
                memcmp(o->output_leaf, "swedish", 7) == 0)
                use_proper_noun_heuristic = true;
            break;
          case 9:
            if (memcmp(o->output_leaf, "norwegian", 9) == 0)
                use_proper_noun_heuristic = true;
            break;
          case 10:
            if (memcmp(o->output_leaf, "indonesian", 10) == 0 ||
                memcmp(o->output_leaf, "portuguese", 10) == 0)
                use_proper_noun_heuristic = true;
            break;
          case 12:
            if (memcmp(o->output_leaf, "dutch_porter", 12) == 0 ||
                memcmp(o->output_leaf, "earlyenglish", 12) == 0)
                use_proper_noun_heuristic = true;
            break;
        }
        if (use_proper_noun_heuristic) {
            w(g, "~Mbool use_proper_noun_heuristic() const override {~N~+"
                 "~Mreturn true;~N"
                 "~-~M}~N"
                 "~N");
        }

        w(g, "~Mstd::string get_description() const override {~N~+"
             "~Mreturn \"");
        for (int i = 0; i < SIZE(o->output_leaf); i++) {
            byte ch = o->output_leaf[i];
            if (ch == '\"' || ch == '\\') {
                write_char(g, '\\');
            }
            write_char(g, ch);
        }
        w(g, "\";~N"
             "~-~M}~N"
             "~-~M};~N~N");

        w(g, "}~N");
    }
}

extern void generate_program_c(struct generator * g) {
    g->outbuf = str_new();
    g->failure_str = str_new();
    write_start_comment(g, "/* ", " */");
    generate_head(g);
    generate_routine_declarations(g);
    g->declarations = g->outbuf;
    g->outbuf = str_new();
    g->literalstring_count = 0;

    for (struct node * p = g->analyser->program; p; p = p->right) {
        generate(g, p);
    }

    generate_amongs(g);
    generate_groupings(g);

    if (g->options->target_lang != LANG_CPLUSPLUS) {
        generate_create(g);
        generate_close(g);
    }

    output_str(g->options->output_src, g->declarations);
    str_delete(g->declarations);
    output_str(g->options->output_src, g->outbuf);
    str_clear(g->outbuf);

    write_start_comment(g, "/* ", " */");
    generate_header_file(g);
    output_str(g->options->output_h, g->outbuf);
    str_delete(g->outbuf);
    str_delete(g->failure_str);
}
