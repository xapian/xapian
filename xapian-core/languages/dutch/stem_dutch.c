/* stem_dutch.c: Dutch stemming algorithm.
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
#include <stdlib.h>
#include <string.h>

#include "pool.h"
#include "stem_dutch.h"

struct dutch_stemmer
{
    char * p;
    int p_size;
    int k;
    int j;
    struct pool * irregulars;
};

#define true 1
#define false 0

/* The main part of the stemming algorithm starts here. z->p is a buffer
   holding a word to be stemmed. The letters are in z->p[0], z->p[1] ...
   ending at z->p[z->k]. z->k is readjusted downwards as the stemming
   progresses. Zero termination is not in fact used in the algorithm.

   Note that only lower case sequences are stemmed. Forcing to lower case
   should be done before stem(...) is called.

   We will write p, k etc in place of z->p, z->k in the comments.
*/

/*
static int not_aeio(struct dutch_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o':
            return false;
        default:
            return true;
    }
}
*/



/* vowel(z, i) is true <=> p[i] is a one of 'a', 'e', 'i', 'o' or 'u'.
*/

static int vowel(struct dutch_stemmer * z, int i)
{   switch (z->p[i])
    {  case 'a': case 'e': case 'i': case 'o': case 'u':
           return true;
       default:
           return false;
    }
}

/* cons(z, i) is true <=> p[i] is a consonant.
*/

static int cons(struct dutch_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'o': case 'u':
            return false;

        case 'i':
            return (0 < i && i < z->k && vowel(z,i-1) && vowel(z,i+1));

        case 'j':
            if (i == 0) return true;
            if (i >= z->k) return false;
            return (z->p[i-1] != 'i' || vowel(z,i+1));

        /* so 'j' in the context 'ijs' 'ijk' is a vowel */

        case 'y': return (i == 0 || vowel(z,i-1));

        default:
            return true;
    }
}

/* m(z) measures the number of consonant sequences between 0 and j. if c is
   a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
   presence,

      <c><v>       gives 0
      <c>vc<v>     gives 1
      <c>vcvc<v>   gives 2
      <c>vcvcvc<v> gives 3
      ....
*/

static int m(struct dutch_stemmer * z)
{   int n = 0;
    int i = 0;
    if (z->j <= 1) return 0;
        /* Following the German stemmer: improves treatment of short words */
    while(true)
    {   if (i > z->j) return n;
        if (! cons(z, i)) break; i++;
    }
    i++;
    while(true)
    {   while(true)
        {   if (i > z->j) return n;
            if (cons(z, i)) break;
            i++;
        }
        i++;
        n++;
        while(true)
        {   if (i > z->j) return n;
            if (! cons(z, i)) break;
            i++;
        }
        i++;
    }
}

/* ends(z, s, length) is true <=> p[0], ... p[k] ends with the string s.
*/

static int ends(struct dutch_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k + 1) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

/* after ends(z, s1), context(z, s2) checks that s1 is preceded by s2
*/

static int context(struct dutch_stemmer * z, const char * s)
{   int keep_j = z->j;
    int keep_k = z->k;
    z->k = z->j;
    {   int result = ends(z,s);
        z->j = keep_j;
        z->k = keep_k;
        return result;
    }
}

static void undouble(struct dutch_stemmer * z)
{   int ch = z->p[z->k];
    switch (ch)
    {   case 'k': case 'd': case 't': /* the only doubles worth reducing */
            if (ch == z->p[z->k - 1]) z->k--;
    }
}

static int ends_e(struct dutch_stemmer * z)
{   if (ends(z,"e") && cons(z,z->j) && m(z) > 0)
    {   z->k = z->j; undouble(z);
        return true;
    }
    return false;
}

static int en_ending(struct dutch_stemmer * z)
{   if (cons(z,z->j) && m(z) > 0 &&
        ! context(z,"gem")) /* algemene etc */
    {   z->k = z->j; undouble(z);
        return true;
    }
    return false;
}

