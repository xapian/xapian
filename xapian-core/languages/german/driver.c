
#include <stdio.h>
#include <ctype.h>  /* for isupper, islower, toupper, tolower */

#include "pool.h"
#include "stemg.h"

#define true 1
#define false 0

static char * s;         /* s is a buffer holding the word to be stemmed */

#define INC 50           /* size units in which s is increased */
static int i_max = INC;  /* maximum offset in s */

#define LETTER(ch)  (islower(ch) || (ch) == '^')

/* increase_s() increases s if it not big enough, returning a new value for s
*/

void increase_s()
{   i_max += INC;
    {   char * new_s = (char *) malloc(i_max+1);
        { int i; for (i = 0; i < i_max; i++) new_s[i] = s[i]; } /* copy across */
        free(s); s = new_s;
    }
}

/* stemfile(z, s, f) stems file f to stdout, using s as a word buffer and z
   as stem structure
*/

void stemfile(struct german_stemmer * z, char * s, FILE * f)
{   while(true)
    {   int ch = getc(f);
        if (ch == EOF) return;
        if (LETTER(ch))
        {   int i = 0;
            while(true)
            {   if (i == i_max) increase_s();

                /* force lower case: */
                if isupper(ch) ch = tolower(ch);

                s[i] = ch; i++;
                ch = getc(f);
                if (!LETTER(ch)) { ungetc(ch, f); break; }
            }
            printf("%s",

            /* Now call the stemmer. The word to be stemmed is in s[0] to
               s[i-1] inclusive. The result is a zero terminated string in
               z.
            */

                   german_stem(z, s, 0, i-1)

                  );
        }
        else putchar(ch);
    }
}

/* typical usage,

   ./stemg a b c > d

   - files a, b, c are read, the words in them lower cased and stemmed, and sent
     to redirected stdout in d.

*/

int main(int argc, char * argv[])
{   int i;

    /* initialise the stemming process: */

    struct german_stemmer * z = setup_german_stemmer();


    printf("version 1: ");
    s = (char *) malloc(i_max+1);
    for (i = 1; i < argc; i++)
    {   FILE * f = fopen(argv[i], "r");
        if (f == 0) { fprintf(stderr, "File %s not found\n", argv[i]); exit(1); }
        stemfile(z, s, f);
    }
    free(s);

    /* closedown the stemming process */

    closedown_german_stemmer(z);

    return 0;
}

