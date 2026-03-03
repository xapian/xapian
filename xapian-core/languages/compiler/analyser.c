#include <assert.h>
#include <limits.h>  /* for INT_MAX */
#include <stdio.h>   /* printf etc */
#include <stdlib.h>  /* exit */
#include <string.h>  /* memmove */
#include "header.h"

/* recursive usage: */

static void read_program_(struct analyser * a, int terminator);
static struct node * read_C(struct analyser * a);
static struct node * new_string_command(struct analyser * a, int token);

static void print_node_(const struct node * p, int n, const char * s) {
    printf("%*s%s", n * 2, s, name_of_token(p->type));
    if (p->name) {
        putchar(' ');
        report_s(stdout, p->name->s);
    }
    if (p->literalstring) {
        printf(" '");
        report_b(stdout, p->literalstring);
        printf("'");
    } else if (p->type == c_number) {
        printf(" %d", p->number);
    }
    printf("\n");
    if (p->AE) print_node_(p->AE, n+1, "# ");
    if (p->left) print_node_(p->left, n+1, "");
    if (p->aux) print_node_(p->aux, n+1, "@ ");
    if (p->right) print_node_(p->right, n, "");
}

extern void print_program(struct analyser * a) {
    if (a->program) print_node_(a->program, 0, "");
}

static struct node * new_node(struct analyser * a, int type) {
    NEW(node, p);
    *p = (struct node){0};
    p->mode = a->mode;
    p->line_number = a->tokeniser->line_number;
    p->type = type;
    p->next = a->nodes;
    a->nodes = p;
    return p;
}

static const char * name_of_mode(int n) {
    switch (n) {
        case m_backward: return "string backward";
        case m_forward:  return "string forward";
    }
    fprintf(stderr, "Invalid mode %d in name_of_mode()\n", n);
    exit(1);
}

static const char * name_of_type(int code) {
    switch (code) {
        case t_string: return "string";
        case t_boolean: return "boolean";
        case t_integer: return "integer";
        case t_routine: return "routine";
        case t_external: return "external";
        case t_grouping: return "grouping";
    }
    fprintf(stderr, "Invalid type code %d in name_of_type()\n", code);
    exit(1);
}

static void count_error(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    if (t->error_count >= 20) { fprintf(stderr, "... etc\n"); exit(1); }
    t->error_count++;
}

static void report_error_location(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    count_error(a);
    fprintf(stderr, "%s:%d: ", t->file, t->line_number);
}

static void report_error_after(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    if (t->previous_token > 0)
        fprintf(stderr, " after %s", name_of_token(t->previous_token));
}

static void omission_error(struct analyser * a, int n) {
    report_error_location(a);
    fprintf(stderr, "%s omitted", name_of_token(n));
    report_error_after(a);
    putc('\n', stderr);
}

static void unexpected_token_error(struct analyser * a,
                                   const char * context) {
    struct tokeniser * t = a->tokeniser;
    if (t->token_reported_as_unexpected) {
        // Avoid duplicate errors if this token was already reported as
        // unexpected and then held.
        return;
    }
    report_error_location(a);
    t->token_reported_as_unexpected = true;
    fprintf(stderr, "unexpected %s", name_of_token(t->token));
    if (t->token == c_number) fprintf(stderr, " %d", t->number);
    if (t->token == c_name) {
        fprintf(stderr, " %.*s", SIZE(t->s), t->s);
    }
    if (context) {
        fprintf(stderr, " in %s", context);
    }
    report_error_after(a);
    putc('\n', stderr);
}

static void substring_without_among_error(struct analyser * a) {
    count_error(a);
    fprintf(stderr, "%s:%d: 'substring' with no matching 'among'\n",
            a->tokeniser->file, a->substring->line_number);
}

static int check_token(struct analyser * a, int code) {
    struct tokeniser * t = a->tokeniser;
    if (t->token != code) { omission_error(a, code); return false; }
    return true;
}

static void hold_token_if_toplevel(struct tokeniser * t) {
    // Hold token if it starts a top-level construct.
    switch (t->token) {
        case c_backwardmode:
        case c_booleans:
        case c_define:
        case c_externals:
        case c_groupings:
        case c_integers:
        case c_routines:
        case c_strings:
            hold_token(t);
    }
}

static int get_token(struct analyser * a, int code) {
    struct tokeniser * t = a->tokeniser;
    read_token(t);
    int x = check_token(a, code);
    if (!x) hold_token(t);
    return x;
}

static struct name * look_for_name(struct analyser * a) {
    const byte * q = a->tokeniser->s;
    for (struct name * p = a->names; p; p = p->next) {
        byte * b = p->s;
        int n = SIZE(b);
        if (n == SIZE(q) && memcmp(q, b, n) == 0) {
            ++p->references;
            return p;
        }
    }
    return NULL;
}

static struct name * find_name(struct analyser * a) {
    struct name * p = look_for_name(a);
    if (p == NULL) {
        report_error_location(a);
        byte * s = a->tokeniser->s;
        fprintf(stderr, "'%.*s' undeclared\n", SIZE(s), s);
    }
    return p;
}

static void check_routine_mode(struct analyser * a, struct name * p, int mode) {
    if (p->mode == m_unknown) {
        p->mode = mode;
    } else if (p->mode != mode) {
        report_error_location(a);
        fprintf(stderr, "%s '%.*s' mis-used in %s mode\n",
                name_of_type(p->type),
                SIZE(p->s), p->s,
                name_of_mode(mode));
    }
}

static void check_name_type(struct analyser * a, struct name * p, int type) {
    if (p->type == type) return;
    if (type == t_routine && p->type == t_external) return;
    report_error_location(a);
    fprintf(stderr, "'%.*s' not of type %s\n",
            SIZE(p->s), p->s,
            name_of_type(type));
}

static void read_names(struct analyser * a, int type) {
    struct tokeniser * t = a->tokeniser;
    if (!get_token(a, c_bra)) return;
    while (true) {
        int token = read_token(t);
        switch (token) {
            case c_len: {
                /* Context-sensitive token - once declared as a name, it loses
                 * its special meaning, for compatibility with older versions
                 * of snowball.
                 */
                SET_SIZE(t->s, 0);
                t->s = add_literal_to_s(t->s, "len");
                goto handle_as_name;
            }
            case c_lenof: {
                /* Context-sensitive token - once declared as a name, it loses
                 * its special meaning, for compatibility with older versions
                 * of snowball.
                 */
                SET_SIZE(t->s, 0);
                t->s = add_literal_to_s(t->s, "lenof");
                goto handle_as_name;
            }
            case c_name:
handle_as_name:
                if (look_for_name(a) != NULL) {
                    report_error_location(a);
                    fprintf(stderr, "'%.*s' re-declared\n", SIZE(t->s), t->s);
                } else {
                    NEW(name, p);
                    *p = (struct name){0};
                    p->mode = m_unknown; /* used for routines, externals */
                    p->s = copy_s(t->s);
                    p->type = type;
                    /* Delay assigning counts until after we've eliminated
                     * variables whose values are never used and checked for
                     * variables which can be localised.
                     */
                    p->count = -1;
                    p->declaration_line_number = t->line_number;
                    p->next = a->names;
                    a->names = p;
                    if (token != c_name) {
                        disable_token(t, token);
                    }
                }
                break;
            default:
                if (!check_token(a, c_ket)) hold_token(t);
                return;
        }
    }
}

static symbol * new_literalstring(struct analyser * a) {
    NEW(literalstring, p);
    p->b = copy_b(a->tokeniser->b);
    p->next = a->literalstrings;
    a->literalstrings = p;
    return p->b;
}

static int read_AE_test(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    switch (read_token(t)) {
        case c_assign: return c_mathassign;
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le: return t->token;
        default:
            unexpected_token_error(a, "integer test expression");
            hold_token(t);
            return c_eq;
    }
}

static int binding(int t) {
    switch (t) {
        case c_plus: case c_minus: return 1;
        case c_multiply: case c_divide: return 2;
        default: return -2;
    }
}

static void mark_used_in(struct analyser * a, struct name * q, struct node * p) {
    if (!q->used) {
        q->used = p;
        q->local_to = a->program_end->name;
    } else if (q->local_to) {
        if (q->local_to != a->program_end->name) {
            /* Used in more than one routine/external. */
            q->local_to = NULL;
        }
    }
}

static void name_to_node(struct analyser * a, struct node * p, int type) {
    struct name * q = find_name(a);
    if (q) {
        check_name_type(a, q, type);
        mark_used_in(a, q, p);
    }
    p->name = q;
}

