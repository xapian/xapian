/* stem_danish.c: Danish stemming algorithm.
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
#include <ctype.h>

#include "pool.h"
#include "stem_danish.h"

struct danish_stemmer
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

   Extra letters

      form:                 o-slash   ae lig
   a) old representation    o^z        -
   b) HTML hex upper case   D8         C6
   c) HTML hex lower case   F8         E6
   d) representation here   'O'        'A'

   Soon Object Muscat will move towards a Unicode representation. Meanwhile,
   this algorithm translates forms (a), (b) and (c) to (d) on entry, and
   (d) back to (c) on exit. This happens at the place marked NOTE below.

   Open source downloaders may in the interim prefer to adjust all this code.

*/

/* cons(z, i) is true <=> p[i] is a consonant.
*/

static int cons(struct danish_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o':
        case 'y':
        case 'u': case 'A': case 'O':
            return false;
        default:
            return true;
    }
}

/* measure(z) establishes z->pos, the position of the minimum stem. The stemmer
   will not remove any characters at or before z->pos.
*/

static void measure(struct danish_stemmer * z)
{   int j = 0;
    int k = z->k;
    z->pos = k; /* default */
    if (k < 2) return;
    while (j < k && cons(z, j)) j++;  /* go to a vowel */
    while (j < k && ! cons(z, j)) j++; /* now go to a cons */
    if (j == 1) j = 2;
    z->pos = j;
}

/* ends(z, s) is true <=> p[0], ... p[k] ends with the string s after pos.
*/

static int ends(struct danish_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k - z->pos) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

static int not_among(int ch, const char * s)
{   int i = 0;
    while (true)
    {   int s_ch = s[i++];
        if (s_ch == 0) return true;
        if (s_ch == ch) return false;
    }
}

/* There is a pattern of endings -end, -ende, -ender, -endt, where the -end should not be removed,
   and a pattern -ende, -endes wheree it should. The confusion of the two types of -ende ending is
   tricky, and not fully soluble. Nevertheless -ende and -endes are removed. By contrast
   the similar problems with -ed and -ede are so severe these endings are not removed.
*/

static int step_1(struct danish_stemmer * z)
{   switch(z->p[z->k])
    {
        case 'd':
            if (ends(z, "ethed") ||
                ends(z, "hed") ||
                ends(z, "ered")) break;
            return false;
        case 'e':
            if (ends(z, "erede") ||
                ends(z, "erende") ||
                ends(z, "ende") ||
                ends(z, "ene") ||
                ends(z, "erne") ||
                ends(z, "ere") ||
                ends(z, "e")) break;
            return false;
        case 'n':
            if (ends(z, "heden") ||
                ends(z, "eren") ||
                ends(z, "en")) break;
            return false;
        case 'r':
            if (ends(z, "heder") ||
                ends(z, "erer") ||
                ends(z, "er")) break;
            return false;
        case 's':
            if (ends(z, "erendes") ||  /* very rare */
                ends(z, "endes") ||
                ends(z, "ernes") ||
                ends(z, "enes") ||
                ends(z, "eres") ||
                ends(z, "es") ||
                ends(z, "heds") ||
                ends(z, "hedens") ||
                ends(z, "erens") ||
                ends(z, "ens") ||
                ends(z, "ers") ||
                ends(z, "erets") ||    /* rare */
                ends(z, "ets") ||
                (ends(z, "s") && not_among(z->p[z->j], "eiuOs"))) break;
            return false;
        case 't':
            if (ends(z, "eret") ||
                ends(z, "et")) break;
            if (ends(z, "lOst"))
            {   z->k --; /* take off the "-t" */
                return true;
            }
            return false;
        default:
            return false;
    }
    z->k = z->j; return true;
}

static int double_cons(struct danish_stemmer * z)
{   return cons(z, z->k) && z->k > 2 && z->p[z->k] == z->p[z->k - 1]; /* double consonant */
}

static void step_2(struct danish_stemmer * z)
{   if (
        ends(z, "dt") ||
        ends(z, "gt") ||
        ends(z, "kt") ||
        ends(z, "gd") ||
        double_cons(z)

       ) z->k --;
}

static void step_3(struct danish_stemmer * z)
{
    if (ends(z, "igst")) z->k -= 2; /* take off -st */
    if (ends(z, "elig") ||
        ends(z, "els") ||
        ends(z, "lig")
       )
       {   z->k = z->j;
           step_2(z);
           return;
       }
    if ( ends(z, "ig") && z->p[z->j] != 'l')
       {   z->k = z->j;
           if (double_cons(z)) z->k --;
           return;
       }
}

extern const char * danish_stem(void * z_, const char * q, int i0, int i1)
{
    struct danish_stemmer * z = (struct danish_stemmer *) z_;

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
            {   case 0xC6: case 0xE6: ch = 'A'; break; /* ae lig */
                case 0xD8: case 0xF8: ch = 'O'; break; /* o^z */
            }
            if (ch == '^' && i0 < j && j < i1)
            {   int letter = q[j-1], accent = q[j+1];
                switch (PAIR(letter, accent))
                {
                    case PAIR('o', 'z'): p[k-1] = toupper(letter); break;
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
            {   case 'A': p[j] = 0xE6; break; /* ae lig */
                case 'O': p[j] = 0xF8; break; /* o^z */
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

    /* What follows demonstrates the idea: these examples are not
       necessarily correct */

    "afvigend",    "afvigende/",
    "afsend",      "afsende/",
    "anvend",      "anvende/anvendes/",
    "bekjend",     "bekjende/",
    "erkiend",     "erkiende/",
    "henvend",     "henvende/",

    0, 0  /* terminator */

};

extern void * setup_danish_stemmer()
{   struct danish_stemmer * z = (struct danish_stemmer *) malloc(sizeof(struct danish_stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return (void *) z;
}

extern void closedown_danish_stemmer(void * z_)
{
    struct danish_stemmer * z = (struct danish_stemmer *) z_;
    free_pool(z->irregulars);
    free(z->p);
    free(z);
}

