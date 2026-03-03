#include <stdio.h>

#define SNOWBALL_VERSION "3.0.0"

typedef unsigned char byte;
typedef unsigned short symbol;

#define true 1
#define false 0

#define MALLOC check_malloc
#define FREE check_free

// Declare variable `V` of type `struct TYPE *` and dynamically allocate it.
// We exit on allocation failure so `V` is always non-NULL.
#define NEW(TYPE, V) struct TYPE * V = (struct TYPE *) MALLOC(sizeof(struct TYPE))

// Similar to NEW() but allocates an array of N objects of type `struct TYPE *`.
#define NEWVEC(TYPE, V, N) struct TYPE * V = (struct TYPE *) MALLOC(sizeof(struct TYPE) * (N))

#define SIZE(p)            ((const int *)(p))[-1]
#define SET_SIZE(p, n)     ((int *)(p))[-1] = (n)
#define ADD_TO_SIZE(p, n)  ((int *)(p))[-1] += (n)
#define CAPACITY(p)        ((int *)(p))[-2]

extern symbol * create_b(int n);
extern void report_b(FILE * out, const symbol * p);
extern void lose_b(symbol * p);
extern symbol * increase_capacity_b(symbol * p, int n);
extern symbol * add_to_b(symbol * p, const symbol * q, int n);
extern symbol * copy_b(const symbol * p);
extern char * b_to_sz(const symbol * p);
extern symbol * add_symbol_to_b(symbol * p, symbol ch);

// These routines are like those above but work in byte instead of symbol.

extern byte * create_s(int n);
extern byte * create_s_from_sz(const char * s);
extern byte * create_s_from_data(const char * s, int n);

extern void report_s(FILE * out, const byte * p);
extern void lose_s(byte * p);
extern byte * increase_capacity_s(byte * p, int n);
extern byte * ensure_capacity_s(byte * p, int n);
extern byte * copy_s(const byte * p);
extern byte * add_s_to_s(byte * p, const byte * s);
extern byte * add_slen_to_s(byte * p, const char * s, int n);
extern byte * add_sz_to_s(byte * p, const char * s);
extern byte * add_char_to_s(byte * p, char ch);
// "" LIT is a trick to make compilation fail if LIT is not a string literal.
#define add_literal_to_s(P, LIT) add_slen_to_s(P, "" LIT, sizeof(LIT) - 1)

struct str; /* defined in space.c */

extern struct str * str_new(void);
extern void str_delete(struct str * str);
extern void str_append(struct str * str, const struct str * add);
extern void str_append_ch(struct str * str, char add);
extern void str_append_s(struct str * str, const byte * q);
extern void str_append_string(struct str * str, const char * s);
extern void str_append_int(struct str * str, int i);
extern void str_append_wchar_as_utf8(struct str * str, symbol ch);
extern void str_clear(struct str * str);
extern void str_assign(struct str * str, const char * s);
extern struct str * str_copy(const struct str * old);
extern byte * str_data(const struct str * str);
extern int str_len(const struct str * str);
extern int str_back(const struct str *str);
extern void str_pop(const struct str *str);
extern void str_pop_n(const struct str *str, int n);
extern void output_str(FILE * outfile, struct str * str);

extern int get_utf8(const symbol * p, int * slot);
extern int put_utf8(int ch, symbol * p);

typedef enum { ENC_SINGLEBYTE = 0, ENC_UTF8, ENC_WIDECHARS } enc;

/* stringdef name and value */
struct m_pair {
    struct m_pair * next;
    byte * name;
    symbol * value;

};

/* struct input must be a prefix of struct tokeniser. */
struct input {
    struct input * next;
    byte * p;
    int c;
    char * file;
    // -1 : Release file with: lose_s((byte *)file)
    //  0 : We don't own file.
    //  1 : Release file with: free(file)
    int file_owned;
    int line_number;

};

struct include {
    struct include * next;
    byte * s;

};

enum token_codes {
    /* The relational operator token values are chosen such that we can
     * invert the relation with a simple xor with 1.
     */
    c_gt = 0, c_le,
    c_ge, c_lt,
    c_eq, c_ne,