static struct node * read_AE(struct analyser * a, struct name * assigned_to, int B) {
    struct tokeniser * t = a->tokeniser;
    struct node * p;
    struct node * q;
    switch (read_token(t)) {
        case c_minus: /* monadic */
            q = read_AE(a, assigned_to, 100);
            if (q->type == c_neg) {
                /* Optimise away double negation, which avoids generators
                 * having to worry about generating "--" (decrement operator
                 * in many languages).
                 */
                p = q->right;
                /* Don't free q, it's in the linked list a->nodes. */
                break;
            }
            if (q->type == c_number) {
                /* Negated constant. */
                q->number = -q->number;
                p = q;
                break;
            }
            p = new_node(a, c_neg);
            p->right = q;
            break;
        case c_bra:
            p = read_AE(a, assigned_to, 0);
            get_token(a, c_ket);
            break;
        case c_name:
            p = new_node(a, c_name);
            name_to_node(a, p, t_integer);
            if (p->name) {
                // $x = x + 1 shouldn't count as a use of x.
                p->name->value_used = (p->name != assigned_to);
            }
            break;
        case c_maxint:
        case c_minint:
            a->int_limits_used = true;
            /* fall through */
        case c_cursor:
        case c_limit:
        case c_len:
        case c_size:
            p = new_node(a, t->token);
            break;
        case c_number:
            p = new_node(a, c_number);
            p->number = t->number;
            p->fixed_constant = true;
            break;
        case c_lenof:
        case c_sizeof: {
            int token = t->token;
            p = new_string_command(a, token);
            if (!p->literalstring) {
                if (p->name) p->name->value_used = true;
                break;
            }

            /* Replace lenof or sizeof on a literal string with a numeric
             * constant.
             */
            int result = 0;
            if (token == c_lenof && t->encoding == ENC_UTF8) {
                // UTF-8.
                symbol * b = p->literalstring;
                int dummy;
                for (int i = 0; i < SIZE(b); i += get_utf8(b + i, &dummy)) {
                    ++result;
                }
            } else {
                result = SIZE(p->literalstring);
            }
            p->type = c_number;
            p->literalstring = NULL;
            p->number = result;
            p->fixed_constant = (token == c_lenof);
            break;
        }
        default:
            unexpected_token_error(a, "integer expression");
            hold_token(t);
            return NULL;
    }
    while (true) {
        int token = read_token(t);
        int b = binding(token);
        if (binding(token) <= B) {
            hold_token(t);
            return p;
        }
        struct node * r = read_AE(a, assigned_to, b);
        if (p->type == c_number && r->type == c_number) {
            // Evaluate constant sub-expression.
            q = new_node(a, c_number);
            switch (token) {
                case c_plus:
                    q->number = p->number + r->number;
                    break;
                case c_minus:
                    q->number = p->number - r->number;
                    break;
                case c_multiply:
                    q->number = p->number * r->number;
                    break;
                case c_divide:
                    if (r->number == 0) {
                        fprintf(stderr, "%s:%d: Division by zero\n",
                                t->file, t->line_number);
                        exit(1);
                    }
                    q->number = p->number / r->number;
                    break;
                default:
                    fprintf(stderr, "Unexpected AE operator %s\n",
                            name_of_token(token));
                    exit(1);
            }
            q->fixed_constant = p->fixed_constant && r->fixed_constant;
            q->line_number = p->line_number;
        } else {
            // Check for specific constant or no-op cases.
            q = NULL;
            switch (token) {
                case c_plus:
                    // 0 + r is r
                    if (p->type == c_number && p->number == 0) {
                        q = r;
                        break;
                    }
                    // p + 0 is p
                    if (r->type == c_number && r->number == 0) {
                        q = p;
                        break;
                    }
                    break;
                case c_minus:
                    // 0 - r is -r
                    if (p->type == c_number && p->number == 0) {
                        q = new_node(a, c_neg);
                        q->right = r;
                        break;
                    }
                    // p - 0 is p
                    if (r->type == c_number && r->number == 0) {
                        q = p;
                        break;
                    }
                    break;
                case c_multiply:
                    // 0 * r is 0
                    if (p->type == c_number && p->number == 0) {
                        q = p;
                        break;
                    }
                    // p * 0 is 0
                    if (r->type == c_number && r->number == 0) {
                        q = r;
                        q->line_number = p->line_number;
                        break;
                    }
                    // -1 * r is -r
                    if (p->type == c_number && p->number == -1) {
                        q = new_node(a, c_neg);
                        q->right = r;
                        q->line_number = p->line_number;
                        break;
                    }
                    // p * -1 is -p
                    if (r->type == c_number && r->number == -1) {
                        q = new_node(a, c_neg);
                        q->right = p;
                        q->line_number = p->line_number;
                        break;
                    }
                    // 1 * r is r
                    if (p->type == c_number && p->number == 1) {
                        q = r;
                        q->line_number = p->line_number;
                        break;
                    }
                    // p * 1 is p
                    if (r->type == c_number && r->number == 1) {
                        q = p;
                        break;
                    }
                    break;
                case c_divide:
                    // p / 1 is p
                    if (r->type == c_number && r->number == 1) {
                        q = p;
                        break;
                    }
                    // p / -1 is -p
                    if (r->type == c_number && r->number == -1) {
                        q = new_node(a, c_neg);
                        q->right = p;
                        q->line_number = p->line_number;
                        break;
                    }
                    // p / 0 is an error!
                    if (r->type == c_number && r->number == 0) {
                        fprintf(stderr, "%s:%d: Division by zero\n",
                                t->file, t->line_number);
                        exit(1);
                    }
                    break;
            }
            if (!q) {
                q = new_node(a, token);
                q->left = p;
                q->right = r;
            }
        }
        p = q;
    }
}

static struct node * read_C_connection(struct analyser * a, struct node * q, int op) {
    struct tokeniser * t = a->tokeniser;
    struct node * p = new_node(a, op);
    struct node * p_end = q;
    p->left = q;
    do {
        q = read_C(a);
        p_end->right = q; p_end = q;
    } while (read_token(t) == op);
    hold_token(t);
    return p;
}

static struct node * read_C_list(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    struct node * p = new_node(a, c_bra);
    struct node * p_end = NULL;
    while (true) {
        int token = read_token(t);
        if (token == c_ket) return p;
        if (token < 0) { omission_error(a, c_ket); return p; }
        hold_token(t);

        struct node * q = read_C(a);
        while (true) {
            token = read_token(t);
            if (token != c_and && token != c_or) {
                hold_token(t);
                break;
            }
            q = read_C_connection(a, q, token);
        }
        if (p_end == NULL) p->left = q; else p_end->right = q;
        p_end = q;
    }
}

static struct node * new_string_command(struct analyser * a, int token) {
    struct node * p = new_node(a, token);
    int str_token = read_token(a->tokeniser);
    if (str_token == c_literalstring) {
        p->literalstring = new_literalstring(a);
    } else if (str_token == c_name) {
        name_to_node(a, p, t_string);
    } else {
        report_error_location(a);
        fprintf(stderr, "string omitted");
        report_error_after(a);
        putc('\n', stderr);
        hold_token(a->tokeniser);
    }
    return p;
}

static struct node * read_literalstring(struct analyser * a) {
    struct node * p = new_node(a, c_literalstring);
    p->literalstring = new_literalstring(a);
    return p;
}

static void reverse_b(symbol * b) {
    int i = 0; int j = SIZE(b) - 1;
    while (i < j) {
        int ch1 = b[i]; int ch2 = b[j];
        b[i++] = ch2; b[j--] = ch1;
    }
}

static int compare_amongvec(const void *pv, const void *qv) {
    const struct amongvec * p = (const struct amongvec*)pv;
    const struct amongvec * q = (const struct amongvec*)qv;
    symbol * b_p = p->b; int p_size = p->size;
    symbol * b_q = q->b; int q_size = q->size;
    int smaller_size = p_size < q_size ? p_size : q_size;
    for (int i = 0; i < smaller_size; i++)
        if (b_p[i] != b_q[i]) return b_p[i] - b_q[i];
    if (p_size - q_size)
        return p_size - q_size;
    return p->line_number - q->line_number;
}

#define nodes_equivalent(P, Q) \
    ((P) == (Q) || ((P) && (Q) && nodes_equivalent_((P), (Q))))

static int nodes_equivalent_(const struct node *p, const struct node *q) {
    if (p == q) return true;
    if (p == NULL || q == NULL) return false;

    if (p->type != q->type) return false;
    if (p->mode != q->mode) return false;
    if (p->type == c_number) {
        if (p->number != q->number)
            return false;
    }

    if (!nodes_equivalent(p->left, q->left)) return false;
    if (!nodes_equivalent(p->AE, q->AE)) return false;
    if (!nodes_equivalent(p->aux, q->aux)) return false;

    if (p->name != q->name) return false;

    if (p->literalstring != q->literalstring) {
        if (!p->literalstring ||
            !q->literalstring ||
            SIZE(p->literalstring) != SIZE(q->literalstring) ||
            memcmp(p->literalstring, q->literalstring,
                   SIZE(p->literalstring) * sizeof(symbol)) != 0) {
            return false;
        }
    }

    return nodes_equivalent(p->right, q->right);
}

static struct node * make_among(struct analyser * a, struct node * p, struct node * substring) {
    NEW(among, x);
    NEWVEC(amongvec, v, p->number);
    struct node * q = p->left;
    struct node * starter = NULL;
    struct amongvec * w0 = v;
    struct amongvec * w1 = v;
    int result = 1;

    int direction = substring != NULL ? substring->mode : p->mode;
    int backward = direction == m_backward;

    *x = (struct among){0};
    x->node = p;
    x->b = v;
    x->shortest_size = INT_MAX;
    x->in_routine = a->current_routine;

