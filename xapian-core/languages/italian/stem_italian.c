/* stem_italian.c: Italian stemming algorithm.
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
#include <ctype.h>  /* for isupper, islower, toupper, tolower */

#include "pool.h"
#include "stem_italian.h"

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

#define true 1
#define false 0

#define CON 0
#define VOW 1

/*-----
    void debug(struct italian_stemmer * z, int n)
    {   int i;
        printf("[%d:", n);
        {   int i;
            for (i = 0; i < z->k; i++)
            {   printf("%C", z->p[i]);
                if (i == z->posV || i == z->pos2) printf("/");
            }
        }
        printf("] ");
    }
----*/

/* char_type(z, i) is CON or VOW according as the character at i is a consonant
   or vowel. Used in measure().
*/

static int vowel(int ch)
{   switch (ch)
    {   case 'a': case 'e': case 'i': case 'o': case 'u':
        case 'A': case 'E': case 'I': case 'O':
            return true;
        default: return false;
    }
}


static int char_type(struct italian_stemmer * z, int i)
{   char * p = z->p;
    switch (p[i])
    {   case 'a': case 'e': case 'o':
        case 'A': case 'E': case 'I': case 'O':
            return VOW;
        case 'u':
            if (0 < i && p[i-1] == 'q') return CON;
            /* drop through to next case */
        case 'i':
            if (0 < i &&
                i < z->k - 1 &&
                vowel(p[i-1]) &&
                vowel(p[i+1])) return CON;
            return VOW;
        default: return CON;
    }
}

/* measure(z) establishes two critical positions in the word, posV and pos2.
   A suffix will only be removed if it leaves behind a stem which is either
   posV or pos2 characters long. posV is used for verbs, pos2 for other forms.

   If the word begins with 2 or more VOWs posV is at the first CON, if it
   begins VOW-CON posV is at the second VOW, if it begins CON posV is at the
   first VOW or the 3rd letter, whichever is further to the right.

   pos2 is at the end of the CON sequence which follows the second VOW sequence.

   posV and pos2 are set to k if the word is too short for these definitions to
   be meaningful.

   Here are some examples ('V' '2' indicate posV and pos2):

           abandonarlo             augusto
             | |      |              |  | |
             V 2      k              V  2 k

           basAndose               proposiciOn
             |  |   |                |  |     |
             V  2   k                V  2     k

           bmw
              |    here posV = pos2 = k
              k
*/

static void measure(struct italian_stemmer * z)
{   int j = 0;
    int k = z->k;
    z->posV = k; /* default */
    if (k < 2) return;
    while (j < k && char_type(z, j) == VOW) j++;
    if (j >= 2) z->posV = j; else
    {
        /* now go to a vowel */
        while (!(j == k || char_type(z, j) == VOW)) j++;
        if (j == 1) j++;  /* to 3rd letter */
        z->posV = j;
    }
    j = 0;
    {   int i; for (i = 0;; i++)
        {   while (j < k && char_type(z, j) == CON) j++;
            if (i == 2) { z->pos2 = j-1; break; }
            while (j < k && char_type(z, j) == VOW) j++;
        }
    }
}

/* look_for(z, s) tests that the characters ..., p[j-1], p[j] contain string
   s. Within s, digits are interpreted as certain classes of vowel, so '0'
   stands for 'a' or 'e' or 'o' or 'i'. The complete scheme is:

       0 = aeoi       5 = aui
       1 = ae         6 = aeiAEI (i.e. aei accented or unaccented)
       2 = ei         7 = ao
       3 = oi         8 = aoi
       4 = aei

   The result is true/false, and if true, j is decreased by the length of s.
*/

