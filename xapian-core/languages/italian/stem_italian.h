
struct italian_stemmer
{
    char * p;
    int p_size;
    int k;

    int j;
    int posV;
    int pos2;
    struct pool * irregulars;

};

extern struct italian_stemmer * setup_italian_stemmer();

extern char * italian_stem(struct italian_stemmer * z, char * q, int i0, int i1);

extern void closedown_italian_stemmer(struct italian_stemmer * z);


/* To set up the stemming process:

       struct italian_stemmer * z = setup_italian_stemmer();

   to use it:

       char * p = italian_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_italian_stemmer(z);

*/