    if (q->type == c_bra) {
        starter = q;
        p->left = q = q->right;
    }

    while (q) {
        if (q->type == c_literalstring) {
            symbol * b = q->literalstring;
            w1->b = b;           /* pointer to case string */
            w1->action = NULL;   /* action gets filled in later */
            w1->line_number = q->line_number;
            w1->size = SIZE(b);  /* number of characters in string */
            w1->i = -1;          /* index of longest substring */
            w1->result = -1;     /* number of corresponding case expression */
            if (q->left) {
                struct name * function = q->left->name;
                w1->function = function;
                ++function->used_in_among;
                check_routine_mode(a, function, direction);
                if (function->among_index == 0) {
                    function->among_index = ++x->function_count;
                }
                w1->function_index = function->among_index;
            } else {
                w1->function = NULL;
                w1->function_index = 0;
                if (w1->size == 0) {
                    // This among contains the empty string without a gating
                    // function so it will always match.
                    x->always_matches = true;
                }
            }
            w1++;
        } else if (q->left == NULL) {
            /* empty command: () */
            w0 = w1;
        } else {
            /* Check for previous action which is the same as this one and use
             * the same action code if we find one.
             */
            int among_result = -1;
            struct node * action = q;
            struct amongvec * w;
            for (w = v; w < w0; ++w) {
                if (w->action && nodes_equivalent(w->action->left, q->left)) {
                    if (w->result <= 0) {
                        printf("Among code %d isn't positive\n", w->result);
                        exit(1);
                    }
                    action = w->action;
                    among_result = w->result;
                    break;
                }
            }
            if (among_result < 0) {
                among_result = result++;
            }

            while (w0 != w1) {
                w0->action = action;
                w0->result = among_result;
                w0++;
            }
        }
        q = q->right;
    }
    if (w1-v != p->number) { fprintf(stderr, "oh! %d %d\n", (int)(w1-v), p->number); exit(1); }
    x->command_count = result - 1;
    {
        NEWVEC(node*, commands, x->command_count);
        for (int i = 0; i != x->command_count; ++i)
            commands[i] = NULL;
        for (w0 = v; w0 < w1; w0++) {
            if (w0->result > 0) {
                /* result == -1 when there's no command. */
                if (w0->result > x->command_count) {
                    fprintf(stderr, "More among codes than expected\n");
                    exit(1);
                }
                if (!commands[w0->result - 1])
                    commands[w0->result - 1] = w0->action;
            } else {
                ++x->nocommand_count;
            }
            if (backward) reverse_b(w0->b);
        }
        x->commands = commands;
    }
    qsort(v, w1 - v, sizeof(struct amongvec), compare_amongvec);

    /* the following loop is O(n squared) */
    for (w0 = w1 - 1; w0 >= v; w0--) {
        symbol * b = w0->b;
        int size = w0->size;
        struct amongvec * w;

        if (size) {
            if (size < x->shortest_size) x->shortest_size = size;
            if (size > x->longest_size) x->longest_size = size;
        }

        for (w = w0 - 1; w >= v; w--) {
            if (w->size < size && memcmp(w->b, b, w->size * sizeof(symbol)) == 0) {
                w0->i = w - v;  /* fill in index of longest substring */
                break;
            }
        }
    }
    if (backward) for (w0 = v; w0 < w1; w0++) reverse_b(w0->b);

    for (w0 = v; w0 < w1 - 1; w0++)
        if (w0->size == (w0 + 1)->size &&
            memcmp(w0->b, (w0 + 1)->b, w0->size * sizeof(symbol)) == 0) {
            count_error(a);
            fprintf(stderr, "%s:%d: among(...) has repeated string '",
                    a->tokeniser->file, (w0 + 1)->line_number);
            report_b(stderr, (w0 + 1)->b);
            fprintf(stderr, "'\n");
            count_error(a);
            fprintf(stderr, "%s:%d: previously seen here\n",
                    a->tokeniser->file, w0->line_number);
        }

    x->literalstring_count = p->number;
    p->among = x;

    if (starter) {
        starter->right = p;
        p = new_node(a, c_bra);
        if (substring) {
            p->left = starter;
        } else {
            substring = new_node(a, c_substring);
            substring->right = starter;
            p->left = substring;
        }
    }

    // Clear any among_index values we set so we correctly handle a function
    // used in more than one among.
    for (int i = 0; i < x->literalstring_count; i++) {
        if (v[i].function) {
            v[i].function->among_index = 0;
        }
    }

    if (x->literalstring_count == 1) {
        // Eliminate single-case amongs.  Sometimes it's the natural way to
        // express a single rule in Snowball code as it can show commonality
        // with rulesets with multiple rules, but it's silly to actually
        // generate as an among.
        if (substring) {
            substring->among = NULL;
            substring->type = c_literalstring;
            substring->literalstring = v[0].b;
            if (v[0].action) {
                // substring ... among ( S (C) )
                //
                // becomes:
                //
                // S ... (C)
                p = v[0].action;
            } else {
                // substring ... among ( S )
                //
                // becomes:
                //
                // S ... true
                p = new_node(a, c_true);
            }
        } else {
            if (v[0].action) {
                // among ( S (C) )
                //
                // becomes:
                //
                // (S C)
                p = v[0].action;
                assert(p->type == c_bra);
                // Insert a c_literalstring node at the start of (C)
                struct node * literalstring = new_node(a, c_literalstring);
                literalstring->literalstring = v[0].b;
                literalstring->right = p->left;
                p->left = literalstring;
            } else {
                // among ( S )
                //
                // becomes:
                //
                // S
                p->type = c_literalstring;
                p->literalstring = v[0].b;
                p->left = NULL;
            }
        }
        if (v[0].function) {
            // If there's an among function, convert the action to:
            //
            // FUNC and C
            struct node * and_node = new_node(a, c_and);
            and_node->left = new_node(a, c_call);
            and_node->left->name = v[0].function;
            and_node->left->right = p;
            p = and_node;
            --v[0].function->used_in_among;
        }
        FREE(x->commands);
        FREE(x);
        FREE(v);
        return p;
    }

    if (x->function_count) {
        if (a->current_routine) a->current_routine->among_with_function = true;
    }

    x->substring = substring;
    if (substring != NULL) substring->among = x;

    if (a->amongs == NULL) a->amongs = x; else a->amongs_end->next = x;
    a->amongs_end = x;

    return p;
}

static int
is_just_true(struct node * q)
{
    if (!q) return 1;
    if (q->type != c_bra && q->type != c_true) return 0;
    return is_just_true(q->left) && is_just_true(q->right);
}

static struct node * read_among(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    struct node * p = new_node(a, c_among);
    struct node * p_end = NULL;
    int previous_token = -1;
    struct node * substring = a->substring;

    a->substring = NULL;
    p->number = 0; /* counts the number of literals */
    if (!get_token(a, c_bra)) return p;
    while (true) {
        struct node * q;
        int token = read_token(t);
        switch (token) {
            case c_literalstring:
                q = read_literalstring(a);
                if (read_token(t) == c_name) {
                    struct node * r = new_node(a, c_name);
                    name_to_node(a, r, t_routine);
                    q->left = r;
                } else {
                    hold_token(t);
                }
                p->number++; break;
            case c_bra:
                if (previous_token == c_bra) {
                    report_error_location(a);
                    fprintf(stderr, "two adjacent bracketed expressions in among(...)\n");
                }
                q = read_C_list(a);
                if (is_just_true(q->left)) {
                    /* Convert anything equivalent to () to () so we handle it
                     * the same way.
                     */
                    q->left = NULL;
                }
                break;
            default:
                unexpected_token_error(a, "among(...)");
                previous_token = token;
                continue;
            case c_ket:
                if (p->number == 0) {
                    report_error_location(a);
                    fprintf(stderr, "empty among(...)\n");
                }
                if (t->error_count == 0) p = make_among(a, p, substring);
                return p;
        }
        previous_token = token;
        if (p_end == NULL) p->left = q; else p_end->right = q;
        p_end = q;
    }
}

static struct node * read_substring(struct analyser * a) {
    struct node * p = new_node(a, c_substring);
    if (a->substring != NULL) {
        substring_without_among_error(a);
    }
    a->substring = p;
    return p;
}

static void check_modifyable(struct analyser * a) {
    if (!a->modifyable) {
        struct tokeniser * t = a->tokeniser;
        report_error_location(a);
        fprintf(stderr, "%s not allowed inside reverse(...)\n",
                name_of_token(t->token));
    }
}

static int ae_uses_name(struct node * p, struct name * q) {
    if (!p) {
        // AE is NULL after a syntax error, e.g. `$x = $y`
        return 0;
    }
    switch (p->type) {
        case c_name:
        case c_lenof:
        case c_sizeof:
            if (p->name == q) return 1;
            break;
        case c_neg:
            return ae_uses_name(p->right, q);
        case c_multiply:
        case c_plus:
        case c_minus:
        case c_divide:
            return ae_uses_name(p->left, q) || ae_uses_name(p->right, q);
    }
    return 0;
}

