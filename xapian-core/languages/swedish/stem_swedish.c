/* stem_swedish.c: Swedish stemming algorithm.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
#include "stem_swedish.h"

struct swedish_stemmer
{
    char * p;
    int p_size;
    int k;
    int k0;
    int j;
    int pos;
    struct pool * irregulars;
};

#define true 1
#define false 0

#define PAIR(a, b)   ((a)<<8|(b))

/* The main part of the stemming algorithm starts here. z->p is a buffer
   holding a word to be stemmed. The letters are in z->p[0], z->p[1] ...
   ending at z->p[z->k]. z->k is readjusted downwards as the stemming
   progresses. Zero termination is not in fact used in the algorithm.

   Note that only lower case sequences are stemmed. Forcing to lower case
   should be done before stem(...) is called.

   We will write p, k etc in place of z->p, z->k in the comments.
*/

/* NOTE

   Swedish is cursed with three extra letters, a-umlaut, a-ring, and o-umlaut.
   In old Muscat these are represented a^u, a^o and a^u. In HTML documents
   they are typically represented as hex C4, C5 and D6 (upper case), and
   E4, E5 and F6 (lower case). In this algorithm the are represented by the
   letters A, B and O. So we have this table:

      form:                 u-umlaut   a-ring   o-umlaut
   a) old representation    a^u        a^o      o^u
   b) HTML hex upper case   C4         C5       D6
   c) HTML hex lower case   E4         E5       F6
   d) representation here   'A'        'B'      'O'

   Soon Object Muscat will move towards a Unicode representation. Meanwhile,
   this algorithm translates forms (a), (b) and (c) to (d) on entry, and
   (d) back to (c) on exit. This happens at the place marked NOTE below.

   Open source downloaders may in the interim prefer to adjust all this code.

*/

/* cons(z, i) is true <=> p[i] is a consonant.
*/

static int cons(struct swedish_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o':
        case 'y':
        case 'u': case 'A': case 'B': case 'O':
            return false;
        default:
            return true;
    }
}

/* measure(z) establishes z->pos, the position of the minimum stem. The stemmer
   will not remove any characters at or before z->pos.
*/

static void measure(struct swedish_stemmer * z)
{   int j = 0;
    int k = z->k;
    z->pos = k; /* default */
    if (k < 2) return;
    while (j < k && cons(z, j)) j++;  /* go to a vowel */
    while (j < k && ! cons(z, j)) j++; /* now go to a cons */
    if (j == 1) j = 2;
    z->pos = j;
}

/* ends(z, s) is true <=> p[0], ... p[k] ends with the string s.
*/

static int ends(struct swedish_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k - z->pos /*+ 1*/) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

static void step_1(struct swedish_stemmer * z)
{   switch(z->p[z->k])
    {   case 'a':
            if (ends(z, "arna") ||
                ends(z, "heterna") ||
                ends(z, "erna") ||
                ends(z, "orna") ||
                ends(z, "a")) break;
            return;
        case 'd':
            if (ends(z, "ad")) break;
            return;
        case 'e':
            if (ends(z, "ade") ||
                ends(z, "ande") ||
                ends(z, "arne") ||
                ends(z, "are") ||
                ends(z, "aste") ||
                ends(z, "e")) break;
            return;
        case 'n':
            if (ends(z, "anden") ||
                ends(z, "aren") ||
                ends(z, "heten") ||
                ends(z, "ern") ||
                ends(z, "en")) break;
            return;
        case 'r':
            if (ends(z, "heter") ||
                ends(z, "ar") ||
                ends(z, "er") ||
                ends(z, "or")) break;
            return;
        case 's':
            if (ends(z, "arnas") ||
                ends(z, "ernas") ||
                ends(z, "ornas") ||
                ends(z, "ades") ||
                ends(z, "andes") ||
                ends(z, "arens") ||
                ends(z, "hetens") ||
                ends(z, "ens") ||
                ends(z, "erns") ||
                ends(z, "as") ||
                ends(z, "es") ||
                ends(z, "s")) break;
            return;
        case 't':
            if (ends(z, "het") ||
                ends(z, "andet") ||
                ends(z, "ast") ||
                ends(z, "at")) break;
            return;
        default:
            return;
    }
    z->k = z->j;
}

static void step_2(struct swedish_stemmer * z)
{   if (ends(z, "tt") ||
        ends(z, "dt") ||
        ends(z, "gt") ||
        ends(z, "kt") ||
        ends(z, "gd") ||
        ends(z, "dd") ||
        ends(z, "nn")) z->k --;
}

static void step_3(struct swedish_stemmer * z)
{   if (ends(z, "lig") ||
        ends(z, "ig") ||
        ends(z, "els")) z->k = z->j;
}

extern const char * swedish_stem(void * z_, const char * q, int i0, int i1)
{
    struct swedish_stemmer * z = (struct swedish_stemmer *) z_;

    int p_size = z->p_size;

    if (i1 - i0 + 50 > p_size)
    {   free(z->p);
        p_size = i1 - i0 + 75; /* ample */ z->p_size = p_size;
        z->p = (char *) malloc(p_size);
    }

    /* strip out accents - see the NOTE above */
    {   char * p = z->p;
        int k = 0;
        int j; for (j = i0; j <= i1; j++)
        {   int ch = q[j];
            switch (ch & 0xFF)
            {   case 0xC4: case 0xE4: ch = 'A'; break; /* a^u */
                case 0xC5: case 0xE5: ch = 'B'; break; /* a^o */
                case 0xD6: case 0xF6: ch = 'O'; break; /* o^u */
            }
            if (ch == '^' && i0 < j && j < i1)
            {   int letter = q[j-1], accent = q[j+1];
                switch (PAIR(letter, accent))
                {   case PAIR('a', 'u'):
                    case PAIR('o', 'u'): p[k-1] = toupper(letter); break;
                    case PAIR('a', 'o'): p[k-1] = 'B'; break;
                } j++;
            } else p[k++] = ch;

        }
        z->k = k - 1;
    }

    {   const char * t = search_pool(z->irregulars, z->k + 1, z->p);
        if (t != 0) return t;
    }

    if (z->k > 1)
    {   measure(z);
        step_1(z);
        step_2(z);
        step_3(z);
    }

    /* restore accents */
    {   char * p = z->p;
        int j; for (j = 0; j <= z->k; j++)
        {
            switch (p[j])
            {   case 'A': p[j] = 0xE4; break; /* a^u */
                case 'B': p[j] = 0xE5; break; /* a^o */
                case 'O': p[j] = 0xF6; break; /* o^u */
            }
        }
    }

    z->p[z->k + 1] = 0; /* C string form for now */
    return z->p;
}

/*
    See the English stemmer for notes on the irregular forms.
*/

static const char * irregular_forms[] = {

    /* fill in along the lines of the other stemmers */

    0, 0  /* terminator */

};

extern void * setup_swedish_stemmer()
{   struct swedish_stemmer * z = (struct swedish_stemmer *) malloc(sizeof(struct swedish_stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return (void *) z;
}

extern void closedown_swedish_stemmer(void * z_)
{
    struct swedish_stemmer * z = (struct swedish_stemmer *) z_;
    free_pool(z->irregulars);
    free(z->p);
    free(z);
}

