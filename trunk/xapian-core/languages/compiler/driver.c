
#include <stdio.h>   /* for main etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strlen */
#include "header.h"

static int eq(char * s1, char * s2) {
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);
    return s1_len == s2_len && memcmp(s1, s2, s1_len) == 0;
}

static void print_arglist(void) {
    fprintf(stderr, "Usage: snowball <file> [options]\n\n"
	            "options are: [-o[utput] file]\n"
                    "             [-s[yntax]]\n"
#ifndef DISABLE_JAVA
                    "             [-j[ava]]\n"
#endif
		    "             [-c++]\n"
                    "             [-w[idechars]]\n"
                    "             [-u[tf8]]\n"
                    "             [-n[ame] class name]\n"
                    "             [-ep[refix] string]\n"
                    "             [-vp[refix] string]\n"
                    "             [-i[nclude] directory]\n"
                    "             [-r[untime] path to runtime headers]\n"
		    "             [-p[arentclassname] parent class name]\n"
           );
    exit(1);
}

static void check_lim(int i, int argc) {
    if (i >= argc) {
        fprintf(stderr, "argument list is one short\n");
        print_arglist();
    }
}

static FILE * get_output(symbol * b) {
    char * s = b_to_s(b);
    FILE * output = fopen(s, "w");
    if (output == 0) {
        fprintf(stderr, "Can't open output %s\n", s);
        exit(1);
    }
    free(s);
    return output;
}

static void read_options(struct options * o, int argc, char * argv[]) {
    char * s;
    int i = 2;

    /* set defauts: */

    o->output_file = 0;
    o->syntax_tree = false;
    o->externals_prefix = "";
    o->variables_prefix = 0;
    o->runtime_path = 0;
    o->parent_class_name = 0;
    o->name = "";
    o->make_lang = LANG_C;
    o->widechars = false;
    o->includes = 0;
    o->includes_end = 0;
    o->utf8 = false;

    /* read options: */

    repeat {
        if (i >= argc) break;
        s = argv[i++];
        {   if (eq(s, "-o") || eq(s, "-output")) {
                check_lim(i, argc);
                o->output_file = argv[i++];
                continue;
            }
            if (eq(s, "-n") || eq(s, "-name")) {
                check_lim(i, argc);
                o->name = argv[i++];
                continue;
            }
#ifndef DISABLE_JAVA
            if (eq(s, "-j") || eq(s, "-java")) {
                o->make_lang = LANG_JAVA;
                o->widechars = true;
                continue;
            }
#endif
            if (eq(s, "-c++")) {
                o->make_lang = LANG_CPLUSPLUS;
                continue;
            }
            if (eq(s, "-w") || eq(s, "-widechars")) {
                o->widechars = true;
		o->utf8 = false;
                continue;
            }
            if (eq(s, "-s") || eq(s, "-syntax")) {
                o->syntax_tree = true;
                continue;
            }
            if (eq(s, "-ep") || eq(s, "-eprefix")) {
                check_lim(i, argc);
                o->externals_prefix = argv[i++];
                continue;
            }
            if (eq(s, "-vp") || eq(s, "-vprefix")) {
                check_lim(i, argc);
                o->variables_prefix = argv[i++];
                continue;
            }
            if (eq(s, "-i") || eq(s, "-include")) {
                check_lim(i, argc);

                {
                    NEW(include, p);
                    symbol * b = add_s_to_b(0, argv[i++]);
                    b = add_s_to_b(b, "/");
                    p->next = 0; p->b = b;

                    if (o->includes == 0) o->includes = p; else
                                          o->includes_end->next = p;
                    o->includes_end = p;
                }
                continue;
            }
            if (eq(s, "-r") || eq(s, "-runtime")) {
                check_lim(i, argc);
                o->runtime_path = argv[i++];
                continue;
            }
            if (eq(s, "-u") || eq(s, "-utf8")) {
                o->utf8 = true;
		o->widechars = false;
                continue;
            }
            if (eq(s, "-p") || eq(s, "-parentclassname")) {
                check_lim(i, argc);
                o->parent_class_name = argv[i++];
                continue;
            }
            fprintf(stderr, "'%s' misplaced\n", s);
            print_arglist();
        }
    }
}

extern int main(int argc, char * argv[]) {

    NEW(options, o);
    if (argc == 1) print_arglist();
    read_options(o, argc, argv);
    {
        symbol * filename = add_s_to_b(0, argv[1]);
        symbol * u = get_input(filename);
        if (u == 0) {
            fprintf(stderr, "Can't open input %s\n", argv[1]);
            exit(1);
        }
        {
            struct tokeniser * t = create_tokeniser(u);
            struct analyser * a = create_analyser(t);
            t->widechars = o->widechars;
            t->includes = o->includes;
            a->utf8 = t->utf8 = o->utf8;
            read_program(a);
            if (t->error_count > 0) exit(1);
            if (o->syntax_tree) print_program(a);
            close_tokeniser(t);
            unless (o->syntax_tree) {
                struct generator * g;

                char * s = o->output_file;
                unless (s) {
                    fprintf(stderr, "Please include the -o option\n");
                    print_arglist();
                    exit(1);
                }
                if (o->make_lang == LANG_C || o->make_lang == LANG_CPLUSPLUS) {
                    symbol * b = add_s_to_b(0, s);
		    b = add_s_to_b(b, ".h");
		    o->output_h = get_output(b);
		    b[SIZE(b) - 1] = 'c';
		    if (o->make_lang == LANG_CPLUSPLUS) {
			b = add_s_to_b(b, "c");
		    }
		    o->output_c = get_output(b);
                    lose_b(b);

                    g = create_generator_c(a, o);
                    generate_program_c(g);
                    close_generator_c(g);
                    fclose(o->output_c);
                    fclose(o->output_h);
                }
#ifndef DISABLE_JAVA
                if (o->make_lang == LANG_JAVA) {
                    symbol * b = add_s_to_b(0, s);
                    b = add_s_to_b(b, ".java");
                    o->output_java = get_output(b);
                    lose_b(b);
                    g = create_generator_java(a, o);
                    generate_program_java(g);
                    close_generator_java(g);
                    fclose(o->output_java);
                }
#endif
            }
            close_analyser(a);
        }
        lose_b(u);
        lose_b(filename);
    }
    {   struct include * p = o->includes;
        until (p == 0)
        {   struct include * q = p->next;
            lose_b(p->b); FREE(p); p = q;
        }
    }
    FREE(o);
    unless (space_count == 0) fprintf(stderr, "%d blocks unfreed\n", space_count);
    return 0;
}