static struct node * read_C(struct analyser * a) {
    struct tokeniser * t = a->tokeniser;
    int token = read_token(t);
    switch (token) {
        case c_bra: {
            struct node * p = read_C_list(a);
            if (p->type != c_bra) {
                fprintf(stderr, "read_C_list returned unexpected type %s\n",
                        name_of_token(p->type));
                exit(1);
            }
            if (p->left && !p->left->right) {
                // Replace a single entry command list with the command it
                // contains in order to make subsequent optimisations easier.
                p = p->left;
            }
            return p;
        }
        case c_backwards: {
            int mode = a->mode;
            if (a->mode == m_backward) {
                report_error_location(a);
                fprintf(stderr, "'backwards' used when already in this mode\n");
            }
            a->mode = m_backward;
            struct node * p = new_node(a, token);
            p->left = read_C(a);
            a->mode = mode;
            return p;
        }
        case c_reverse: {
            int mode = a->mode;
            int modifyable = a->modifyable;
            a->modifyable = false;
            a->mode = mode == m_forward ? m_backward : m_forward;
            struct node * p = new_node(a, token);
            p->left = read_C(a);
            a->mode = mode;
            a->modifyable = modifyable;
            return p;
        }
        case c_not: {
            struct node * subcommand = read_C(a);
            if (subcommand->type == c_booltest) {
                /* We synthesise a special command for "not" applied to testing
                 * a boolean variable.
                 */
                subcommand->type = c_not_booltest;
                return subcommand;
            }
            struct node * p = new_node(a, token);
            p->left = subcommand;
            return p;
        }
        case c_try:
        case c_test:
        case c_do:
        case c_repeat: {
            struct node * p = new_node(a, token);
            p->left = read_C(a);
            return p;
        }
        case c_fail: {
            struct node * p = new_node(a, token);
            p->left = read_C(a);
            if (!p->left || is_just_true(p->left)) {
                p->type = c_false;
                p->left = NULL;
            }
            return p;
        }
        case c_goto:
        case c_gopast: {
            struct node * subcommand = read_C(a);
            if (subcommand->type == c_grouping || subcommand->type == c_non) {
                /* We synthesise special commands for "goto" or "gopast" when
                 * used on a grouping or an inverted grouping - the movement of
                 * c by the matching action is exactly what we want!
                 *
                 * Adding the tokens happens to give unique values (the code
                 * would fail to compile if it didn't!)
                 */
                switch (token + subcommand->type) {
                    case c_goto + c_grouping:
                        subcommand->type = c_goto_grouping;
                        break;
                    case c_gopast + c_grouping:
                        subcommand->type = c_gopast_grouping;
                        break;
                    case c_goto + c_non:
                        subcommand->type = c_goto_non;
                        break;
                    case c_gopast + c_non:
                        subcommand->type = c_gopast_non;
                        break;
                    default:
                        fprintf(stderr, "Unexpected go/grouping combination: %s %s",
                                name_of_token(token),
                                name_of_token(subcommand->type));
                        exit(1);
                }
                return subcommand;
            }

            struct node * p = new_node(a, token);
            p->left = subcommand;
            return p;
        }
        case c_loop:
        case c_atleast: {
            struct node * n = new_node(a, token);
            n->AE = read_AE(a, NULL, 0);
            n->left = read_C(a);

            // n->AE is NULL after a syntax error, e.g. `loop next`.
            if (n->AE && n->AE->type == c_number) {
                if (n->AE->number <= 0) {
                    if (token == c_loop) {
                        // `loop N C`, where N <= 0 is a no-op.
                        if (n->AE->fixed_constant) {
                            fprintf(stderr,
                                    "%s:%d: warning: loop %d C is a no-op\n",
                                    t->file, n->AE->line_number, n->AE->number);
                        }
                        n->AE = NULL;
                        n->left = NULL;
                        n->type = c_true;
                    } else {
                        // `atleast N C` where N <= 0 -> `repeat C`.
                        if (n->AE->fixed_constant) {
                            fprintf(stderr,
                                    "%s:%d: warning: atleast %d C is just repeat C\n",
                                    t->file, n->AE->line_number, n->AE->number);
                        }
                        n->AE = NULL;
                        n->type = c_repeat;
                    }
                } else if (n->AE->number == 1) {
                    if (token == c_loop) {
                        // `loop 1 C` -> `C`.
                        if (n->AE->fixed_constant) {
                            fprintf(stderr,
                                    "%s:%d: warning: loop 1 C is just C\n",
                                    t->file, n->AE->line_number);
                        }
                        n = n->left;
                    }
                }
            }
            return n;
        }
        case c_setmark: {
            struct node * n = new_node(a, token);
            if (get_token(a, c_name)) {
                name_to_node(a, n, t_integer);
                if (n->name) n->name->initialised = true;
            }
            return n;
        }
        case c_tomark:
        case c_atmark: {
            struct node * n = new_node(a, token);
            n->AE = read_AE(a, NULL, 0);
            return n;
        }
        case c_hop: {
            struct node * n = new_node(a, token);
            n->AE = read_AE(a, NULL, 0);
            // n->AE is NULL after a syntax error, e.g. `hop hop`.
            if (n->AE && n->AE->type == c_number) {
                if (n->AE->number == 1) {
                    // Convert `hop 1` to `next`.
                    n->AE = NULL;
                    n->type = c_next;
                } else if (n->AE->number == 0) {
                    if (n->AE->fixed_constant) {
                        fprintf(stderr,
                                "%s:%d: warning: hop 0 is a no-op\n",
                                t->file, n->AE->line_number);
                    }
                    n->AE = NULL;
                    n->type = c_true;
                } else if (n->AE->number < 0) {
                    fprintf(stderr,
                            "%s:%d: warning: hop %d now signals f (as was "
                            "always documented) rather than moving the cursor "
                            "in the opposite direction\n",
                            t->file, n->AE->line_number, n->AE->number);
                    n->AE = NULL;
                    n->type = c_false;
                }
            }
            return n;
        }
        case c_delete:
            check_modifyable(a);
            /* fall through */
        case c_next:
        case c_tolimit:
        case c_atlimit:
        case c_leftslice:
        case c_rightslice:
        case c_true:
        case c_false:
            return new_node(a, token);
        case c_debug:
            a->debug_used = true;
            return new_node(a, token);
        case c_assignto:
        case c_sliceto: {
            check_modifyable(a);
            struct node * n = new_node(a, token);
            if (get_token(a, c_name)) {
                name_to_node(a, n, t_string);
                if (n->name) n->name->initialised = true;
            }
            if (token == c_assignto) {
                fprintf(stderr,
                        "%s:%d: warning: Use of `=>` is not recommended, "
                        "see https://snowballstem.org/compiler/snowman.html "
                        "section 13.3 for details\n",
                        t->file, n->line_number);
            }
            return n;
        }
        case c_assign:
        case c_insert:
        case c_attach:
        case c_slicefrom: {
            check_modifyable(a);
            struct node * n = new_string_command(a, token);
            if (n->name) n->name->value_used = true;
            return n;
        }
        case c_setlimit: {
            struct node * n = new_node(a, token);
            n->left = read_C(a);
            get_token(a, c_for);
            n->aux = read_C(a);
            return n;
        }
        case c_set:
        case c_unset: {
            struct node * n = new_node(a, token);
            if (get_token(a, c_name)) {
                name_to_node(a, n, t_boolean);
                if (n->name) n->name->initialised = true;
            }
            return n;
        }
        case c_dollar: {
            read_token(t);
            if (t->token == c_bra) {
                /* Handle newer $(AE REL_OP AE) syntax. */
                struct node * n = read_AE(a, NULL, 0);
                read_token(t);
                token = t->token;
                switch (token) {
                    case c_assign:
                        count_error(a);
                        fprintf(stderr, "%s:%d: Expected relational operator (did you mean '=='?)\n",
                                t->file, t->line_number);
                        // Assume it was == to try to avoid an error avalanche.
                        token = c_eq;
                        /* FALLTHRU */
                    case c_eq:
                    case c_ne:
                    case c_gt:
                    case c_ge:
                    case c_lt:
                    case c_le: {
                        struct node * lhs = n;
                        struct node * rhs = read_AE(a, NULL, 0);
                        if (lhs->type == c_number && rhs->type == c_number) {
                            // Evaluate constant numeric test expression.
                            int result;
                            switch (token) {
                                case c_eq:
                                    result = (lhs->number == rhs->number);
                                    break;
                                case c_ne:
                                    result = (lhs->number != rhs->number);
                                    break;
                                case c_gt:
                                    result = (lhs->number > rhs->number);
                                    break;
                                case c_ge:
                                    result = (lhs->number >= rhs->number);
                                    break;
                                case c_lt:
                                    result = (lhs->number < rhs->number);
                                    break;
                                case c_le:
                                    result = (lhs->number <= rhs->number);
                                    break;
                                default:
                                    fprintf(stderr, "Unexpected numeric test operator %s\n",
                                            name_of_token(t->token));
                                    exit(1);
                            }
                            n = new_node(a, result ? c_true : c_false);
                        } else {
                            n = new_node(a, token);
                            n->left = lhs;
                            n->AE = rhs;
                        }
                        get_token(a, c_ket);
                        break;
                    }
                    default:
                        unexpected_token_error(a, "integer test expression");
                        hold_token(t);
                        (void)read_AE(a, NULL, 0);
                        get_token(a, c_ket);
                        break;
                }
                return n;
            }

            if (t->token != c_name) {
                unexpected_token_error(a, "integer test expression");
                hold_token(t);
                return new_node(a, c_dollar);
            }

            struct name * q = find_name(a);
            if (q && q->type == t_string) {
                /* Assume for now that $ on string both initialises and uses
                 * the string variable.  FIXME: Can we do better?
                 */
                q->initialised = true;
                q->value_used = true;
                struct node * p = new_node(a, c_dollar);
                int mode = a->mode;
                int modifyable = a->modifyable;
                a->mode = m_forward;
                a->modifyable = true;
                p->left = read_C(a);
                a->mode = mode;
                a->modifyable = modifyable;
                p->name = q;
                mark_used_in(a, q, p);
                return p;
            }

            if (q && q->type != t_integer) {
                /* If $ is used on an unknown name or a name which isn't a
                 * string or an integer then we assume the unknown name is an
                 * integer as $ is used more often on integers than strings, so
                 * hopefully this it less likely to cause an error avalanche.
                 *
                 * For an unknown name, we'll already have reported an error.
                 */
                report_error_location(a);
                fprintf(stderr, "'%.*s' not of type integer or string\n",
                        SIZE(q->s), q->s);
                q = NULL;
            }
            struct node * p = new_node(a, read_AE_test(a));
            switch (p->type) {
                case c_eq:
                case c_ne:
                case c_gt:
                case c_ge:
                case c_lt:
                case c_le:
                    p->left = new_node(a, c_name);
                    p->left->name = q;
                    p->AE = read_AE(a, NULL, 0);
                    if (q) {
                        q->value_used = true;
                        mark_used_in(a, q, p);
                    }
                    return p;
            }

            /* +=, etc don't "initialise" as they only amend an existing value.
             * Similarly, they don't count as using the value.
             */
            p->name = q;
            p->AE = read_AE(a, q, 0);
            if (p->AE && p->AE->type == c_number) {
                switch (p->type) {
                    case c_plusassign:
                    case c_minusassign:
                        if (p->AE->number == 0) {
                            // `$x+=0` and `$x-=0` are no-ops.
                            p->type = c_true;
                            p->name = NULL;
                            p->AE = NULL;
                        } else if (p->AE->number < 0) {
                            // `$x+=-N` -> `$x-=N`, etc as
                            // this may result in slightly
                            // shorter target language code.
                            p->type ^= (c_plusassign ^ c_minusassign);
                            p->AE->number = -p->AE->number;
                        }
                        break;
                    case c_multiplyassign:
                    case c_divideassign:
                        if (p->AE->number == 1) {
                            // `$x*=1` and `$x/=1` are no-ops.
                            p->type = c_true;
                            p->name = NULL;
                            p->AE = NULL;
                        } else if (p->AE->number == 0) {
                            if (p->type == c_divide) {
                                fprintf(stderr, "%s:%d: Division by zero\n",
                                        t->file, t->line_number);
                                exit(1);
                            }
                            // `$x*=0` -> `$x=0`
                            p->type = c_mathassign;
                        } else if (p->AE->number == -1) {
                            // `$x/=-1` -> `$x*=-1`
                            p->type = c_multiplyassign;
                        }
                        break;
                }
            }
            if (p->type == c_mathassign && q) {
                /* $x = x + 1 doesn't initialise x. */
                if (!ae_uses_name(p->AE, q)) {
                    q->initialised = true;
                }
            }
            if (q) mark_used_in(a, q, p);
            return p;
        }
        case c_name:
            {
                struct name * q = find_name(a);
                struct node * p = new_node(a, c_name);
                if (q) {
                    mark_used_in(a, q, p);
                    switch (q->type) {
                        case t_boolean:
                            p->type = c_booltest;
                            q->value_used = true;
                            break;
                        case t_integer:
                            report_error_location(a);
                            fprintf(stderr, "integer name '%.*s' misplaced\n",
                                    SIZE(t->s), t->s);
                            break;
                        case t_string:
                            q->value_used = true;
                            break;
                        case t_routine:
                        case t_external:
                            p->type = c_call;
                            check_routine_mode(a, q, a->mode);
                            break;
                        case t_grouping:
                            p->type = c_grouping; break;
                    }
                }
                p->name = q;
                return p;
            }
        case c_non:
            {
                struct node * p = new_node(a, token);
                read_token(t);
                if (t->token == c_minus) read_token(t);
                if (!check_token(a, c_name)) {
                    hold_token(t);
                    return p;
                }
                name_to_node(a, p, t_grouping);
                return p;
            }
        case c_literalstring: {
            struct node * p = read_literalstring(a);
            if (SIZE(p->literalstring) == 0) {
                fprintf(stderr,
                        "%s:%d: warning: empty literal string is a no-op\n",
                        t->file, p->line_number);
                p->type = c_true;
                p->literalstring = NULL;
            }
            return p;
        }
        case c_among: return read_among(a);
        case c_substring: return read_substring(a);
        default:
            unexpected_token_error(a, 0);
            return NULL;
    }
}

