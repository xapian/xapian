
struct dutch_stemmer
{
    char * p;
    int p_size;
    int k;
    int j;
    struct pool * irregulars;
};

extern struct dutch_stemmer * setup_dutch_stemmer();

extern char * dutch_stem(struct dutch_stemmer * z, char * q, int i0, int i1);

extern void closedown_dutch_stemmer(struct dutch_stemmer * z);


/* To set up the dutch_stemming process:

       struct dutch_stemmer * z = setup_dutch_stemmer();

   to use it:

       char * p = dutch_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_dutch_stemmer(z);

*/

