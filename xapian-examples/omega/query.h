#ifndef QUERY_H
#define QUERY_H

int set_probabilistic( const char *, const char * );
int set_boolean( const char *query, const char *old_query, int boolean_present );
void set_relevant( const char * );
long do_match( long int, long int ); /* Ol 1997-01-31 return msize */

void add_bterm( const char * );
void do_showdoc( long int, long int, long int );

void muscat_stem( char * );

extern int dec_sep, thou_sep;

extern int percent_min;
#endif /* QUERY_H */