static int next_symbol(symbol * p, symbol * W, int utf8) {
    if (utf8) {
        int ch;
        int j = get_utf8(p, & ch);
        *W = ch;
        return j;
    } else {
        *W = *p;
        return 1;
    }
}

static symbol * alter_grouping(symbol * p, symbol * q, int style, int utf8) {
    int j = 0;
    if (style == c_plus) {
        while (j < SIZE(q)) {
            symbol W;
            int width = next_symbol(q + j, &W, utf8);
            p = add_symbol_to_b(p, W);
            j += width;
        }
    } else {
        while (j < SIZE(q)) {
            symbol W;
            int width = next_symbol(q + j, &W, utf8);
            for (int i = 0; i < SIZE(p); i++) {
                if (p[i] == W) {
                    memmove(p + i, p + i + 1, (SIZE(p) - i - 1) * sizeof(symbol));
                    ADD_TO_SIZE(p, -1);
                }
            }
            j += width;
        }
    }
    return p;
}

static int compare_symbol(const void *pv, const void *qv) {
    const symbol * p = (const symbol*)pv;
    const symbol * q = (const symbol*)qv;
    return *p - *q;
}

static int finalise_grouping(struct grouping * p) {
    if (SIZE(p->b) == 0) {
        // Empty grouping - leave things in a non-surprising state.
        p->smallest_ch = p->largest_ch = 0;
        return false;
    }

    qsort(p->b, SIZE(p->b), sizeof(symbol), compare_symbol);
    p->smallest_ch = p->b[0];
    p->largest_ch = p->b[SIZE(p->b) - 1];

    // Eliminate duplicates.
    symbol ch = p->b[0];
    int j = 1;
    for (int i = 1; i < SIZE(p->b); i++) {
        if (p->b[i] != ch) {
            ch = p->b[j++] = p->b[i];
        }
    }
    SET_SIZE(p->b, j);
    return true;
}

static void read_define_grouping(struct analyser * a, struct name * q) {
    struct tokeniser * t = a->tokeniser;
    int style = c_plus;
    {
        NEW(grouping, p);
        *p = (struct grouping){0};
        if (a->groupings == NULL) a->groupings = p; else a->groupings_end->next = p;
        a->groupings_end = p;
        if (q) {
            if (q->grouping != NULL) {
                report_error_location(a);
                fprintf(stderr, "'%.*s' redefined\n", SIZE(t->s), t->s);
                q->grouping->name = NULL;
            }
            q->grouping = p;
        }
        p->name = q;
        p->line_number = t->line_number;
        p->b = create_b(0);
        while (true) {
            switch (read_token(t)) {
                case c_name: {
                    struct name * r = find_name(a);
                    if (!r) break;

                    check_name_type(a, r, t_grouping);
                    if (r == q) {
                        count_error(a);
                        fprintf(stderr, "%s:%d: %.*s defined in terms of itself\n",
                                t->file, t->line_number, SIZE(r->s), r->s);
                    } else if (!r->grouping) {
                        count_error(a);
                        fprintf(stderr, "%s:%d: %.*s undefined\n",
                                t->file, t->line_number, SIZE(r->s), r->s);
                    } else {
                        p->b = alter_grouping(p->b, r->grouping->b, style, false);
                    }
                    r->used_in_definition = true;
                    break;
                }
                case c_literalstring:
                    p->b = alter_grouping(p->b, t->b, style, (a->encoding == ENC_UTF8));
                    break;
                default:
                    unexpected_token_error(a, "grouping definition");
                    hold_token_if_toplevel(t);
                    // Don't report an error for an empty grouping as well.
                    (void)finalise_grouping(p);
                    return;
            }
            switch (read_token(t)) {
                case c_plus:
                case c_minus: style = t->token; break;
                default: goto label0;
            }
        }
    label0:
        if (!finalise_grouping(p)) {
            report_error_location(a);
            fprintf(stderr, "empty grouping\n");
        }
        hold_token(t);
    }
}