static int look_for(struct italian_stemmer * z, char * s)
{   char * p = z->p;
    int length = strlen(s);
    int jbase = z->j-length+1;
    if (jbase < 0) return false;
    {   int i; for (i = 0; i < length; i++)
        {   int ch_s = s[i];
            int ch = p[jbase+i];
            switch (ch_s)
            {   case '0':
                    if (ch != 'a' && ch != 'e' && ch != 'o' && ch != 'i') return false;
                    break;
                case '1':
                    if (ch != 'a' && ch != 'e') return false;
                    break;
                case '2':
                    if (ch != 'e' && ch != 'i') return false;
                    break;
                case '3':
                    if (ch != 'o' && ch != 'i') return false;
                    break;
                case '4':
                    if (ch != 'a' && ch != 'e' && ch != 'i') return false;
                    break;
                case '5':
                    if (ch != 'a' && ch != 'u' && ch != 'i') return false;
                    break;
                case '6':
                    if (ch != 'a' && ch != 'e' && ch != 'i' &&
                        ch != 'A' && ch != 'E' && ch != 'I') return false;
                    break;
                case '7':
                    if (ch != 'a' && ch != 'o') return false;
                    break;
                case '8':
                    if (ch != 'a' && ch != 'o' && ch != 'i') return false;
                    break;
                default:
                    if (ch != ch_s) return false;
            }
        }
    }
    z->j -= length;
    return true;
}

/* ends(z, s) applies look_for(z, s) at the end of the word, i.e. with j = k-1. */

static int ends(struct italian_stemmer * z, char * s)
{  z->j = z->k - 1;
   return look_for(z, s);
}

/* setto(z, s) sets (j+1), ... k to the characters in the string s, readjusting
   k.
*/

static void setto(struct italian_stemmer * z, char * s)
{   int length = strlen(s);
    memmove(z->p+z->j+1, s, length);
    z->k = z->j+length+1;
}

/* attachV(z, s) attaches s to the so long as j is not before posV,
   after_posV(z) tests that j is not before posV,
   chopV(z, s) prepares to remove suffix s if it is after posV,

   and similarly,

   attach2(z, s) attaches s to the so long as j is not before pos2,
   after_pos2(z) tests that j is not before pos2,
   chop2(z, s) prepares to remove suffix s if it is after pos2.

*/

static int attachV(struct italian_stemmer * z, char * s)
{   if (z->j < z->posV) return false;
    setto(z, s);
    return true;
}

static int after_posV(struct italian_stemmer * z)
{   if (z->j < z->posV) return false;
    z->k = z->j+1;
    return true;
}

static int chopV(struct italian_stemmer * z, char * s) { return ends(z, s) && after_posV(z); }

static int attach2(struct italian_stemmer * z, char * s)
{   if (z->j < z->pos2) return false;
    setto(z, s);
    return true;
}

static int after_pos2(struct italian_stemmer * z)
{   if (z->j < z->pos2) return false;
    z->k = z->j+1;
    return true;
}

static int chop2(struct italian_stemmer * z, char * s) { return ends(z, s) && after_pos2(z); }

/*

*/

static int attached_pronoun(struct italian_stemmer * z)
{   char * p = z->p;
    int b = z->k;
    switch (p[b-2])
    {   case 'c':
            if (!(ends(z, "ci"))) return false;
            break;
        case 'm':
            if (!(ends(z, "mi"))) return false;
            break;
        case 't':
            if (!(ends(z, "ti"))) return false;
            break;
        case 'l':
            if (!(ends(z, "gli") || ends(z, "l0"))) return false;
            break;
        case 'v':
            if (!(ends(z, "vi"))) return false;
            break;
        case 'n':
            if (!(ends(z, "sene") || ends(z, "ne"))) return false;
            break;
        case 's':
            if (!(ends(z, "si"))) return false;
            break;
        default:
            return false;
    }
    z->k = z->j + 1;

        if (ends(z, "1ndo")) { if (z->j >= z->posV) return true; } else

        if (ends(z, "r"))
        {   int ch;
            z->k = z->k + 1;      /* a bit messy - test with care */
            ch = p[z->k - 1];
            p[z->k - 1] = 'e';
            if (ends(z, "4re")) { if (z->j >= z->posV) return true; }
            p[z->k - 1] = ch;
        }

    z->k = b; return false;
}

