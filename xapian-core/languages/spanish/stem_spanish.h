
struct spanish_stemmer
{
    char * p;
    int p_size;
    int k;

    int j;
    int posV;
    int pos2;
    struct pool * irregulars;

};

extern struct spanish_stemmer * setup_spanish_stemmer();

extern char * spanish_stem(struct spanish_stemmer * z, char * q, int i0, int i1);

extern void closedown_spanish_stemmer(struct spanish_stemmer * z);


/* To set up the stemming process:

       struct spanish_stemmer * z = setup_spanish_stemmer();

   to use it:

       char * p = spanish_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_spanish_stemmer(z);

*/