static void read_define_routine(struct analyser * a, struct name * q) {
    struct node * p = new_node(a, c_define);
    a->current_routine = q;
    if (q) {
        int type = q->type;
        if (type != t_grouping && type != t_routine && type != t_external) {
            report_error_location(a);
            fprintf(stderr, "'%.*s' not of type grouping, routine or external\n",
                    SIZE(q->s), q->s);
        }
        if (q->definition != NULL) {
            report_error_location(a);
            fprintf(stderr, "'%.*s' redefined\n", SIZE(q->s), q->s);
        }
        if (q->mode == m_unknown) {
            q->mode = a->mode;
        } else if (q->mode != a->mode) {
            report_error_location(a);
            fprintf(stderr, "'%.*s' declared as %s mode; used as %s mode",
                    SIZE(q->s), q->s,
                    name_of_mode(a->mode), name_of_mode(q->mode));
        }
    }
    p->name = q;
    if (a->program == NULL) a->program = p; else a->program_end->right = p;
    a->program_end = p;
    get_token(a, c_as);
    p->left = read_C(a);
    if (q) q->definition = p;
    /* We should get a node with a NULL right pointer from read_C() for the
     * routine's code.  We synthesise a "functionend" node there so
     * optimisations such as dead code elimination and tail call optimisation
     * can easily see where the function ends.
     */
    assert(p->left->right == NULL);
    if (p->left->type == c_bra) {
        /* Put the "functionend" node at the end of the command list. */
        struct node * e = p->left->left;
        if (e) {
            while (e->right) e = e->right;
            e->right = new_node(a, c_functionend);
        } else {
            p->left = new_node(a, c_functionend);
        }
    } else {
        /* Put the "functionend" node after the single command. */
        p->left->right = new_node(a, c_functionend);
    }

    if (a->substring != NULL) {
        substring_without_among_error(a);
        a->substring = NULL;
    }
    a->current_routine = NULL;
}

static void read_define(struct analyser * a) {
    if (get_token(a, c_name)) {
        struct name * q = find_name(a);
        int type;
        if (q) {
            type = q->type;
        } else {
            /* No declaration so sniff next token - if it is a string or name
             * we parse as a grouping, otherwise we parse as a routine.  This
             * avoids an avalanche of further errors if `as` is missing from a
             * routine definition.
             */
            switch (peek_token(a->tokeniser)) {
                case c_literalstring:
                case c_name:
                    type = t_grouping;
                    break;
                default:
                    type = t_routine;
            }
        }

        if (type == t_grouping) {
            read_define_grouping(a, q);
        } else {
            read_define_routine(a, q);
        }
    }
}

static void read_backwardmode(struct analyser * a) {
    int mode = a->mode;
    a->mode = m_backward;
    if (get_token(a, c_bra)) {
        read_program_(a, c_ket);
        check_token(a, c_ket);
    }
    a->mode = mode;
}

static void read_program_(struct analyser * a, int terminator) {
    struct tokeniser * t = a->tokeniser;
    while (true) {
        int token = read_token(t);
        switch (token) {
            case c_strings:     read_names(a, t_string); break;
            case c_booleans:    read_names(a, t_boolean); break;
            case c_integers:    read_names(a, t_integer); break;
            case c_routines:    read_names(a, t_routine); break;
            case c_externals:   read_names(a, t_external); break;
            case c_groupings:   read_names(a, t_grouping); break;
            case c_define:      read_define(a); break;
            case c_backwardmode:read_backwardmode(a); break;
            default:
                if (token == terminator) return;
                unexpected_token_error(a, 0);
                return;
            case -1:
                return;
        }
    }
}

static void remove_dead_assignments(struct node * p, struct name * q) {
    if (p->name == q) {
        switch (p->type) {
            case c_assignto:
            case c_sliceto:
            case c_mathassign:
            case c_plusassign:
            case c_minusassign:
            case c_multiplyassign:
            case c_divideassign:
            case c_setmark:
            case c_set:
            case c_unset:
            case c_dollar:
                /* c_true is a no-op. */
                p->type = c_true;
                p->AE = NULL;
                p->name = NULL;
                break;
            default:
                /* There are no read accesses to this variable, so any
                 * references must be assignments.
                 */
                fprintf(stderr, "Unhandled type of dead assignment via %s\n",
                        name_of_token(p->type));
                exit(1);
        }
    }
    if (p->AE) remove_dead_assignments(p->AE, q);
    if (p->left) remove_dead_assignments(p->left, q);
    if (p->aux) remove_dead_assignments(p->aux, q);
    if (p->right) remove_dead_assignments(p->right, q);
}

enum {
    // Not set on at least one code path leading to a use.
    USE_BEFORE_SET,
    // Need to keep checking.
    UNKNOWN,
    // Set on any code path leading to a use.
    SET_BEFORE_ANY_USE
};

/* Find out if every codepath in the command with node p to a use of variable v
 * sets v first.
 *
 * The checks err towards being too conservative and may report that v can't be
 * safely localised when it can, but they allow localising all variables which
 * can be trivially made local in existing stemmers.
 *
 * p:    the node of the command to check.
 * func: the c_define of the routine/external this code is in.
 * v:    the variable to check.
 */
static int always_set_before_use_(struct node * p, struct node * func,
                                  struct name * v) {
    if (!p) return UNKNOWN;
    switch (p->type) {
        case c_call: {
            if (p->name->definition == func) {
                /* We've recursed into the function we're considering
                 * localising this variable into, which means we can't
                 * localise it because then changes to the variable in
                 * the nested call won't be reflected after it returns.
                 */
                return USE_BEFORE_SET;
            }
            // We know v is only referenced in the function we are checking.
            return UNKNOWN;
        }
        case c_among: {
            int all_pass = true;
            struct among * x = p->among;
            for (int i = 1; i <= x->command_count; i++) {
                int r = always_set_before_use_(x->commands[i - 1], func, v);
                if (r == USE_BEFORE_SET) return r;
                all_pass = all_pass && (r == SET_BEFORE_ANY_USE);
            }
            if (all_pass) return SET_BEFORE_ANY_USE;
            return UNKNOWN;
        }
        case c_or: {
            struct node * q = p->left;
            int all_pass = true;
            while (q) {
                int r = always_set_before_use_(q, func, v);
                if (r == USE_BEFORE_SET) return r;
                all_pass = all_pass && (r == SET_BEFORE_ANY_USE);
                q = q->right;
            }
            if (all_pass) return SET_BEFORE_ANY_USE;
            return UNKNOWN;
        }
        case c_and:
        case c_bra: {
            struct node * q = p->left;
            while (q) {
                int r = always_set_before_use_(q, func, v);
                if (r != UNKNOWN) return r;
                q = q->right;
            }
            return UNKNOWN;
        }
        case c_backwards:
        case c_not:
        case c_reverse:
        case c_test:
            return always_set_before_use_(p->left, func, v);
        case c_do:
        case c_fail:
        case c_gopast:
        case c_goto:
        case c_try:
        case c_repeat: {
            if (always_set_before_use_(p->left, func, v) == USE_BEFORE_SET)
                return USE_BEFORE_SET;
            return UNKNOWN;
        }
        case c_atleast:
        case c_loop:
            if (always_set_before_use_(p->AE, func, v) == USE_BEFORE_SET)
                return USE_BEFORE_SET;
            return always_set_before_use_(p->left, func, v);
        case c_mathassign:
            // Check AE first: `x = x + 1` uses `x` before it sets it.
            if (always_set_before_use_(p->AE, func, v) == USE_BEFORE_SET)
                return USE_BEFORE_SET;
            if (p->name == v)
                return SET_BEFORE_ANY_USE;
            return UNKNOWN;
        case c_assignto:
        case c_set:
        case c_setmark:
        case c_sliceto:
        case c_unset:
            if (p->name == v)
                return SET_BEFORE_ANY_USE;
            return UNKNOWN;
        case c_atlimit:
        case c_delete:
        case c_grouping:
        case c_leftslice:
        case c_literalstring:
        case c_next:
        case c_non:
        case c_number:
        case c_rightslice:
        case c_debug:
        case c_substring:
        case c_tolimit:
        case c_false:
        case c_true:
        case c_goto_grouping:
        case c_gopast_grouping:
        case c_goto_non:
        case c_gopast_non:
            return UNKNOWN;
        case c_atmark:
        case c_hop:
        case c_tomark:
            if (always_set_before_use_(p->AE, func, v) == USE_BEFORE_SET)
                return USE_BEFORE_SET;
            return UNKNOWN;
        case c_assign:
        case c_attach:
        case c_booltest:
        case c_insert:
        case c_name:
        case c_not_booltest:
        case c_slicefrom:
            if (p->name == v) {
                return USE_BEFORE_SET;
            }
            return UNKNOWN;
        case c_functionend:
            return SET_BEFORE_ANY_USE;
        case c_divide:
        case c_minus:
        case c_multiply:
        case c_plus:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le: {
            int r = always_set_before_use_(p->left, func, v);
            if (r != UNKNOWN) return r;
            return always_set_before_use_(p->right, func, v);
        }
        case c_neg:
            return always_set_before_use_(p->right, func, v);
        case c_lenof:
        case c_sizeof:
            if (p->name == v) {
                return USE_BEFORE_SET;
            }
            return UNKNOWN;
        case c_cursor:
        case c_len:
        case c_limit:
        case c_maxint:
        case c_minint:
        case c_size:
            return UNKNOWN;
        case c_setlimit: {
            int r = always_set_before_use_(p->aux, func, v);
            if (r != UNKNOWN) return r;
            return always_set_before_use_(p->left, func, v);
        }
        case c_divideassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_plusassign:
            if (p->name == v) {
                return USE_BEFORE_SET;
            }
            if (always_set_before_use_(p->AE, func, v) == USE_BEFORE_SET) {
                return USE_BEFORE_SET;
            }
            return UNKNOWN;
        case c_dollar:
            if (p->name != v) {
                return UNKNOWN;
            }
#if 0
            // This check is valid, but currently it's better to not
            // localise a variable if string-$ is used on it has definitely
            // been set because for some target languages that means we need to
            // initialise to an empty string at the start of the function and
            // incur overhead from doing so.
            if (p->left->type == c_assign) {
                // Special-case `$x = S` because it's easy to handle.
                return SET_BEFORE_ANY_USE;
            }
#endif
            // Otherwise, for now we assume that `$x C` might use `x` before
            // setting it.  If string-$ sees wider use we can do better here.
            return USE_BEFORE_SET;
        case c_backwardmode:
        case c_define: // We always start from c_define's ->left.
        case c_booleans:
        case c_externals:
        case c_groupings:
        case c_integers:
        case c_routines:
        case c_strings:
            // Allowing these would allow checking the whole program.
            assert(0);
            return UNKNOWN;
        case c_comment1:
        case c_comment2:
        case c_decimal:
        case c_get:
        case c_hex:
        case c_stringdef:
        case c_stringescapes:
            // These are only use in the tokeniser.
            assert(0);
            break;
        case c_as:
        case c_for:
        case c_ket:
            // These shouldn't occur in this context.
            assert(0);
            break;
    }
    /* Pessimistic assumption for cases we don't handle yet. */
    printf("Assuming the worst about '%s' (%d)\n", name_of_token(p->type), p->type);
    return USE_BEFORE_SET;
}