/*

   verb_ending(z) is searching for one of the following system of endings,

   1st conjugation

     Infinitive  -are

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------

     Present     -o      -i      -a      -iamo   -ate    -ano
     Subjunctive -i      -i      -i      -iamo   -iate   -ino
     Future      -erO    -erai   -erA    -eremo  -erete  -eranno
     Conditional -erei   -eresti -erebbe -eremmo -ereste -erebbero
     Imperfect   -avo    -avi    -ava    -avamo  -avate  -avano
     Past        -ai     -asti   -O      -ammo   -aste   -arono
     Imp.Subj.   -assi   -assi   -asse   -assimo -aste   -assero

     Pres.Part.  -ando
     Past.Part.  -ato -ata -ati -ate


   2nd conjugation

     Infinitive  -ere

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------

     Present     -o      -i      -e      -iamo   -ete    -ono
     Subjunctive -a      -a      -a      -iamo   -iate   -ano
     Future      -erO    -erai   -erA    -eremo  -erete  -eranno
     Conditional -erei   -eresti -erebbe -eremmo -ereste -erebbero
     Imperfect   -evo    -evi    -eva    -evamo  -evate  -evano
     Past        -ei     -esti   -E      -emmo   -este   -erono
     Imp.Subj.   -essi   -essi   -esse   -essimo -este   -essero

     Pres.Part.  -endo
     Past.Part.  -uto -uta -uti -ute

   3rd conjugation

     Infinitive  -ir

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------

     Present     -(isc)o -(isc)i -(isc)e -iamo   -ite    -(isc)ono
     Subjunctive -(isc)a -(isc)a -(isc)a -iamo   -iate   -(isc)ano
     Future      -irO    -irai   -irA    -iremo  -irete  -iranno
     Conditional -irei   -iresti -irebbe -iremmo -ireste -irebbero
     Imperfect   -ivo    -ivi    -iva    -ivamo  -ivate  -ivano
     Past        -ii     -isti   -I      -immo   -iste   -irono
     Imp.Subj.   -issi   -issi   -isse   -issimo -iste   -issero

     Pres.Part.  -endo
     Past.Part.  -ito -ita -iti -ite

*/

static int verb_ending(struct italian_stemmer * z)
{   switch (z->p[z->k - 2])
    {   case 'a':
            if (chopV(z, "2rai")) return true;
            break;
        case 'b':
            if (chopV(z, "2rebbe")) return true;
            break;
        case 'c':
            if (chopV(z, "isc0")) return true;
            break;
        case 'd':
            if (chopV(z, "1nd0")) return true;
            break;
        case 'e':
            if (chopV(z, "2rei")) return true;
            break;
        case 'm':
            if (chopV(z, "iamo") || chopV(z, "2remo") || chopV(z, "2remmo") ||
                chopV(z, "4vamo") || chopV(z, "4mmo")) return true;
            break;
        case 'n':
            if (chopV(z, "4vano") || chopV(z, "isc7no") || chopV(z, "4rono") ||
                chopV(z, "7no") || chopV(z, "2ranno")) return true;
            break;
        case 'r':
            if (chopV(z, "4re") || chopV(z, "2rO") || chopV(z, "2rA") ||
                chopV(z, "2rebbero") || chopV(z, "4ssero")) return true;
            break;
        case 't':
            if (chopV(z, "5t0") || chopV(z, "ete") || chopV(z, "2rete") ||
                chopV(z, "2rest2") || chopV(z, "4vate")) return true;
            break;
        case 'v':
            if (chopV(z, "4v8")) return true;
            break;
    } return false;
}

static int stem_OUS(struct italian_stemmer * z) { return chop2(z, "os"); }
static int stem_ABLE(struct italian_stemmer * z) { return chop2(z, "abil"); }
static int stem_ATIV(struct italian_stemmer * z) { return chop2(z, "ativ"); }
static int stem_IC(struct italian_stemmer * z) { return chop2(z, "ic"); }
static int stem_IV(struct italian_stemmer * z) { return chop2(z, "iv"); }