    /* Other token values just need to be unique. */
    c_among, c_and, c_as, c_assign, c_assignto, c_atleast,
    c_atlimit, c_atmark, c_attach, c_backwardmode, c_backwards,
    c_booleans, c_bra, c_comment1, c_comment2, c_cursor, c_debug,
    c_decimal, c_define, c_delete, c_divide, c_divideassign, c_do,
    c_dollar, c_externals, c_fail, c_false, c_for, c_get,
    c_gopast, c_goto, c_groupings, c_hex, c_hop, c_insert,
    c_integers, c_ket, c_leftslice, c_len, c_lenof, c_limit, c_loop,
    c_maxint, c_minint, c_minus, c_minusassign, c_multiply,
    c_multiplyassign, c_next, c_non, c_not, c_or, c_plus,
    c_plusassign, c_repeat, c_reverse, c_rightslice, c_routines,
    c_set, c_setlimit, c_setmark, c_size, c_sizeof, c_slicefrom,
    c_sliceto, c_stringdef, c_stringescapes, c_strings, c_substring,
    c_test, c_tolimit, c_tomark, c_true, c_try, c_unset,

    /* These token values don't directly correspond to a keyword. */
    c_name,
    c_number,
    c_literalstring,

    /* These token values are synthesised by the analyser. */
    c_mathassign,
    c_neg,
    c_call,
    c_grouping,
    c_booltest,
    c_functionend,
    c_goto_grouping,
    c_gopast_grouping,
    c_goto_non,
    c_gopast_non,
    c_not_booltest,

    NUM_TOKEN_CODES
};

enum uplus_modes {
    UPLUS_NONE = 0,
    UPLUS_DEFINED,
    UPLUS_UNICODE
};

/* struct input must be a prefix of struct tokeniser. */
struct tokeniser {
    struct input * next;
    byte * p;
    int c;
    char * file;
    // -1 : Release file with: lose_s((byte *)file)
    //  0 : We don't own file.
    //  1 : Release file with: free(file)
    int file_owned;
    int line_number;

    // Used for c_literalstring values.
    symbol * b;
    // Used for c_name names.
    byte * s;
    int number;
    // String escape start character or -1.
    int m_start;
    // String escape end character.
    int m_end;
    // Link list of stringdefs.
    struct m_pair * m_pairs;
    // Nesting depth of get directives.
    int get_depth;
    int error_count;
    int token;
    int previous_token;
    byte token_held;
    byte token_reported_as_unexpected;
    enc encoding;

    struct include * includes;

    /* Mode in which U+ has been used:
     * UPLUS_NONE - not used yet
     * UPLUS_DEFINED - stringdef U+xxxx ....
     * UPLUS_UNICODE - {U+xxxx} used with implicit meaning
     */
    int uplusmode;

    char token_disabled[NUM_TOKEN_CODES];
};

extern byte * get_input(const char * filename);
extern struct tokeniser * create_tokeniser(byte * b, char * file);
extern int read_token(struct tokeniser * t);
extern int peek_token(struct tokeniser * t);
#define hold_token(T) ((T)->token_held = true)
extern const char * name_of_token(int code);
extern void disable_token(struct tokeniser * t, int code);
extern void close_tokeniser(struct tokeniser * t);

extern int space_count;
extern void * check_malloc(size_t n);
extern void check_free(void * p);

struct node;

struct name {
    struct name * next;
    byte * s;
    byte type;                  /* t_string etc */
    byte mode;                  /* for routines, externals (m_forward, etc) */
    byte value_used;            /* (For variables) is its value ever used? */
    byte initialised;           /* (For variables) is it ever initialised? */
    byte used_in_definition;    /* (grouping) used in grouping definition? */
    byte amongvar_needed;       /* for routines, externals */
    byte among_with_function;   /* (routines/externals) contains among with func */
    struct node * definition;   /* (routines/externals) c_define node */
    int used_in_among;          /* (routines/externals) Count of uses in amongs */
    // Initialised to -1; set to -2 if reachable from an external.
    // Reachable names are then numbered 0, 1, 2, ... with separate numbering
    // per type.
    int count;
    // Number of references to this name.
    int references;
    struct grouping * grouping; /* for grouping names */
    struct node * used;         /* First use, or NULL if not used */
    struct name * local_to;     /* Local to one routine/external */
    int among_index;            /* for functions used in among */
    int declaration_line_number;/* Line number of declaration */
};

