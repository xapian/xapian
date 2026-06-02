struct token {
    byte code;        /* Token code (from enum token_code) */
    byte s_size;      /* Size of token */
    const char s[14]; /* Token */
};

/* List of alphabetical tokens and their corresponding codes (symbol tokens are
 * instead handled using `switch` in tokenise.c).
 *
 * Tokens below are ordered primarily by length, then by ASCII collating
 * order amongst tokens of the same length.
 */
static const struct token alpha_tokens[] = {
  { c_as,             2, "as" },
  { c_do,             2, "do" },
  { c_or,             2, "or" },
  { c_and,            3, "and" },
  { c_for,            3, "for" },
  { c_get,            3, "get" },
  { c_hex,            3, "hex" },
  { c_hop,            3, "hop" },
  { c_len,            3, "len" },
  { c_non,            3, "non" },
  { c_not,            3, "not" },
  { c_set,            3, "set" },
  { c_try,            3, "try" },
  { c_fail,           4, "fail" },
  { c_goto,           4, "goto" },
  { c_loop,           4, "loop" },
  { c_next,           4, "next" },
  { c_size,           4, "size" },
  { c_test,           4, "test" },
  { c_true,           4, "true" },
  { c_among,          5, "among" },
  { c_false,          5, "false" },
  { c_lenof,          5, "lenof" },
  { c_limit,          5, "limit" },
  { c_unset,          5, "unset" },
  { c_atmark,         6, "atmark" },
  { c_attach,         6, "attach" },
  { c_cursor,         6, "cursor" },
  { c_define,         6, "define" },
  { c_delete,         6, "delete" },
  { c_gopast,         6, "gopast" },
  { c_insert,         6, "insert" },
  { c_maxint,         6, "maxint" },
  { c_minint,         6, "minint" },
  { c_repeat,         6, "repeat" },
  { c_sizeof,         6, "sizeof" },
  { c_tomark,         6, "tomark" },
  { c_atleast,        7, "atleast" },
  { c_atlimit,        7, "atlimit" },
  { c_decimal,        7, "decimal" },
  { c_reverse,        7, "reverse" },
  { c_setmark,        7, "setmark" },
  { c_strings,        7, "strings" },
  { c_tolimit,        7, "tolimit" },
  { c_booleans,       8, "booleans" },
  { c_integers,       8, "integers" },
  { c_routines,       8, "routines" },
  { c_setlimit,       8, "setlimit" },
  { c_backwards,      9, "backwards" },
  { c_externals,      9, "externals" },
  { c_groupings,      9, "groupings" },
  { c_stringdef,      9, "stringdef" },
  { c_substring,      9, "substring" },
  { c_backwardmode,  12, "backwardmode" },
  { c_stringescapes, 13, "stringescapes" }
};

#define NUM_ALPHA_TOKENS ((int)(sizeof(alpha_tokens) / sizeof(alpha_tokens[0])))
