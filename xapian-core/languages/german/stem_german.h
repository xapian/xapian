
struct german_stemmer
{
    char * p;
    int p_size;
    int k;
    int k0;
    int j;
    struct pool * irregulars;
};

extern struct german_stemmer * setup_german_stemmer();

extern char * german_stem(struct german_stemmer * z, char * q, int i0, int i1);

extern void closedown_german_stemmer(struct german_stemmer * z);


/* To set up the german_stemming process:

       struct german_stemmer * z = setup_german_stemmer();

   to use it:

       char * p = german_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_german_stemmer(z);

*/