struct literalstring {
    struct literalstring * next;
    symbol * b;
};

struct amongvec {
    symbol * b;      /* the string giving the case */
    int size;        /* - and its size */
    struct node * action; /* the corresponding action */
    int i;           /* the amongvec index of the longest substring of b */
    int result;      /* the numeric result for the case */
    int line_number; /* for diagnostics and stable sorting */
    int function_index; /* 1-based */
    struct name * function;
};

struct among {
    struct among * next;
    struct amongvec * b;      /* pointer to the amongvec */
    int number;               /* amongs are numbered 0, 1, 2 ... */
    int literalstring_count;  /* in this among */
    int command_count;        /* in this among (excludes "no command" entries) */
    int nocommand_count;      /* number of "no command" entries in this among */
    int function_count;       /* number of different functions in this among */
    byte amongvar_needed;     /* do we need to set among_var? */
    byte always_matches;      /* will this among always match? */
    byte used;                /* is this among in reachable code? */
    int shortest_size;        /* smallest non-zero string length in this among */
    int longest_size;         /* longest string length in this among */
    struct node * substring;  /* i.e. substring ... among ( ... ) */
    struct node ** commands;  /* array with command_count entries */
    struct node * node;       /* pointer to the node for this among */
    struct name * in_routine; /* pointer to name for routine this among is in */
};

struct grouping {
    struct grouping * next;
    symbol * b;               /* the characters of this group */
    int largest_ch;           /* character with max code */
    int smallest_ch;          /* character with min code */
    struct name * name;       /* so g->name->grouping == g */
    int line_number;
};

struct node {
    struct node * next;
    struct node * left;
    struct node * aux;     /* used in setlimit */
    struct among * among;  /* used in among */
    struct node * right;
    byte type;
    byte mode;
    // We want to distinguish constant AEs which have the same value everywhere
    // (e.g. 42, 2+2, lenof '{U+0246}') from constant AEs which can have a
    // different value depending on platform and/or target language and/or
    // Unicode mode (e.g. maxint, sizeof '{U+0246}') - some warnings which
    // depend on a constant AEs value should only fire for the first set.
    byte fixed_constant;
    // Return 0 for always f.
    // Return 1 for always t.
    // Return -1 for don't know (or can raise t or f).
    signed char possible_signals;
    struct node * AE;
    struct name * name;
    symbol * literalstring;
    int number;
    int line_number;
};

enum name_types {
    t_size = 6,

    t_string = 0, t_boolean = 1, t_integer = 2, t_routine = 3, t_external = 4,
    t_grouping = 5

/*  If this list is extended, adjust write_varname in generator.c  */
};

struct analyser {
    struct tokeniser * tokeniser;
    struct node * nodes;
    struct name * names;
    struct literalstring * literalstrings;
    byte mode;
    byte modifyable;          /* false inside reverse(...) */
    struct node * program;
    struct node * program_end;
    /* name_count[i] counts the number of names of type i, where i is an enum
     * name_types value.  These counts *EXCLUDE* localised variables and
     * variables which optimised away (e.g. declared but never used).
     */
    int name_count[t_size];
    /* name_count[t_string] + name_count[t_boolean] + name_count[t_integer] */
    int variable_count;
    struct among * amongs;
    struct among * amongs_end;
    int among_with_function_count; /* number of amongs with functions */
    struct grouping * groupings;
    struct grouping * groupings_end;
    struct node * substring;  /* pending 'substring' in current routine definition */
    struct name * current_routine; /* routine/external we're currently on. */
    enc encoding;
    byte int_limits_used;     /* are maxint or minint used? */
    byte debug_used;          /* is the '?' command used? */
};

enum analyser_modes {
    // m_unknown is used as the initial value for struct node's mode member.
    // When a routine (or external) is used or defined we check the mode
    // member matches, but for the first use/definition we see we want to
    // instead set it to the mode of that use/definition.
    m_forward = 0, m_backward, m_unknown
};

extern void print_program(struct analyser * a);
extern struct analyser * create_analyser(struct tokeniser * t);
extern void close_analyser(struct analyser * a);

/** Read and analyse the program.
 *
 *  @param localise_mask  bitmask of variable types the generator can localise
 */
