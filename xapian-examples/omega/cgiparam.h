/* functions to deal with CGI parameters */

/* decode the query from stdin as "NAME=VALUE" pairs */
extern void decode_test( void );

/* decode the query as a POST */
extern void decode_post( void );

/* decode the query as a GET */
extern void decode_get( void );

/* Get an entry with a given name */
/* If there are multiple entries, you might get any, but then qsort will have
 * already jumbled them for you */
extern char *GetEntry( char *pName );

/* Get *first* entry with a given name, ready to iterate if required */
/* NB first could be any as qsort will have jumbled them */
extern char *FirstEntry( char *pName, int *p_which );

/* Get the next entry with a given name */
extern char *NextEntry( char *pName, int *p_which );

/* Safer versions of stdlib */
char *xstrdup(const char *string);
void *xmalloc(size_t size);