static int stem_AT(struct italian_stemmer * z)
{   if (chop2(z, "at")) { stem_IC(z); return true; }
    return false;
}

static int remove_suffix(struct italian_stemmer * z)
{   char * p = z->p;
    switch (p[z->k - 2])
    {   case 'c':
            if (chop2(z, "atric2")) { stem_IC(z); return true; }
            if (chop2(z, "ic0")) return true;
            break;
        case 'h':
            if (chop2(z, "ich2")) return true;
            break;
        case 'i':
            if (ends(z, "logi1") && attach2(z, "log")) return true;
            break;
        case 'l':
            if (chop2(z, "abil2")) return true;
            if (chop2(z, "ibil2")) return true;
            break;
        case 'm':
            if (chop2(z, "ism3")) return true;
            break;
        case 'n':
            if (chop2(z, "azion2")) { stem_IC(z); return true; }
            if (ends(z, "uzion2") && attach2(z, "u")) return true;
            if (ends(z, "usion2") && attach2(z, "u")) return true;
            break;
        case 'r':
            if (chop2(z, "ator2")) { stem_IC(z); return true; }
            break;
        case 's':
            if (chop2(z, "os0")) return true;
            break;
        case 't':
            if (chop2(z, "ist6")) return true;
            if (chopV(z, "amente")) /* Note chopV here */
            {   if (stem_OUS(z)) return true;
                if (stem_ABLE(z)) return true;
                if (stem_ATIV(z)) return true;
                if (stem_IC(z)) return true;
                stem_IV(z); return true;
            }
            if (chopV(z, "ament3") || chopV(z, "iment3")) return true;
            if (chop2(z, "mente")) return true;
            if (chop2(z, "itA"))
            {   if (stem_ABLE(z)) return true;
                if (stem_IV(z)) return true;
                stem_IC(z); return true;
            }
            break;
        case 'v':
            if (chop2(z, "iv0")) { stem_AT(z); return true; }
            break;
        case 'z':
            if (chop2(z, "anz1")) return true;
            if (ends(z, "enz1") && attachV(z, "ente")) return true;
                           /* Note attachV here */
            break;
    } return false;
}

static void tidy_up(struct italian_stemmer * z)
{   char * p = z->p;
    {   int i;
        for (i = 0; i < z->k; i++) p[i] = tolower(p[i]);
    }

       /* lose a,e,i,o and possible preceding i */
    if (chopV(z, "0")) chopV(z, "i");

    if (ends(z, "gh")) { attachV(z, "g"); return; }
    if (ends(z, "ch")) { attachV(z, "c"); return; }
}

/* In italian_stem(z, p, i, j), p is a struct italian_stemmer pointer, and the
   string to be stemmed is from p[i] to p[j] inclusive. Typically i is zero
   and j is the offset to the last character of a string, (p[j+1] == '\0').
   The stemmer return a pointer to the stemmed form of this word in structure
   z.
*/

#define PAIR(a, b)   ((a)<<8|(b))

extern char * italian_stem(struct italian_stemmer * z, char * q, int i0, int i1)
{   char * p = z->p;
    int p_size = z->p_size;
    int k = 0;
    if (i1-i0+50 > p_size)
    {   free(p);
        p_size = i1-i0+75; /* ample */ z->p_size = p_size;
        p = (char *) malloc(p_size); z->p = p;
    }

    /* move into p, translating accents */
    {   int j; for (j = i0; j<=i1; j++)
        {   int ch = q[j];
            if (ch == '^' && i0 < j && j < i1)
            {   int letter = q[j-1], accent = q[j+1];
                switch (PAIR(letter, accent))
                {   case PAIR('a', 'g'):
                    case PAIR('e', 'a'):
                    case PAIR('e', 'g'):
                    case PAIR('i', 'g'):
                    case PAIR('o', 'g'): p[k-1] = toupper(letter); break;
                } j++;
            } else p[k++] = ch;
        }
    }
    z->k = k;

    {   char * t = search_pool(z->irregulars, k, p);
        if (t != 0) return t;
    }

    measure(z);
    {   attached_pronoun(z);
        if (!remove_suffix(z)) verb_ending(z);
        tidy_up(z);
    }
    p[z->k] = 0; /* C string form for now */
    return p;
}