static int always_set_before_use(struct node * p, struct node * func,
                                 struct name * v) {
    return always_set_before_use_(p, func, v) != USE_BEFORE_SET;
}

static void remove_unreachable_routine(struct analyser * a, struct name * q) {
    struct node ** ptr = &(a->program);
    while (*ptr) {
        if ((*ptr)->name == q) {
            *ptr = (*ptr)->right;
        } else {
            ptr = &((*ptr)->right);
        }
    }
}

// Return 0 for always f.
// Return 1 for always t.
// Return -1 for don't know (or can raise t or f).
static int check_possible_signals(struct analyser * a, struct node * p) {
    switch (p->type) {
        case c_fail:
        case c_false:
            /* Always gives signal f. */
            return 0;
        case c_assign:
        case c_attach:
        case c_debug:
        case c_delete:
        case c_do:
        case c_insert:
        case c_leftslice:
        case c_repeat:
        case c_rightslice:
        case c_set:
        case c_setmark:
        case c_slicefrom:
        case c_sliceto:
        case c_tolimit:
        case c_tomark:
        case c_true:
        case c_try:
        case c_unset:
        case c_mathassign:
        case c_plusassign:
        case c_minusassign:
        case c_multiplyassign:
        case c_divideassign:
        case c_functionend:
            /* Always gives signal t. */
            return 1;
        case c_not: {
            int res = p->left->possible_signals;
            if (res >= 0)
                res = !res;
            if (res == 0 && p->right) {
                if (p->right->type != c_functionend) {
                    fprintf(stderr, "%s:%d: warning: 'not' always signals f here so following commands are unreachable\n",
                            a->tokeniser->file, p->line_number);
                }
                p->right = NULL;
            }
            return res;
        }
        case c_setlimit: {
            /* If either always signals f, setlimit does to. */
            int res = p->left->possible_signals;
            int res2 = p->aux->possible_signals;
            if (res == 0 || res2 == 0) {
                return 0;
            }
            // If both always signal t, setlimit does to.  Otherwise we know at
            // least one is unknown and that means setlimit's signal is unknown.
            // We can achieve that with a simple bitwise or.
            return res | res2;
        }
        case c_and:
        case c_bra: {
            struct node * q = p->left;
            int r = 1;
            while (q) {
                int res = q->possible_signals;
                if (res == 0) {
                    // If any command always signals f, then the list always
                    // signals f.
                    if (q->right) {
                        if (q->right->type != c_functionend) {
                            fprintf(stderr, "%s:%d: warning: command always signals f here so rest of %s is unreachable\n",
                                    a->tokeniser->file, q->line_number,
                                    (p->type == c_and ? "'and'" : "command list"));
                        }
                        q->right = NULL;
                    }
                    return res;
                }
                if (res < 0) r = res;
                q = q->right;
            }
            return r;
        }
        case c_atleast:
        case c_backwards:
        case c_dollar:
        case c_loop:
        case c_reverse:
        case c_test:
            /* Give same signal as p->left. */
            return p->left->possible_signals;
        case c_call:
            // If the call recurses back into the current routine then this
            // will still be -1.
            return p->name->definition->possible_signals;
        case c_gopast:
        case c_goto:
        case c_goto_grouping:
        case c_gopast_grouping:
        case c_goto_non:
        case c_gopast_non:
            /* FIXME: unless we can prove that c is either definitely atlimit
             * or definitely not atlimit... */
            return -1;
        case c_atlimit:
        case c_atmark:
        case c_booltest:
        case c_not_booltest:
        case c_hop:
        case c_literalstring:
        case c_next:
        case c_eq:
        case c_ne:
        case c_gt:
        case c_ge:
        case c_lt:
        case c_le:
        case c_grouping:
        case c_non:
        case c_name:
            /* FIXME: unless we can prove... */
            return -1;
        case c_substring: {
            struct among * x = p->among;
            if (x->always_matches) {
                return 1;
            }
            return -1;
        }
        case c_among: {
            struct among * x = p->among;
            int r = 1;

            if (x->substring == NULL) {
                if (!x->always_matches) {
                    r = -1;
                }
            }

            if (x->command_count > 0) {
                int trues = (x->nocommand_count > 0);
                int falses = false;
                for (int i = 1; i <= x->command_count; i++) {
                    int res = x->commands[i - 1]->possible_signals;
                    if (res == 0) {
                        falses = true;
                    } else if (res > 0) {
                        trues = true;
                    } else {
                        falses = trues = true;
                    }
                    if (falses && trues) break;
                }
                if (!trues) {
                    // All commands in among always fail.
                    return 0;
                }
                if (falses) {
                    // Commands in among can succeed or fail.
                    return -1;
                }
            }
            return r;
        }
        case c_or: {
            int r = 0;
            for (struct node * q = p->left; q; q = q->right) {
                // Just check this node - q->right is a separate clause of
                // the OR.
                int res = q->possible_signals;
                if (res > 0) {
                    // If any clause of the OR always signals t, then the OR
                    // always signals t.
                    if (q->right) {
                        if (q->right->type != c_functionend) {
                            fprintf(stderr, "%s:%d: warning: command always signals t here so rest of 'or' is unreachable\n",
                                    a->tokeniser->file,
                                    q->line_number);
                        }
                        q->right = NULL;
                    }
                    return 1;
                }
                if (res < 0) {
                    r = res;
                }
            }
            return r;
        }
        default:
            return -1;
    }
}

static void visit_routine(struct analyser * a, struct name * n);

static void visit_node(struct analyser * a, struct node * p) {
    while (p) {
        if (p->name) {
            if (p->type == c_call) {
                visit_routine(a, p->name);
            } else {
                // Mark as reachable.
                p->name->count = -2;
            }
        } else if (p->type == c_among) {
            struct among * x = p->among;
            x->used = true;
            for (int i = 0; i < x->literalstring_count; ++i) {
                if (x->b[i].function)
                    visit_routine(a, x->b[i].function);
            }
            for (int i = 0; i < x->command_count; ++i) {
                visit_node(a, x->commands[i]);
            }
        }
        if (p->left) {
            visit_node(a, p->left);
        }
        if (p->aux) {
            visit_node(a, p->aux);
        }
        if (p->AE) {
            visit_node(a, p->AE);
        }

        p->possible_signals = check_possible_signals(a, p);

        p = p->right;
    }
}

static void visit_routine(struct analyser * a, struct name * n) {
    if (n->count == -2) {
        // Already visited.  We set n->count before walking the definition so
        // this also prevents the walk from reentering a routine via recursive
        // calls.
        return;
    }
    n->count = -2;

    struct node * p = n->definition;

    // Recursive functions are valid in the Snowball language, but aren't
    // actually used in typical snowball programs so we take a simple
    // approach and handle them by setting pessimistic assumptions here which
    // will be used if a function calls itself (directly or indirectly).
    p->possible_signals = -1; // Assume it could signal t or f.

    visit_node(a, p->left);

    // Update with calculated value.
    p->possible_signals = p->left->possible_signals;
}

