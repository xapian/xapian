
struct stemmer
{
    char * p;
    int p_size;
    int k;
    int j;
    struct pool * irregulars;
};

extern struct stemmer * setup_stemmer();

extern char * stem(struct stemmer * z, char * q, int i0, int i1);

extern void closedown_stemmer(struct stemmer * z);


/* To set up the stemming process:

       struct stemmer * z = setup_stemmer();

   to use it:

       char * p = stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_stemmer(z);

*/
