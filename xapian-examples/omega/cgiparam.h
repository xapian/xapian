/* functions to deal with CGI parameters
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

/* decode the query from stdin as "NAME=VALUE" pairs */
extern void decode_test( void );

/* decode the query as a POST */
extern void decode_post( void );

/* decode the query as a GET */
extern void decode_get( void );

extern multimap<string, string> cgi_params;