/* This table of irregular forms should grow into a more cosidered
   list after an analysis over time of frequently reported errors. It
   is presented here with the irregular forms of the commonest verbs
   just to show how it works, not because of any critical importance
   of these verbs. The form of the table is:

     "p1" "s11/s12/s13/ ... /"
     "p2" "s21/s22/s23/ ... /"
     ...
     "pn" "sn1/sn2/sn3/ ... /"
     0, 0

   String sij is mapped to paradigm form pi, and the main stemming
   process is then bypassed.
*/

static char * irregular_forms[] = {

    "aver" ,

    "ho/hai/ha/abbiamo/avete/hanno/"
    "abbia/abbiate/abbiano/"
    "avro^g/avrai/avra^g/avremo/avrete/avranno/"
    "avrei/avresti/avrebbe/avremmo/avreste/avrebbero/"
    "avevo/avevi/aveva/avevamo/avevate/avevano/"
    "ebbi/avesti/ebbe/avemmo/aveste/ebbero/"
    "avessi/avesse/avessimo/avessero/"
    "avendo/"
    "avuto/avuta/avuti/avute/"
  ,
    "dar" ,

    "do/da^g/diamo/danno/"
    "dia/diate/diano/"
    "daro^g/darai/dara^g/daremo/darete/daranno/"
    "darei/daresti/darebbe/daremmo/dareste/darebbero/"
    "davo/davi/dava/davamo/davate/davano/"
    "diedi/desti/diede/demmo/deste/diedero/"
    "dessi/desse/dessimo/dessero/"
    "dando/"
  ,
    /* omit dai dat- */

    "esser" ,

    "sono/sei/e^g/siamo/siete/"
    "sia/siate/siano/"
    "saro^g/sarai/sara^g/saremo/sarete/saranno/"
    "sarei/saresti/sarebbe/saremmo/sareste/sarebbero/"
    "ero/eri/era/eravamo/eravate/erano/"
    "fui/fosti/fu/fummo/foste/furono/"
    "fossi/fosse/fossimo/fossero/"
    "essendo/"
  ,
    "far" ,

    "faccio/fai/facciamo/fanno/"
    "faccia/facciate/facciano/"
    "faro^g/farai/fara^g/faremo/farete/faranno/"
    "farei/faresti/farebbe/faremmo/fareste/farebbero/"
    "facevo/facevi/faceva/facevamo/facevate/facevano/"
    "feci/facesti/fece/facemmo/faceste/fecero/"
    "facessi/facesse/facessimo/facessero/"
    "facendo/"
  ,
    /* omit fa fat- */

    "star" ,

    "sto/stai/sta/stiamo/stanno/"
    "stia/stiate/stiano/"
    "staro^g/starai/stara^g/staremo/starete/staranno/"
    "starei/staresti/starebbe/staremmo/stareste/starebbero/"
    "stavo/stavi/stava/stavamo/stavate/stavano/"
    "stetti/stesti/stette/stemmo/steste/stettero/"
    "stessi/stesse/stessimo/stessero/"
    "stando/"
  ,
    /* omit stat- */

  0, 0  /* terminator */
};

extern struct italian_stemmer * setup_italian_stemmer()
{  struct italian_stemmer * z = (struct italian_stemmer *) malloc(sizeof(struct italian_stemmer));
   z->p = 0; z->p_size = 0;
   z->irregulars = create_pool(irregular_forms);
   return z;
}

extern void closedown_italian_stemmer(struct italian_stemmer * z)
{  free_pool(z->irregulars);
   free(z->p);
   free(z);
}

