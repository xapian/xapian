/* query.h: query functions for ferretfx
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#ifndef QUERY_H
#define QUERY_H

int set_probabilistic(const string&, const string&);
long do_match(long int, long int); /* Ol 1997-01-31 return msize */

void add_bterm(const string &);

extern void run_query(void);
extern void print_caption(long int);
extern void print_page_links(char, long int, long int);
extern void do_picker(char prefix, const char **opts);

extern char dec_sep, thou_sep;

extern matchop op;

extern string gif_dir;

extern string raw_prob;
extern long int msize;
extern map<docid, bool> ticked;
extern string query_string;
extern map<char, string> filter_map;
extern char *fmtstr;
extern string ad_keywords;
extern vector<termname> new_terms;
extern map<termname, int> matching_map;

typedef enum { NORMAL, PLUS, MINUS /*, BOOL_FILTER*/ } termtype;

extern void check_term(const string &name, termtype type);

#endif /* QUERY_H */
