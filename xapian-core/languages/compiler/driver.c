#include <ctype.h>   /* for toupper etc */
#include <stdio.h>   /* for fprintf etc */
#include <stdlib.h>  /* for free etc */
#include <string.h>  /* for strcmp */
#include "header.h"

#define DEFAULT_JAVA_PACKAGE "org.tartarus.snowball.ext"
#define DEFAULT_JAVA_BASE_CLASS "org.tartarus.snowball.SnowballProgram"
#define DEFAULT_JAVA_AMONG_CLASS "org.tartarus.snowball.Among"
#define DEFAULT_JAVA_STRING_CLASS "java.lang.StringBuilder"

#define DEFAULT_DART_BASE_CLASS "SnowballProgram"

#define DEFAULT_GO_PACKAGE "snowball"
#define DEFAULT_GO_SNOWBALL_RUNTIME "github.com/snowballstem/snowball/go"

#define DEFAULT_ADA_PACKAGE "Snowball"
#define DEFAULT_ADA_SNOWBALL_RUNTIME "github.com/snowballstem/snowball/ada"

#define DEFAULT_CS_NAMESPACE "Snowball"
#define DEFAULT_CS_BASE_CLASS "Stemmer"
#define DEFAULT_CS_AMONG_CLASS "Among"
#define DEFAULT_CS_STRING_CLASS "StringBuilder"

#define DEFAULT_CPLUSPLUS_NAMESPACE "Snowball"
#define DEFAULT_CPLUSPLUS_BASE_CLASS "Stemmer"

#define DEFAULT_JS_BASE_CLASS "BaseStemmer"

#define DEFAULT_PYTHON_BASE_CLASS "BaseStemmer"

static int eq(const char * s1, const char * s2) {
    return strcmp(s1, s2) == 0;
}

static void print_arglist(int exit_code) {
    FILE * f = exit_code ? stderr : stdout;
    fprintf(f, "Usage: snowball SOURCE_FILE... [OPTIONS]\n\n"
               "Supported options:\n"
               "  -o, -output OUTPUT_BASE\n"
               "  -s, -syntax                      show syntax tree and stop\n"
               "  -comments                        generate comments\n"
               "  -ada                             generate Ada\n"
               "  -c++                             generate C++\n"
               "  -cs, -csharp                     generate C#\n"
               "  -dart                            generate Dart\n"
               "  -go                              generate Go\n"
               "  -j, -java                        generate Java\n"
               "  -js                              generate Javascript\n"
               "  -pascal                          generate Pascal\n"
               "  -php                             generate PHP\n"
               "  -py, -python                     generate Python\n"
               "  -rust                            generate Rust\n"
               "  -w, -widechars\n"
               "  -u, -utf8\n"
               "  -n, -name CLASS_NAME\n"
               "  -ep, -eprefix EXTERNAL_PREFIX\n"
               "  -vp, -vprefix VARIABLE_PREFIX\n"
               "  -i, -include DIRECTORY\n"
               "  -r, -runtime DIRECTORY\n"
               "  -cheader                         header name to include from C/C++ file\n"
               "  -hheader                         header name to include from C/C++ header\n"
               "  -p, -parentclassname CLASS_NAME  fully qualified parent class name\n"
               "  -P, -Package PACKAGE_NAME        package name for stemmers\n"
               "  -S, -Stringclass STRING_CLASS    StringBuffer-compatible class\n"
               "  -a, -amongclass AMONG_CLASS      fully qualified name of the Among class\n"
               "  -gor, -goruntime PACKAGE_NAME    Go snowball runtime package\n"
               "  --help                           display this help and exit\n"
               "  --version                        output version information and exit\n"
           );
    exit(exit_code);
}

static void check_lim(int i, int argc) {
    if (i >= argc) {
        fprintf(stderr, "argument list is one short\n");
        print_arglist(1);
    }
}

static FILE * get_output(byte * s) {
    s[SIZE(s)] = 0;
    const char * filename = (const char *)s;
    FILE * output = fopen(filename, "w");
    if (output == NULL) {
        fprintf(stderr, "Can't open output %s\n", filename);
        exit(1);
    }
    return output;
}

