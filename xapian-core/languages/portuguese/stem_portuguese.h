
struct portuguese_stemmer
{
    char * p;
    int p_size;
    int k;

    int j;
    int posV;
    int pos2;
    struct pool * irregulars;

};

extern struct portuguese_stemmer * setup_portuguese_stemmer();

extern char * portuguese_stem(struct portuguese_stemmer * z, char * q, int i0, int i1);

extern void closedown_portuguese_stemmer(struct portuguese_stemmer * z);


/* To set up the stemming process:

       struct portuguese_stemmer * z = setup_portuguese_stemmer();

   to use it:

       char * p = portuguese_stem(z, q, i0, i1);

   The word to be stemmed is in byte address q offsets i0 to i1
   inclusive (i.e. from q[i0] to q[i1]). The stemmed result is the
   C string at address p.

   To close down the stemming process:

   closedown_portuguese_stemmer(z);

*/