extern void read_program(struct analyser * a, unsigned localise_mask);

struct generator {
    struct analyser * analyser;
    struct options * options;
    int unreachable;           /* 0 if code can be reached, 1 if current code
                                * is unreachable. */
    int var_number;            /* Number of next variable to use. */
    struct str * outbuf;       /* temporary str to store output */
    struct str * declarations; /* str storing variable declarations */
    int next_label;
    int max_label;             /* Only used by Python */
    int margin;

    /* Target language code to execute in case of failure. */
    struct str * failure_str;

    int label_used;      /* Keep track of whether the failure label is used. */
    int failure_label;
    int debug_count;
    int copy_from_count; /* count of calls to copy_from() */

    const char * S[10];  /* strings */
    byte * B[10];        /* byte blocks */
    int I[10];           /* integers */

    int line_count;      /* counts number of lines output */
    int line_labelled;   /* in ISO C, will need extra ';' if it is a block end */
    int literalstring_count;
    int keep_count;      /* used to number keep/restore pairs to avoid compiler warnings
                            about shadowed variables */
    int temporary_used;  /* track if temporary variable used (Ada and Pascal) */
    char java_import_arrays; /* need `import java.util.Arrays;` */
    char java_import_chararraysequence; /* need `import org.tartarus.snowball.CharArraySequence;` */
};

/* Special values for failure_label in struct generator. */
enum special_labels {
    x_return = -1
};

struct options {
    /* for the command line: */
    byte * output_file;
    // output_file but without any path.
    byte * output_leaf;
    // Extension specified in -o option (or NULL if none).
    byte * extension;
    byte * name;
    FILE * output_src;
    FILE * output_h;
    byte syntax_tree;
    byte comments;
    enc encoding;
    enum {
        LANG_C = 0, // We generate C by default.
        LANG_ADA,
        LANG_CPLUSPLUS,
        LANG_CSHARP,
        LANG_DART,
        LANG_GO,
        LANG_JAVA,
        LANG_JAVASCRIPT,
        LANG_PASCAL,
        LANG_PHP,
        LANG_PYTHON,
        LANG_RUST
    } target_lang;
    const char * externals_prefix;
    const char * variables_prefix;
    const char * cheader;
    const char * hheader;
    const char * runtime_path;
    const char * parent_class_name;
    const char * package;
    const char * go_snowball_runtime;
    const char * string_class;
    const char * among_class;
    struct include * includes;
    struct include * includes_end;
};

/* Generator functions common to several backends. */

extern struct generator * create_generator(struct analyser * a, struct options * o);
extern void close_generator(struct generator * g);

extern void write_char(struct generator * g, int ch);
extern void write_newline(struct generator * g);
extern void write_string(struct generator * g, const char * s);
extern void write_wchar_as_utf8(struct generator * g, symbol ch);
extern void write_int(struct generator * g, int i);
extern void write_hex4(struct generator * g, int ch);
extern void write_symbol(struct generator * g, symbol s);
extern void write_s(struct generator * g, const byte * b);
extern void write_str(struct generator * g, struct str * str);
extern void write_c_relop(struct generator * g, int relop);

extern void write_comment_content(struct generator * g, struct node * p,
                                  const char * end);
extern void write_generated_comment_content(struct generator * g);
extern void write_start_comment(struct generator * g,
                                const char * comment_start,
                                const char * comment_end);

extern int K_needed(struct generator * g, struct node * p);
extern int repeat_restore(struct generator * g, struct node * p);

extern int just_return_on_fail(struct generator * g);
extern int tailcallable(struct generator * g, struct node * p);

/* Generator for C code. */
extern void generate_program_c(struct generator * g);

/* Generator for Java code. */
extern void generate_program_java(struct generator * g);

/* Generator for Dart code. */
extern void generate_program_dart(struct generator * g);

/* Generator for C# code. */
extern void generate_program_csharp(struct generator * g);

extern void generate_program_pascal(struct generator * g);

extern void generate_program_php(struct generator * g);

/* Generator for Python code. */
extern void generate_program_python(struct generator * g);

extern void generate_program_js(struct generator * g);

extern void generate_program_rust(struct generator * g);

extern void generate_program_go(struct generator * g);

extern void generate_program_ada(struct generator * g);
