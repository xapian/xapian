/* stem_german.c: German stemming algorithm.
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
#include "stem_german.h"

struct german_stemmer
{
    char * p;
    int p_size;
    int k;
    int k0;
    int j;
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

static int not_aeio(struct german_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o':
            return false;
        default:
            return true;
    }
}

/* cons(z, i) is true <=> p[i] is a consonant.
*/

static int cons(struct german_stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o':
            return false;
        case 'u':
            if (i <= 0 || not_aeio(z, i - 1)) return false;
            if (i >= z->k || not_aeio(z, i + 1)) return false;
            return true;
        case 'y':
            return (i==0) || !cons(z, i - 1);
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

static int m(struct german_stemmer * z)
{   int n = 0;
    int i = 0;
    if (z->j <= 1) return 0;  /* This improves treatment of short words */
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

static int ends(struct german_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k + 1) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

/* starts(z, s) is true <=> p[0], ... p[k] starts with the string s.
*/

static int starts(struct german_stemmer * z, const char * s)
{   int length = strlen(s);
    if (length > z->k + 1) return false;
    if (memcmp(z->p + z->k0, s, length) != 0) return false;
    z->k0 += length;
    return true;
}

/* setto(z, s) sets p[j + 1] ... to the characters in the string s,
   readjusting k.
*/

/*
static void setto(struct german_stemmer * z, const char * s)
{   int length = strlen(s);
    memmove(z->p + z->j + 1, s, length);
    z->k = z->j + length;
}
*/



static int separable_prefix(struct german_stemmer * z)
{
    switch (PAIR(z->p[z->k0], z->p[z->k0 + 1]))
    {   default:
             return false;

        case PAIR('a', 'b'):
            return starts(z, "ab");

        case PAIR('a', 'n'):
            return starts(z, "an");

        case PAIR('a', 'u'):
            return starts(z, "auf") || starts(z, "auseinander") ||
                   starts(z, "aus");

        case PAIR('b', 'e'):
            return starts(z, "bei");

        case PAIR('d', 'a'):
            return starts(z, "dar") || starts(z, "davon") || starts(z, "da");

        case PAIR('d', 'u'):
            return starts(z, "durch");

        case PAIR('e', 'i'):
            return starts(z, "ein");

        case PAIR('e', 'm'):
            return starts(z, "empor");

        case PAIR('e', 'n'):
            return starts(z, "entgegen");

        case PAIR('f', 'e'):
            return starts(z, "fertig") || starts(z, "fest");

        case PAIR('f', 'o'):
            return starts(z, "fort");

        case PAIR('f', 'r'):
            return starts(z, "frei");

        case PAIR('h', 'e'):
            return starts(z, "heim") || starts(z, "herab") ||
                   starts(z, "heran") || starts(z, "herauf") ||
                   starts(z, "heraus") || starts(z, "herbei") ||
                   starts(z, "heruber") || starts(z, "herum") ||
                   starts(z, "herunter") || starts(z, "hervor") ||
                   starts(z, "her");

        case PAIR('h', 'i'):
            return starts(z, "hierher") || starts(z, "hier") ||
                   starts(z, "hinab") || starts(z, "hinauf") ||
                   starts(z, "hinaus") || starts(z,  "hinein");

        case PAIR('l', 'o'):
            return starts(z, "los");

        case PAIR('m', 'i'):
            return starts(z, "mit");

        case PAIR('n', 'a'):
            return starts(z, "nach");

        case PAIR('n', 'i'):
            return starts(z, "nieder");

        case PAIR('s', 'a'):
            return starts(z, "satt");

        case PAIR('t', 'e'):
            return starts(z, "teil");

        case PAIR('u', 'b'):
            return starts(z, "ubel");

        case PAIR('u', 'm'):
            return starts(z, "umher") || starts(z, "um");

        case PAIR('u', 'n'):
            return starts(z, "unter");

        case PAIR('v', 'o'):
            return starts(z, "voll") || starts(z, "voran") ||
                   starts(z, "voraus") || starts(z, "vorwarts") ||
                   starts(z, "vor");

        case PAIR('w', 'a'):
            return starts(z, "wahr");

        case PAIR('w', 'e'):
            return starts(z, "weg") || starts(z, "weiter");

        case PAIR('w', 'i'):
            return starts(z, "wiederher") || starts(z, "wieder");

        case PAIR('z', 'u'):
            return starts(z, "zurecht") || starts(z, "zuruck") ||
                   starts(z, "zusammen") || starts(z, "zu");

    }
}

static void step_0(struct german_stemmer * z)
{   z->k0 = 0;
    if (separable_prefix(z) && starts(z, "zu"))
    {   if (ends(z, "en") &&
            z->j - z->k0 > 1 && /* this is like the test in m(z) */
            m(z) > 0)
        {   memmove(z->p + z->k0 - 2, z->p + z->k0, z->k - z->k0 + 1);
                 /* remove 'zu' */
            z->k -= 2;
        }
    }
}

static int drop(struct german_stemmer * z, const char * s, int (*f)(struct german_stemmer *))
{   if (ends(z, s) && f(z) && m(z) > 0) {  z->k = z->j; return true; }
    return false;
}

static int ends_s(struct german_stemmer * z)
{   switch(z->p[z->j])
    {   case 'b': case 'd': case 'f': case 'g': case 'h': case 'k':
        case 'l': case 'm': case 'n': case 'r': case 't':
            return true;
        default: return false;
    }
}

/* the delicate test on the next line looks after 'ernst' 'kunst' etc */

static int ends_st(struct german_stemmer * z) { return ends_s(z) && z->p[z->j] != 'r' && z->j > 2; }
static int ends_i(struct german_stemmer * z) { return z->p[z->j] != 'e'; }
static int ends_e(struct german_stemmer * z) { return true; }

/*
static int ends_t(struct german_stemmer * z) { return ends_s(z); }
*/

static int drop_e(struct german_stemmer * z, const char * s) { return drop(z, s, ends_e); }
static int drop_s(struct german_stemmer * z, const char * s) { return drop(z, s, ends_s); }
static int drop_st(struct german_stemmer * z, const char * s) { return drop(z, s, ends_st); }
/*
static int drop_t(struct german_stemmer * z, const char * s) { return drop(z, s, ends_t); }
*/

static void step_1x(struct german_stemmer * z)
{   switch(z->p[z->k])
    {   case 'r':
            drop_e(z, "er"); return;
        case 't':
            if (drop_e(z, "est")) return;
            drop_st(z, "st"); return;
        case 'n':
            drop_e(z, "en"); return;
    }
}

static void step_1(struct german_stemmer * z)
{   switch(z->p[z->k])
    {   case 'm':
            drop_e(z, "em");
            break;
        case 'n':
            if (drop_e(z, "ern")) break;
            drop_e(z, "en");
            break;
        case 'r':
            drop_e(z, "er");
            break;
        case 's':
            if (drop_e(z, "es")) break;
            drop_s(z, "s");
            break;
        case 'e':
            drop_e(z, "e");
            break;
    }
    step_1x(z);
}

static void step_2x(struct german_stemmer * z)
{   switch(z->p[z->k])
    {   case 'r': drop_e(z, "er"); return;
        case 'n': drop_e(z, "en"); return;
    }
}

static int ends1(struct german_stemmer * z, const char * s)
{   if (ends(z, s) && m(z) > 1) {  z->k = z->j; return true; }
    return false;
}

static int ends1i(struct german_stemmer * z, const char * s)
{   if (ends(z, s) && m(z) > 1 && ends_i(z)) {  z->k = z->j; return true; }
    return false;
}

static void step_2(struct german_stemmer * z)
{   switch(z->p[z->k])
    {   case 'd':  if (ends1(z, "end")) { ends1i(z, "ig"); return; }
                   return;
        case 'g':  if (ends1(z, "ung")) { ends1i(z, "ig"); return; }
                   ends1i(z, "ig");
                   return;
        case 'h':  if (ends1(z, "lich")) { step_2x(z); return; }
                   ends1i(z, "isch");
                   return;
        case 'k':  ends1i(z, "ik");
                   return;
        case 't':  if (ends1(z, "keit"))
                   {   if (ends1(z, "lich")) return;
                       ends1(z, "ig"); return;
                   }
                   if (ends1(z, "heit")) { step_2x(z); return; }
        default: return;
    }
}




extern const char * german_stem(void * z_, const char * q, int i0, int i1)
{
    struct german_stemmer * z = (struct german_stemmer *) z_;
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

    step_0(z);
    step_1(z);
    step_2(z);

    z->p[z->k + 1] = 0; /* C string form for now */
    return z->p;
}

/*
    See the English stemmer for notes on the irregular forms.

    The list of German irregularities needs further development.
*/

static const char * irregular_forms[] = {

    "abendstern", "abendstern/",       /* Otherwise 'abendstern' -> 'abend' etc */
    "morgenstern", "morgenstern/",

    0, 0  /* terminator */

};

extern void * setup_german_stemmer()
{
    struct german_stemmer * z = (struct german_stemmer *) malloc(sizeof(struct german_stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return (void *) z;
}

extern void closedown_german_stemmer(void * z_)
{
    struct german_stemmer * z = (struct german_stemmer *) z_;
    free_pool(z->irregulars);
    free(z->p);
    free(z);
}