static int step_1(struct dutch_stemmer * z) /* result true if '-e' removed */
{   if ((ends(z,"en") || ends(z,"ene")) && en_ending(z)) return false;
    if (ends(z,"s") || ends(z,"se"))
    {   if (cons(z,z->j) && m(z) > 0) { z->k = z->j; return false; }
    }
    return ends_e(z);
}

static int chop(struct dutch_stemmer * z, const char * s)
{   if (ends(z,s) && m(z) > 1) { z->k = z->j; return true; }
    return false;
}

static int valid_i(struct dutch_stemmer * z) { return z->p[z->j] != 'e'; }
static int valid_h(struct dutch_stemmer * z) { return z->p[z->j] != 'c'; }

static int chop_test(struct dutch_stemmer * z, const char * s, int (*f)(struct dutch_stemmer *))
{   if (ends(z,s) && m(z) > 1 && f(z)) { z->k = z->j; return true; }
    return false;
}

static void step_2(struct dutch_stemmer * z, int e_removed)
{   switch (z->p[z->k])
    {   case 'd':
            if (chop(z,"end")) { chop_test(z,"ig",valid_i); return; }
            if (chop_test(z,"heid",valid_h))  /* not -scheid etc */
            {   if (chop(z,"lijk")) {  ends_e(z); return; }
                if (chop_test(z,"ig",valid_i)) return;
                if (chop(z,"end")) return;
                if (chop(z,"baar")) return;
                if (ends(z,"en") && en_ending(z)) return;
            }
            return;
        case 'g':
            if (chop(z,"ing"))
            {   if (chop_test(z,"ig",valid_i)) return;
                undouble(z);
            }
            chop_test(z,"ig",valid_i);
            return;
        case 'k':
            if (chop(z,"lijk")) { ends_e(z); return; }
            return;
        case 'r':
            if (chop(z,"baar")) return;
            if (e_removed && chop(z,"bar")) return;
            return;
    }
}

static int aeou(struct dutch_stemmer * z,int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'o': case 'u':
            return true;
        default: return false;
    }
}

static void step_3(struct dutch_stemmer * z)   /* undouble vowel in -cvvc context */
{   int k = z->k;
    if (k >= 3 &&
        z->p[k - 1] == z->p[k - 2] &&
        aeou(z,k-1) &&
        cons(z,k) &&
        cons(z,k-3))
    {   memmove(z->p + k - 2, z->p + k - 1, 2);
        z->k--;
    }
}

extern const char * dutch_stem(struct dutch_stemmer * z, const char * q, int i0, int i1)
{
    int p_size = z->p_size;

    if (i1 - i0 + 50 > p_size)
    {   free(z->p);
        p_size = i1 - i0 + 75; /* ample */ z->p_size = p_size;
        z->p = (char *) malloc(p_size);
    }

    /* strip out accents (i.e. discard umlauts) */
    {   int k = 0;
        int j; for (j = i0; j<=i1; j++)
        {   int ch = q[j];
            if (ch == '^' && i0 < j && j < i1) j++; else z->p[k++] = ch;
        }
        z->k = k - 1;
    }

    {   const char * t = search_pool(z->irregulars, z->k + 1, z->p);
        if (t != 0) return t;
    }

    step_2(z,step_1(z));
    step_3(z);

    z->p[z->k + 1] = 0; /* C string form for now */
    return z->p;
}

/*
    See the English stemmer for notes on the irregular forms.

    The list of Dutch irregularities is left blank at the moment.
*/

static const char * irregular_forms[] = {

    0, 0  /* terminator */

};

extern struct dutch_stemmer * setup_dutch_stemmer()
{   struct dutch_stemmer * z = (struct dutch_stemmer *) malloc(sizeof(struct dutch_stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return z;
}

extern void closedown_dutch_stemmer(struct dutch_stemmer * z)
{   free_pool(z->irregulars);
    free(z->p);
    free(z);
}


