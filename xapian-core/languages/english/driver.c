/* driver.c: Test harness for english stemming algorithm
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <stdio.h>
#include <ctype.h>  /* for isupper, islower, toupper, tolower */

#include "pool.h"
#include "stem_english.h"

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

void stemfile(void * z, char * s, FILE * f)
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

                   english_stem(z, s, 0, i-1)

                  );
        }
        else putchar(ch);
    }
}

/* typical usage,

   ./stemf a b c > d

   - files a, b, c are read, the words in them lower cased and stemmed, and sent
     to redirected stdout in d.

*/

int main(int argc, char * argv[])
{   int i;

    /* initialise the stemming process: */

    void * z = setup_english_stemmer();

    s = (char *) malloc(i_max+1);
    for (i = 1; i < argc; i++)
    {   FILE * f = fopen(argv[i], "r");
        if (f == 0) { fprintf(stderr, "File %s not found\n", argv[i]); exit(1); }
        stemfile(z, s, f);
    }
    free(s);

    /* closedown the stemming process */

    closedown_english_stemmer(z);

    return 0;
}