static struct options * read_options(int * argc_ptr, char * argv[]) {
    int argc = *argc_ptr;
    int i = 1;
    int new_argc = 1;
    /* Note down the last option used to specify an explicit encoding so
     * we can warn we ignored it for languages with a fixed encoding.
     */
    const char * encoding_opt = NULL;

    NEW(options, o);
    *o = (struct options){0};

    // Set defaults which differ from empty initialisation.
    o->target_lang = LANG_C;
    o->encoding = ENC_SINGLEBYTE;

    /* read options: */

    while (i < argc) {
        char * s = argv[i++];
        if (s[0] != '-' || s[1] == '\0') {
            /* Non-option argument - shuffle down. */
            argv[new_argc++] = s;
            continue;
        }

        {
            if (eq(s, "-o") || eq(s, "-output")) {
                check_lim(i, argc);
                o->output_file = create_s_from_sz(argv[i++]);
                continue;
            }
            if (eq(s, "-n") || eq(s, "-name")) {
                check_lim(i, argc);
                o->name = create_s_from_sz(argv[i++]);
                continue;
            }
            if (eq(s, "-js")) {
                o->target_lang = LANG_JAVASCRIPT;
                continue;
            }
            if (eq(s, "-php")) {
                o->target_lang = LANG_PHP;
                continue;
            }
            if (eq(s, "-rust")) {
                o->target_lang = LANG_RUST;
                continue;
            }
            if (eq(s, "-go")) {
                o->target_lang = LANG_GO;
                continue;
            }
            if (eq(s, "-j") || eq(s, "-java")) {
                o->target_lang = LANG_JAVA;
                continue;
            }
            if (eq(s, "-dart")) {
                o->target_lang = LANG_DART;
                continue;
            }
            if (eq(s, "-cs") || eq(s, "-csharp")) {
                o->target_lang = LANG_CSHARP;
                continue;
            }
            if (eq(s, "-c++")) {
                o->target_lang = LANG_CPLUSPLUS;
                continue;
            }
            if (eq(s, "-pascal")) {
                o->target_lang = LANG_PASCAL;
                continue;
            }
            if (eq(s, "-py") || eq(s, "-python")) {
                o->target_lang = LANG_PYTHON;
                continue;
            }
            if (eq(s, "-ada")) {
                o->target_lang = LANG_ADA;
                continue;
            }
            if (eq(s, "-w") || eq(s, "-widechars")) {
                encoding_opt = s;
                o->encoding = ENC_WIDECHARS;
                continue;
            }
            if (eq(s, "-s") || eq(s, "-syntax")) {
                o->syntax_tree = true;
                continue;
            }
            if (eq(s, "-comments")) {
                o->comments = true;
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
            if (eq(s, "-cheader")) {
                check_lim(i, argc);
                o->cheader = argv[i++];
                continue;
            }
            if (eq(s, "-hheader")) {
                check_lim(i, argc);
                o->hheader = argv[i++];
                continue;
            }
            if (eq(s, "-i") || eq(s, "-include")) {
                check_lim(i, argc);

                {
                    NEW(include, p);
                    *p = (struct include){0};
                    byte * include_dir = add_sz_to_s(NULL, argv[i++]);
                    include_dir = add_char_to_s(include_dir, '/');
                    p->s = include_dir;

                    if (o->includes == NULL) {
                        o->includes = p;
                    } else {
                        o->includes_end->next = p;
                    }
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
                encoding_opt = s;
                o->encoding = ENC_UTF8;
                continue;
            }
            if (eq(s, "-p") || eq(s, "-parentclassname")) {
                check_lim(i, argc);
                o->parent_class_name = argv[i++];
                continue;
            }
            if (eq(s, "-P") || eq(s, "-Package")) {
                check_lim(i, argc);
                o->package = argv[i++];
                continue;
            }
            if (eq(s, "-S") || eq(s, "-stringclass")) {
                check_lim(i, argc);
                o->string_class = argv[i++];
                continue;
            }
            if (eq(s, "-a") || eq(s, "-amongclass")) {
                check_lim(i, argc);
                o->among_class = argv[i++];
                continue;
            }
            if (eq(s, "-gor") || eq(s, "-goruntime")) {
                check_lim(i, argc);
                o->go_snowball_runtime = argv[i++];
                continue;
            }
            if (eq(s, "--help")) {
                print_arglist(0);
            }

            if (eq(s, "--version")) {
                printf("Snowball compiler version " SNOWBALL_VERSION "\n");
                exit(0);
            }

            fprintf(stderr, "'%s' misplaced\n", s);
            print_arglist(1);
        }
    }
    if (new_argc == 1) {
        fprintf(stderr, "no source files specified\n");
        print_arglist(1);
    }
    argv[new_argc] = NULL;

    /* Set language-dependent defaults. */
    switch (o->target_lang) {
        case LANG_C:
            encoding_opt = NULL;
            break;
        case LANG_CPLUSPLUS:
            encoding_opt = NULL;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_CPLUSPLUS_BASE_CLASS;
            if (!o->package)
                o->package = DEFAULT_CPLUSPLUS_NAMESPACE;
            break;
        case LANG_CSHARP:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_CS_BASE_CLASS;
            if (!o->string_class)
                o->string_class = DEFAULT_CS_STRING_CLASS;
            if (!o->among_class)
                o->among_class = DEFAULT_CS_AMONG_CLASS;
            if (!o->package)
                o->package = DEFAULT_CS_NAMESPACE;
            break;
        case LANG_GO:
            o->encoding = ENC_UTF8;
            if (!o->package)
                o->package = DEFAULT_GO_PACKAGE;
            if (!o->go_snowball_runtime)
                o->go_snowball_runtime = DEFAULT_GO_SNOWBALL_RUNTIME;
            break;
        case LANG_ADA:
            o->encoding = ENC_UTF8;
            if (!o->package)
                o->package = DEFAULT_ADA_PACKAGE;
            break;
        case LANG_JAVA:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_JAVA_BASE_CLASS;
            if (!o->string_class)
                o->string_class = DEFAULT_JAVA_STRING_CLASS;
            if (!o->among_class)
                o->among_class = DEFAULT_JAVA_AMONG_CLASS;
            if (!o->package)
                o->package = DEFAULT_JAVA_PACKAGE;
            break;
        case LANG_DART:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_DART_BASE_CLASS;
            break;
        case LANG_JAVASCRIPT:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_JS_BASE_CLASS;
            break;
        case LANG_PHP:
            o->encoding = ENC_UTF8;
            break;
        case LANG_PYTHON:
            o->encoding = ENC_WIDECHARS;
            if (!o->parent_class_name)
                o->parent_class_name = DEFAULT_PYTHON_BASE_CLASS;
            break;
        case LANG_RUST:
            o->encoding = ENC_UTF8;
            break;
        default:
            break;
    }

    if (encoding_opt) {
        fprintf(stderr, "warning: %s only meaningful for C and C++\n",
                encoding_opt);
    }

    if (o->target_lang != LANG_C && o->target_lang != LANG_CPLUSPLUS) {
        if (o->runtime_path) {
            fprintf(stderr, "warning: -r/-runtime only meaningful for C and C++\n");
        }
        if (o->externals_prefix) {
            fprintf(stderr, "warning: -ep/-eprefix only meaningful for C and C++\n");
        }
        if (o->variables_prefix) {
            fprintf(stderr, "warning: -vp/-vprefix only meaningful for C and C++\n");
        }
    }

    // Split any extension off o->output_file and set o->output_leaf to just
    // its leafname (which e.g. is used to generate `#include "english.h"` in
    // path/to/english.c).
    if (!o->output_file) {
        // Default output uses the basename from the first Snowball source.
        // E.g. algorithms/english.sbl -> english
        const char * first_source = argv[1];
        const char * slash = strrchr(first_source, '/');
        const char * leaf = (slash == NULL) ? first_source : slash + 1;

        slash = strrchr(leaf, '\\');
        if (slash != NULL) leaf = slash + 1;

        const char * dot = strrchr(leaf, '.');
        if (dot) {
            o->extension = create_s_from_sz(dot);
            o->output_file = create_s_from_data(leaf, dot - leaf);
        } else {
            o->output_file = create_s_from_sz(leaf);
        }
        o->output_leaf = copy_s(o->output_file);
    } else {
        // Remove any extension from o->output_file so `-o path/to/english.c`
        // works.
        o->output_file[SIZE(o->output_file)] = '\0';
        const char * output_file = (const char *)o->output_file;
        const char * slash = strrchr(output_file, '/');
        const char * leaf = (slash == NULL) ? output_file : slash + 1;

        slash = strrchr(leaf, '\\');
        if (slash != NULL) leaf = slash + 1;

        const char * dot = strrchr(leaf, '.');
        if (dot) {
            o->extension = create_s_from_sz(dot);
            SET_SIZE(o->output_file, dot - output_file);
            o->output_leaf = create_s_from_data(leaf, dot - leaf);
        } else {
            o->output_leaf = create_s_from_sz(leaf);
        }
    }

    if (!o->name) {
        o->name = copy_s(o->output_leaf);
        const byte * dot = memchr(o->name, '.', SIZE(o->name));
        if (dot) {
            // Trim off any extension (we only remove the last of multiple
            // extensions above).
            SET_SIZE(o->name, dot - o->name);
        }
        switch (o->target_lang) {
            case LANG_CSHARP:
            case LANG_PASCAL:
                /* Upper case initial letter. */
                o->name[0] = toupper(o->name[0]);
                break;
            case LANG_CPLUSPLUS:
            case LANG_JAVASCRIPT:
            case LANG_PHP:
            case LANG_PYTHON: {
                /* Upper case initial letter and change each
                 * underscore+letter or hyphen+letter to an upper case
                 * letter.
                 */
                size_t len = SIZE(o->name);
                size_t new_len = 0;
                int uc_next = true;
                for (size_t j = 0; j != len; ++j) {
                    byte ch = o->name[j];
                    if (ch == '_' || ch == '-') {
                        uc_next = true;
                    } else {
                        if (uc_next) {
                            o->name[new_len] = toupper(ch);
                            uc_next = false;
                        } else {
                            o->name[new_len] = ch;
                        }
                        ++new_len;
                    }
                }
                SET_SIZE(o->name, new_len);
                break;
            }
            default:
                /* Just use as-is, e.g. that's the Java convention. */
                break;
        }
    }

    *argc_ptr = new_argc;
    return o;
}

extern int main(int argc, char * argv[]) {
    int i;
    struct options * o = read_options(&argc, argv);
    {
        char * file = argv[1];
        byte * u = get_input(file);
        if (u == NULL) {
            fprintf(stderr, "Can't open input %s\n", file);
            exit(1);
        }
        {
            struct tokeniser * t = create_tokeniser(u, file);
            struct analyser * a = create_analyser(t);
            struct input ** next_input_ptr = &(t->next);
            unsigned localise_mask = 0;
            a->encoding = t->encoding = o->encoding;
            t->includes = o->includes;
            /* If multiple source files are specified, set up the others to be
             * read after the first in order, using the same mechanism as
             * 'get' uses. */
            for (i = 2; i != argc; ++i) {
                NEW(input, q);
                *q = (struct input){0};
                file = argv[i];
                u = get_input(file);
                if (u == NULL) {
                    fprintf(stderr, "Can't open input %s\n", file);
                    exit(1);
                }
                q->p = u;
                q->file = file;
                q->line_number = 1;
                *next_input_ptr = q;
                next_input_ptr = &(q->next);
            }
            *next_input_ptr = NULL;

            /* Whether it's helpful to try to localise string variables varies
             * greatly between target languages.  One reason for this is likely
             * to be that strings are immutable in some languages (e.g. Dart,
             * Javascript, Python) so each string operation creates a new
             * string anyway.
             *
             * We've attempted to benchmark most languages to decide.
             *
             * One potential gotcha here is for garbage collected languages,
             * where our benchmark might not trigger GC and in that case our
             * timing is missing the cost of that, which any long running
             * indexing process will eventually incur.
             *
             * We've mostly used the following artificial benchmark which
             * exercises a local string variable to test this:
             *
             *   strings ( s )
             *   routines ( r )
             *   externals ( stem )
             *   define r as (-> s s)
             *   define stem as ( next [tolimit] loop 100000000 do r )
             *
             * Replace e.g. english.sbl with this and build the stemwords
             * equivalent for the target language, then:
             *
             * $ echo nonalphabetisations|time ./stemwords
             *
             * The appropriate number of iterations to use varies, and is
             * annotated below.
             */
            switch (o->target_lang) {
                case LANG_ADA:
                    // 1000000000: local 13.7s vs global 5.2s
                case LANG_C:
                    // We lack a way generate lose_s(v) on every `return` from
                    // the function, but manually adjusting the generated code
                    // to do this gives:
                    //
                    // 1000000000: local 44.9s vs global 6.3s
                case LANG_CPLUSPLUS:
                    // String variables are handled the same as LANG_C.
                case LANG_CSHARP:
                    // 100000000: local 18.8s vs global 12.4s
                case LANG_JAVA:
                    // 1000000000: local 10.1s vs global 7.1s
                case LANG_RUST:
                    // 1000000000: localising was slightly slower.
                    localise_mask = (1 << t_boolean) | (1 << t_integer);
                    break;
                case LANG_DART:
                    // Not timed, but strings are immutable so seems likely
                    // to be helpful to localise.
                case LANG_GO:
                    // 1000000000: localising was about 10% faster.
                case LANG_PASCAL:
                    // Slightly faster.
                case LANG_PHP:
                    // Slightly faster.
                case LANG_PYTHON:
                    // 10000000: local 7.6s vs global 7.9s.  Microbenchmarking
                    // with timeit alligns with this.
                case LANG_JAVASCRIPT:
                    // 10000000: Slightly faster.
                    localise_mask = (1 << t_boolean) | (1 << t_integer) | (1 << t_string);
                    break;
            }
            read_program(a, localise_mask);
            if (t->error_count > 0) exit(1);
            if (o->syntax_tree) print_program(a);
            if (!o->syntax_tree) {
                struct generator * g = create_generator(a, o);
                switch (o->target_lang) {
                    case LANG_C:
                    case LANG_CPLUSPLUS: {
                        byte * s = copy_s(o->output_file);
                        s = add_literal_to_s(s, ".h");
                        o->output_h = get_output(s);
                        SET_SIZE(s, SIZE(o->output_file));
                        if (o->extension &&
                            !(SIZE(o->extension) == 2 && memcmp(o->extension, ".h", 2) == 0)) {
                            s = add_s_to_s(s, o->extension);
                        } else if (o->target_lang == LANG_CPLUSPLUS) {
                            s = add_literal_to_s(s, ".cc");
                        } else {
                            s = add_literal_to_s(s, ".c");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);

                        generate_program_c(g);
                        fclose(o->output_src);
                        fclose(o->output_h);
                        break;
                    }
#ifndef TARGET_C_ONLY
                    case LANG_ADA: {
                        byte * s = copy_s(o->output_file);
                        s = add_literal_to_s(s, ".ads");
                        o->output_h = get_output(s);
                        SET_SIZE(s, SIZE(o->output_file));
                        if (o->extension &&
                            !(SIZE(o->extension) == 4 && memcmp(o->extension, ".ads", 2) == 0)) {
                            s = add_s_to_s(s, o->extension);
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".adb");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);

                        generate_program_ada(g);
                        fclose(o->output_src);
                        fclose(o->output_h);
                        break;
                    }
                    case LANG_CSHARP: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".cs");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_csharp(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_DART: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".dart");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_dart(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_GO: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".go");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_go(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_JAVA: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".java");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_java(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_JAVASCRIPT: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".js");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_js(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_PASCAL: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".pas");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_pascal(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_PHP: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".php");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_php(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_PYTHON: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".py");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_python(g);
                        fclose(o->output_src);
                        break;
                    }
                    case LANG_RUST: {
                        byte * s = copy_s(o->output_file);
                        if (o->extension) {
                            s = add_s_to_s(s, o->extension);
                        } else {
                            s = add_literal_to_s(s, ".rs");
                        }
                        o->output_src = get_output(s);
                        lose_s(s);
                        generate_program_rust(g);
                        fclose(o->output_src);
                        break;
                    }
#else
                    default:
                        fprintf(stderr, "Support for requested target language not enabled\n");
                        exit(1);
#endif
                }
                close_generator(g);
            }
            close_tokeniser(t);
            close_analyser(a);
        }
        lose_s(u);
    }
    {   struct include * p = o->includes;
        while (p) {
            struct include * q = p->next;
            lose_s(p->s);
            FREE(p);
            p = q;
        }
    }
    lose_s(o->extension);
    lose_s(o->name);
    lose_s(o->output_file);
    lose_s(o->output_leaf);
    FREE(o);
    if (space_count) fprintf(stderr, "%d blocks unfreed\n", space_count);
    return 0;
}
