
struct french_stemmer;

extern struct french_stemmer * setup_french_stemmer();

extern char * french_stem(struct french_stemmer * z, char * q, int i0, int i1);

extern void closedown_french_stemmer(struct french_stemmer * z);


/* To set up the stemming process:

       struct french_stemmer * z = setup_french_stemmer();

   to use it:

       char * p = french_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_french_stemmer(z);

*/
