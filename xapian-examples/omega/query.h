#ifndef QUERY_H
#define QUERY_H

int set_probabilistic(const string&, const string&);
long do_match(long int, long int); /* Ol 1997-01-31 return msize */

void add_bterm(const string &);

extern char dec_sep, thou_sep;

extern matchop op;

#endif /* QUERY_H */