extern void read_program(struct analyser * a, unsigned localise_mask) {
    read_program_(a, -1);
    for (struct name * q = a->names; q; q = q->next) {
        // Declaring but not defining is only an error if used.  We'll issue
        // a warning later on if there are no errors.
        if (!q->used) continue;

        int error = false;
        switch (q->type) {
            case t_external: case t_routine:
                error = (q->definition == NULL);
                break;
            case t_grouping:
                error = (q->grouping == NULL);
                break;
        }
        if (error) {
            count_error(a);
            fprintf(stderr, "%s:%d: %s '%.*s' declared but not defined\n",
                    a->tokeniser->file, q->used->line_number,
                    name_of_type(q->type),
                    SIZE(q->s), q->s);
        }
    }

    // Skip name warning checks if there are errors.
    if (a->tokeniser->error_count)
        return;

    for (struct name * n = a->names; n; n = n->next) {
        if (n->type == t_external) {
            if (!n->used) {
                // Externals can be called from outside of Snowball, so if they
                // aren't already marked as used we set the `used` field to
                // point to the definition so we can just check this field
                // later.
                n->used = n->definition;
            }
            visit_routine(a, n);
        }
    }

    for (struct name * q = a->names; q; q = q->next) {
        if (q->references == 0) {
            fprintf(stderr, "%s:%d: warning: %s '%.*s' ",
                    a->tokeniser->file,
                    q->declaration_line_number,
                    name_of_type(q->type),
                    SIZE(q->s), q->s);
            if (q->type == t_routine ||
                q->type == t_external ||
                q->type == t_grouping) {
                fprintf(stderr, "declared but not defined\n");
            } else {
                fprintf(stderr, "declared but not used\n");
            }
            q->used = NULL;
            continue;
        }

        if (q->type == t_routine || q->type == t_grouping) {
            /* It's OK to define a grouping but only use it to define other
             * groupings.
             */
            if (!q->used && !q->used_in_definition) {
                int line_num;
                if (q->type == t_routine) {
                    line_num = q->definition->line_number;
                } else {
                    line_num = q->grouping->line_number;
                }
                fprintf(stderr, "%s:%d: warning: %s '%.*s' defined but not used\n",
                        a->tokeniser->file,
                        line_num,
                        name_of_type(q->type),
                        SIZE(q->s), q->s);
                continue;
            }
        }

        if (q->type < t_routine) {
            if (!q->initialised) {
                fprintf(stderr, "%s:%d: warning: %s '%.*s' is never initialised\n",
                        a->tokeniser->file,
                        q->declaration_line_number,
                        name_of_type(q->type),
                        SIZE(q->s), q->s);
            } else if (!q->value_used) {
                fprintf(stderr, "%s:%d: warning: %s '%.*s' is set but never used\n",
                        a->tokeniser->file,
                        q->declaration_line_number,
                        name_of_type(q->type),
                        SIZE(q->s), q->s);
                remove_dead_assignments(a->program, q);
                q->used = NULL;
                continue;
            }
        }

        if (q->count == -1) {
            // Used but use is not reachable by calling any externals so
            // suppress all code generation for this name.
            //
            // We only issue a warning about unreachability for routines here
            // to avoid excess diagnostics, since other types must be used in a
            // routine which is not reachable (or will have been warned about as
            // unused by the check above).
            if (q->type == t_routine) {
                fprintf(stderr, "%s:%d: warning: %s '%.*s' not reachable from any externals\n",
                        a->tokeniser->file,
                        q->declaration_line_number,
                        name_of_type(q->type),
                        SIZE(q->s), q->s);
                remove_unreachable_routine(a, q);
            }
            q->used = NULL;
        }
    }

    /* We've now identified variables whose values are never used and
     * names which are unreachable, and cleared "used" for them, so go
     * through and unlink the unused ones.
     */
    struct name * n = a->names;
    struct name ** n_ptr = &(a->names);
    while (n) {
        if (!n->used) {
            if (n->grouping) {
                // Clear the name field then loop through and remove from
                // the groupings list just below.
                n->grouping->name = NULL;
            } else if (n->definition) {
                remove_unreachable_routine(a, n);
            }
            struct name * n_next = n->next;
            lose_s(n->s);
            FREE(n);
            n = n_next;
            *n_ptr = n;
            continue;
        }
        n_ptr = &(n->next);
        n = n->next;
    }

    // Remove groupings which aren't used.
    struct grouping * g = a->groupings;
    struct grouping ** g_ptr = &(a->groupings);
    while (g) {
        if (!g->name) {
            struct grouping * g_next = g->next;
            lose_b(g->b);
            FREE(g);
            g = g_next;
            *g_ptr = g;
            continue;
        }
        g_ptr = &(g->next);
        g = g->next;
    }

    // Remove amongs which are in unreachable routines from the list
    // and number the others.
    {
        int among_count = 0;
        struct among ** a_ptr = &(a->amongs);
        while (*a_ptr) {
            struct among * x = *a_ptr;
            if (!x->used) {
                *a_ptr = x->next;
                continue;
            }

            x->number = among_count++;
            if (x->function_count > 0) ++a->among_with_function_count;

            for (int i = 1; i <= x->command_count; i++) {
                int merge_with = 0;
                struct node * command = x->commands[i - 1];
                assert(command->type == c_bra);
                if (!command->left || is_just_true(command->left)) {
                    // Optimisation has turned this action into a no-op.
                    command->left = NULL;
                    merge_with = -1;
                } else {
                    for (int k = 1; k < i; ++k) {
                        if (nodes_equivalent(command->left, x->commands[k - 1]->left)) {
                            // Optimisation has made this action equivalent
                            // to an earlier one.
                            merge_with = k;
                            break;
                        }
                    }
                }
                if (!merge_with) continue;

                // Update references to this command index to be `merge_with`
                // and subtract one from references to command indexes after
                // this one.
                for (int j = 0; j < x->literalstring_count; ++j) {
                    int diff = (x->b[j].result - i);
                    if (diff == 0) {
                        x->b[j].result = merge_with;
                        if (merge_with == 0) {
                            assert(x->b[j].action->type == c_bra);
                            x->b[j].action->left = NULL;
                        }
                    } else if (diff > 0) {
                        --x->b[j].result;
                    }
                }
                memmove(x->commands + (i - 1), x->commands + i,
                        sizeof(x->commands[0]) * (x->command_count - i));
                --x->command_count;
                ++x->nocommand_count;
                --i;
            }

            if (x->command_count > 1 ||
                (x->command_count == 1 && x->nocommand_count > 0)) {
                /* We need to set among_var rather than just checking if
                 * find_among*() returns zero or not.
                 */
                x->amongvar_needed = true;
                if (x->in_routine)
                    x->in_routine->amongvar_needed = true;
            }

            a_ptr = &(x->next);
        }
    }

    /* Localise variables.
     *
     * We localise variables which are only referenced in a single function
     * (routine or external) and which are always set before being read within
     * that function (since a function could rely on a variable's previous
     * value surviving).
     *
     * We could potentially localise variables referenced in multiple functions
     * provided that they are always set before use in every function they are
     * referenced in, and that these functions don't call one another, but that
     * situation doesn't occur in any of the stemmers we currently ship.
     */
    memset(a->name_count, 0, sizeof(a->name_count));
    for (struct name * name = a->names; name; name = name->next) {
        if (name->local_to != NULL) {
            if (localise_mask & (1 << name->type)) {
                struct node * func = name->local_to->definition;
                if (!always_set_before_use(func->left, func, name)) {
                    fprintf(stderr,
                            "%s:%d: info: Could not localise %s `%.*s` to routine `%.*s`\n",
                            a->tokeniser->file, func->line_number,
                            name_of_type(name->type),
                            SIZE(name->s), name->s,
                            SIZE(func->name->s), func->name->s);
                    report_s(stderr, name->s);
                    fprintf(stderr, "\n");
                    name->local_to = NULL;
                }
            } else {
                name->local_to = NULL;
            }
        }
        if (name->local_to == NULL) {
            name->count = a->name_count[name->type]++;
        }
    }
    a->variable_count = a->name_count[t_string] +
                        a->name_count[t_boolean] +
                        a->name_count[t_integer];
}

extern struct analyser * create_analyser(struct tokeniser * t) {
    NEW(analyser, a);
    *a = (struct analyser){0};
    a->tokeniser = t;
    a->mode = m_forward;
    a->modifyable = true;
    return a;
}

extern void close_analyser(struct analyser * a) {
    {
        struct node * q = a->nodes;
        while (q) {
            struct node * q_next = q->next;
            FREE(q);
            q = q_next;
        }
    }
    {
        struct name * q = a->names;
        while (q) {
            struct name * q_next = q->next;
            lose_s(q->s);
            FREE(q);
            q = q_next;
        }
    }
    {
        struct literalstring * q = a->literalstrings;
        while (q) {
            struct literalstring * q_next = q->next;
            lose_b(q->b); FREE(q);
            q = q_next;
        }
    }
    {
        struct among * q = a->amongs;
        while (q) {
            struct among * q_next = q->next;
            FREE(q->b);
            FREE(q->commands);
            FREE(q);
            q = q_next;
        }
    }
    {
        struct grouping * q = a->groupings;
        while (q) {
            struct grouping * q_next = q->next;
            lose_b(q->b); FREE(q);
            q = q_next;
        }
    }
    FREE(a);
}
