/* stem_norwegian.c: Norwegian stemming algorithm.
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
#include "stem_norwegian.h"

struct norwegian_stemmer
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

   Nowegian has three extra letters, a-ring, ae ligature, and o-slash.

   See the comments in the Swedish and Danish stemmers at this point.
   To summarise:

   Norwegian :   a-ring    ae lig    o-slash
   Danish    :      -      ae lig    o-slash
   Swedish   :   a-ring      -         -       a-umlaut   o-uumlaut

*/

/* cons(z, i) is true <=> p[i] is a consonant.
*/

static int cons(struct norwegian_stemmer * z, int i)
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

static void measure(struct norwegian_stemmer * z)
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

static int ends(struct norwegian_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k - z->pos /*+ 1*/) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

/* In Norwegian, -ade, -ende endings are also removed, whereas this was not
   in Swedish, where it seemed to lead to too many false removals.
*/

static void step_1(struct norwegian_stemmer * z)
{   switch(z->p[z->k])
    {   case 'a':
            if (
                ends(z, "a")
               ) break;
            return;
        case 'e':
            if (ends(z, "erte")) z->k -= 2; else /* -erte -> -er */
            if (
                ends(z, "ede") ||
                ends(z, "ende") ||
                ends(z, "ande") ||
                ends(z, "ane") ||
                ends(z, "hetene") ||
                ends(z, "ene") ||
                ends(z, "e")
               ) break;
            return;
        case 'n':
            if (
                ends(z, "heten") ||
                ends(z, "en")
               ) break;
            return;
        case 'r':
            if (
                ends(z, "heter") ||
                ends(z, "er") ||
                ends(z, "ar")
               ) break;
            return;
        case 's':
            if (
                ends(z, "as") ||
                ends(z, "edes") ||
                ends(z, "endes") ||
                ends(z, "hetenes") ||  /* rare */
                ends(z, "enes") ||
                ends(z, "es") ||
                ends(z, "hetens") ||
                ends(z, "ens") ||
                ends(z, "ers") ||
                ends(z, "ets") ||
                ends(z, "s") && ((cons(z, z->j) && z->p[z->j] != 's') ||
                                 z->p[z->j] == 'o')
               ) break;
            return;
        case 't':
            if (
                ends(z, "het") ||
                ends(z, "et") ||
                ends(z, "ast")
               ) break;
            if (ends(z, "ert")) z->k--;  /* -ert -> -er */
            return;
        default:
            return;
    }
    z->k = z->j;
}

static void step_2(struct norwegian_stemmer * z)
{   if (
        ends(z, "dt") ||
        ends(z, "vt")
       ) z->k --;
}

static void step_3(struct norwegian_stemmer * z)
{   if (
        ends(z, "eleg") ||    /* variant of -elig- */
        ends(z, "leg") ||     /* variant of -lig- */
        ends(z, "elig") ||
        ends(z, "lig") ||
        ends(z, "eig") ||
        ends(z, "ig") ||
        ends(z, "elov") ||
        ends(z, "hetslov") || /* Rare. -elseslov- is possible, but very rare */
        ends(z, "slov") ||
        ends(z, "lov") ||
        ends(z, "els")
       ) z->k = z->j;
}

extern const char * norwegian_stem(void * z_, const char * q, int i0, int i1)
{
    struct norwegian_stemmer * z = (struct norwegian_stemmer *) z_;

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
            {
                case 0xC6: case 0xE6: ch = 'A'; break; /* ae lig */
                case 0xC5: case 0xE5: ch = 'B'; break; /* a^o */
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
            {   case 'A': p[j] = '\xE6'; break; /* ae lig */
                case 'B': p[j] = '\xE5'; break; /* a^o */
                case 'O': p[j] = '\xF8'; break; /* o^z */
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

    "utred",  "utrede/utredes/",
    "utsted", "utstede/utstedes/",    /* for example ... */

    "anvend",  "anvende/anvendes/",
    "avhend",  "avhende/avhendes/",
    "innvend", "innvende/innvendes/",
    "omvend",  "omvende/omvendes/",   /* perhaps ... */



    0, 0  /* terminator */

};

extern void * setup_norwegian_stemmer()
{   struct norwegian_stemmer * z = (struct norwegian_stemmer *) malloc(sizeof(struct norwegian_stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return (void *) z;
}

extern void closedown_norwegian_stemmer(void * z_)
{
    struct norwegian_stemmer * z = (struct norwegian_stemmer *) z_;
    free_pool(z->irregulars);
    free(z->p);
    free(z);
}

