/* stem_french.c: French stemming algorithm.
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
#include <ctype.h>  /* for isupper, islower, toupper, tolower */

#include "pool.h"
#include "stem_french.h"

#define true 1
#define false 0

#define CON 0
#define VOW 1

struct french_stemmer
{
    char * p;
    int p_size;
    int k;

    int j;
    int posV;
    int pos0;
    int pos1;
    int pos2;
    struct pool * irregulars;
    struct pool * irregulars2;

};

/*-------
    void debug(struct french_stemmer * z, int n)
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
-------*/

/* char_type(z, i) is CON or VOW according as the character at i is a consonant
   or vowel. Used in measure().

   Upper case letter usage:

        A      a^h   (a circumflex)
        F      e^a   (e acute)
        G      e^g   (e grave)
        E      e^h   (e circumflex)
        I      i^h   (i circumflex)
        U      u^h   (u cicumflex)
        J      i^u   (i trema, or diaeresis)

*/

static int vowel(int ch)
{   switch (ch)
    {   case 'a': case 'e': case 'i': case 'o': case 'u':
        case 'A': case 'F': case 'G': case 'E': case 'I': case 'U': case 'J':
     /*  a-hat     e-acute   e-grave  e-hat     i-hat     u-hat     i-trema  */
            return true;
        default: return false;
    }
}

static int char_type(struct french_stemmer * z, int i)
{   char * p = z->p;
    switch (p[i])
    {   case 'a': case 'e': case 'o':
        case 'A': case 'F': case 'G': case 'E': case 'I': case 'U': case 'J':
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
        case 'y':
            if (0 < i &&
                i < z->k - 1 &&
                ! vowel(p[i-1]) &&
                ! vowel(p[i+1])) return VOW;
            return CON;
        default: return CON;
    }
}

/* measure(z) establishes two critical positions in the word, posV and pos2.
   A suffix will only be removed if it leaves behind a stem which is either
   posV or pos2 characters long. posV is used for verbs, pos2 for other forms.

   If the word begins with 2 or more VOWs posV is at the first CON, if it
   begins VOW-CON posV is at the second VOW, if it begins CON posV is at the
   first VOW [or the 3rd letter, whichever is further to the right].

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

   pos0 and pos1 are also established for certain rarer tests.
*/

static void measure(struct french_stemmer * z)
{   int j = 0;
    int k = z->k;
    z->posV = k; /* default */
    if (k < 2) return;
    while (j < k && char_type(z, j) == VOW) j++;
    if (j >= 2) z->posV = j; else
    {
        /* now go to a vowel */
        while (!(j == k || char_type(z, j) == VOW)) j++;
        z->posV = j;
    }
    j = 0;
    {   int i; for (i = 0;; i++)
        {   while (j < k && char_type(z, j) == CON) j++;
            if (i == 0) z->pos0 = j-1; else
            if (i == 1) z->pos1 = j-1; else
            if (i == 2) { z->pos2 = j-1; break; }
            while (j < k && char_type(z, j) == VOW) j++;
        }
    }
}

/* look_for(z, s) tests that the characters ..., p[j-1], p[j] contain string
   s. The result is true/false, and if true, j is decreased by the length of
   s.
*/

static int look_for(struct french_stemmer * z, const char * s)
{   int length = strlen(s);
    int jbase = z->j - length + 1;
    if (jbase < 0) return false;
    if (memcmp(s, z->p + jbase, length) != 0) return false;
    z->j -= length;
    return true;
}

/* ends(z, s) applies look_for(z, s) at the end of the word, i.e. with j = k-1. */

static int ends(struct french_stemmer * z, const char * s)
{  z->j = z->k - 1;
/*printf("[%s] ",s); */
   return look_for(z, s);
}

/* setto(z, s) sets (j+1), ... k to the characters in the string s, readjusting
   k.
*/

static void setto(struct french_stemmer * z, const char * s)
{   int length = strlen(s);
    memmove(z->p + z->j + 1, s, length);
    z->k = z->j + length + 1;
}

/* attachV(z, s) attaches s to the so long as j is not before posV,
   after_posV(z) tests that j is not before posV,
   chopV(z, s) prepares to remove suffix s if it is after posV,

   and similarly,

   attach2(z, s) attaches s to the so long as j is not before pos2,
   after_pos2(z) tests that j is not before pos2,
   chop2(z, s) prepares to remove suffix s if it is after pos2.

   and similarly attach0, attach1 etc.

*/

static int attachV(struct french_stemmer * z, const char * s)
{   if (z->j < z->posV) return false;
    setto(z, s);
    return true;
}

static int after_posV(struct french_stemmer * z)
{   if (z->j < z->posV) return false;
    z->k = z->j + 1;
    return true;
}

static int chopV(struct french_stemmer * z, const char * s) { return ends(z, s) && after_posV(z); }


static int attach0(struct french_stemmer * z, const char * s)
{   if (z->j < z->pos0) return false;
    setto(z, s);
    return true;
}

static int attach1(struct french_stemmer * z, const char * s)
{   if (z->j < z->pos1) return false;
    setto(z, s);
    return true;
}

/*
static int after_pos1(struct french_stemmer * z)
{   if (z->j < z->pos1) return false;
    z->k = z->j + 1;
    return true;
}
*/

/*
static int chop1(struct french_stemmer * z, const char * s)
{
return ends(z, s) && after_pos1(z);
}
*/

static int attach2(struct french_stemmer * z, const char * s)
{   if (z->j < z->pos2) return false;
    setto(z, s);
    return true;
}

static int after_pos2(struct french_stemmer * z)
{   if (z->j < z->pos2) return false;
    z->k = z->j + 1;
    return true;
}

static int chop2(struct french_stemmer * z, const char * s) { return ends(z, s) && after_pos2(z); }

/* chopV_ge(z, s) is like chopV, but removes a preceding 'e' after 'g'.
*/

static int chopV_ge(struct french_stemmer * z, const char * s)
{   if (! (ends(z, s) && after_posV(z))) return false;
    if (look_for(z, "ge")) z->k --;
    return true;
}

/* chopV_CON(z, s) is like chopV, but the residual string must end with
   a CON.
*/

static int chopV_CON(struct french_stemmer * z, const char * s)
{   return ends(z, s) && char_type(z, z->j) == CON && after_posV(z);
}


/*

   verb_ending(z) is searching among the following system of endings,

   1st conjugation

     Infinitive  -er

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -e      -es     -e      -ons    -ez     -ent
     Subjunctive -e      -es     -e      -ions   -iez    -ent
     Future      -erai   -eras   -era    -erons  -erez   -eront
     Conditional -erais  -erais  -erait  -erions -eriez  -eraient
     Imperfect   -ais    -ais    -ait    -ions   -iez    -aient
     Past        -ai     -as     -a      -Ames   -Ates   -Grent
     Imp.Subj.   -asse   -asses  -At     -assions-assiez -assent

     Pres.Part.  -ant -ante -ants -antes
     Past.Part.  -F -Fe -Fs -Fes


   2nd conjugation

     Infinitive  -re

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -s      -s      -       -ons    -ez     -ent
     Subjunctive -e      -es     -e      -ions   -iez    -ent
     Future      -rai    -ras    -ra     -rons   -rez    -ront
     Conditional -rais   -rais   -rait   -rions  -riez   -raient
     Imperfect   -ais    -ais    -ait    -ions   -iez    -aient
     Past        -is     -is     -it     -Imes   -Ites   -irent
     Imp.Subj.   -isse   -isses  -It     -issions-issiez -issent

     Pres.Part.  -ant -ante -ants -antes
     Past.Part.  -u -ue -us -ues

   3rd conjugation

     Infinitive  -ir

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -is     -is     -it     -issons -issez  -issent
     Subjunctive -isse   -isses  -isse   -issions-issiez -issent
     Future      -irai   -iras   -ira    -irons  -irez   -iront
     Conditional -erais  -erais  -erait  -erions -eriez  -eraient
     Imperfect   -issais -issais -issait -issions-issiez -issaient
     Past        -is     -is     -it     -Imes   -Ites   -irent
     Imp.Subj.   -isse   -isses  -It     -issions-issiez -issent

     Pres.Part.  -issant -issante -issants -issantes
     Past.Part.  -i -ie -is -ies

*/

static int verb_ending(struct french_stemmer * z)
{   switch (z->p[z->k - 1])
    {   case 'F':
            if (chopV(z, "F")) return true;
            break;
        case 'a':
            if (chopV(z, "era") || chopV_CON(z, "ira") || chopV_ge(z, "a"))
                return true;
            break;
        case 'e':
            if (chopV_ge(z, "asse") || chopV_CON(z, "isse") || chopV(z, "Fe") ||
                chopV_CON(z, "ie") || chopV_CON(z, "issante") || chopV_ge(z, "ante"))
                return true;
            break;
        case 'i':
            if (chopV(z, "erai") || chopV_CON(z, "irai") || chopV_ge(z, "ai") ||
                chopV_CON(z, "i")) return true;
            break;
        case 'r':
            if (chopV(z, "er") || chopV_CON(z, "ir")) return true;
            break;
        case 's':
            switch (z->p[z->k - 2])
            {   case 'n':
                    if (chopV_CON(z, "issons") ||  chopV(z, "erons") ||
                        chopV_CON(z, "irons") || chopV_ge(z, "assions") ||
                        chopV_CON(z, "issions") ||  chopV(z, "erions") ||
                        chopV_CON(z, "irions") ||
                        chop2(z, "ions")) return true;  /* Note chop2 */
                    break;
                case 'e':
                    if (chopV_ge(z, "Ames") || chopV_CON(z, "Imes") ||
                        chopV_ge(z, "asses") || chopV_CON(z, "isses") ||
                        chopV(z, "Fes") || chopV_CON(z, "ies") ||
                        chopV_ge(z, "Ates") || chopV_CON(z, "Ites") ||
                        chopV_CON(z, "issantes") || chopV_ge(z, "antes")) return true;
                    break;
                case 'i':
                    if (chopV_CON(z, "issais") ||  chopV(z, "erais") ||
                        chopV_CON(z, "irais") ||  chopV_ge(z, "ais") ||
                        chopV_CON(z, "is")) return true;
                    break;
                case 'a':
                    if (chopV(z, "eras") || chopV_CON(z, "iras") ||
                        chopV_ge(z, "as")) return true;
                    break;
                case 't':
                    if (chopV_CON(z, "issants") || chopV_ge(z, "ants")) return true;
                    break;
                case 'F':
                    if (chopV(z, "Fs")) return true;
                    break;
            }
            break;
        case 't':
             switch (z->p[z->k - 2])
             {   case 'n':
                     if (chopV_CON(z, "issant") ||  chopV_ge(z, "ant") ||
                         chopV(z, "eront") || chopV_CON(z, "iront") ||
                         chopV(z, "Grent") || chopV_CON(z, "irent") ||
                         chopV_ge(z, "assent") || chopV_CON(z, "issent") ||
                         chopV_CON(z, "issaient") || chopV(z, "eraient") ||
                         chopV_CON(z, "iraient") || chopV_ge(z, "aient")) return true;
                     break;
                 case 'i':
                     if (chopV_CON(z, "issait") ||  chopV(z, "erait") ||
                         chopV_CON(z, "irait") ||  chopV_ge(z, "ait") ||
                         chopV_CON(z, "it")) return true;
                     break;
                 case 'A':
                     if (chopV_ge(z, "At")) return true;
                     break;
                 case 'I':
                     if (chopV_CON(z, "It")) return true;
                     break;
             }
             break;
        case 'z':
            if (chopV_ge(z, "assiez") || chopV_CON(z, "issiez") ||
                chopV(z, "eriez") || chopV_CON(z, "iriez") || chopV_CON(z, "issez") ||
                chopV(z, "erez") || chopV_CON(z, "irez") || chopV(z, "iez") ||
                chopV(z, "ez")) return true;
            break;
    }
    return false;
}

/*
static int stem_OUS(struct french_stemmer * z) { return chop2(z, "os"); }
*/
static int stem_ABLE(struct french_stemmer * z) { return chop2(z, "abl"); }
static int stem_ATIV(struct french_stemmer * z) { return chop2(z, "ativ"); }
static int stem_IQU(struct french_stemmer * z) { return chop2(z, "iqu"); }
static int stem_IV(struct french_stemmer * z) { return chop2(z, "iv"); }

static int stem_IC(struct french_stemmer * z)
{   if (ends(z, "ic")) { if (after_pos2(z)) return true; setto(z, "iqu"); }
    return false;
}

static int stem_AT(struct french_stemmer * z)
{   if (chop2(z, "at")) { stem_IC(z); return true; }
    return false;
}

static int stem_ABIL(struct french_stemmer * z)
{   if (ends(z, "abil")) setto(z, "abl");
    return stem_ABLE(z);
}

static int stem_EUSE(struct french_stemmer * z)
{   return ends(z, "eus") && (after_pos2(z) || attach1(z, "eux"));
}

static int remove_suffix(struct french_stemmer * z)
{   char * p = z->p;
    switch (p[z->k - 1])
    {   case 'e':
            if (chop2(z, "ance")) return true;
            if (chop2(z, "ence") && attach1(z, "ent")) return true; /* Note attach1 */
            if (chop2(z, "atrice")) { stem_IC(z); return true; }

            if (ends(z, "euse") && (after_pos2(z) || attach1(z, "eux"))) return true;
            if (chop2(z, "ique")) return true;
            if (chop2(z, "isme")) return true;
            if (chop2(z, "able")) return true;
            if (chop2(z, "iste")) return true;
            if (chop2(z, "ive")) {  stem_AT(z); return true; }
            if (ends(z, "logie") && attach2(z, "log")) return true;
            return false;
        case 't':
            if (! ends(z, "ent")) return false;
            if (chopV_CON(z, "issement")) return true;    /* verbal form */
            if (chopV(z, "ement"))                        /* Note chop1 here */
            {   if (stem_EUSE(z)) return true;
                if (stem_ABLE(z)) return true;
                if (stem_ATIV(z)) return true;
                if (stem_IQU(z)) return true;
                stem_IV(z); return true;
            }
            if (ends(z, "emment") && attach0(z, "ent")) { verb_ending(z); return true; }
            if (ends(z, "amment") && attach0(z, "ant")) { verb_ending(z); return true; }
            /* attach0 slightly better than attachV here */
            if (ends(z, "ment") &&
                z->j - 1 >= z->posV &&
                char_type(z, z->j) == VOW)

            {   z->k = z->j + 1;
                verb_ending(z);
                return true;
            }
            return false;
        case 'n':
            if (chop2(z, "ation")) { stem_IC(z); return true; }
            if (ends(z, "ution") && attach2(z, "u")) return true;
            if (ends(z, "usion") && attach2(z, "u")) return true;
            return false;
        case 'F':
            if (chop2(z, "itF"))
            {   if (stem_ABIL(z)) return true;
                if (stem_IV(z)) return true;
                stem_IC(z); return true;
            }
            return false;
        case 'f':
            if (chop2(z, "if")) { stem_AT(z); return true; }
            return false;
        case 'r':
            if (chop2(z, "ateur")) { stem_IC(z); return true; }
            return false;
        case 'x':
            if (chop2(z, "eux")) return true; /* ignoring -euxs possibility */
            if (ends(z, "eaux")) { setto(z, "eau"); return true; }
            if (ends(z, "aux") && attach1(z, "al")) return true;
            return false;
        default:
            return false;
    }
}

static int s_or_t(int ch) { return ch == 's' || ch == 't'; }
static int not_aiou(int ch) { return ch == 'e' || ch == 'F' || ! vowel(ch); }

static void residual_ending(struct french_stemmer * z)
{   char * p = z->p;

    /* The next two tests must be done after verb_ending() so the longer
       -assions can be removed first.
    */

    if (ends(z, "ion") && s_or_t(p[z->j]) && after_pos2(z)) return;
    if (ends(z, "s") && not_aiou(p[z->j])) z->k--;

    if (ends(z, "ier") && attachV(z, "i")) return;  /* cuisinier(s) etc */
    if (ends(z, "iGre") && attachV(z, "i")) return; /* cuisiniGre(s) etc */

    chopV(z, "e");
}

static int double_letter(struct french_stemmer * z)
{   switch (z->p[z->k - 1])
    {   case 'n':
            if (ends(z, "enn") || ends(z, "onn")) return true;
            break;
        case 't':
            if (ends(z, "ett")) return true;
            break;
        case 'l':
            if (ends(z, "ell") || ends(z, "eill")) return true;
            break;
    }
    return false;
}

/* In french_stem(z, p, i, j), p is a struct french_stemmer pointer, and the
   string to be stemmed is from p[i] to p[j] inclusive. Typically i is zero
   and j is the offset to the last character of a string, (p[j+1] == '\0').
   The stemmer return a pointer to the stemmed form of this word in structure
   z.
*/

#define PAIR(a, b)   ((a)<<8|(b))

extern const char * french_stem(struct french_stemmer * z, const char * q, int i0, int i1)
{   char * p = z->p;
    int p_size = z->p_size;
    int k = 0;
    if (i1-i0+50 > p_size)
    {   free(p);
        p_size = i1-i0+75; /* ample */ z->p_size = p_size;
        p = (char *) malloc(p_size); z->p = p;
    }

    /* POINT A:
       move into p, translating accents. This block depends on the Muscat
       accent representation scheme, and will need to be rewritten
       for alternative schemes.
    */

    {   int j; for (j = i0; j<=i1; j++)
        {   int ch = q[j];
            if (ch == '^' && i0 < j && j < i1)
            {   int letter = q[j-1], accent = q[j+1];
                switch (PAIR(letter, accent))
                {   case PAIR('a', 'h'):
                    case PAIR('e', 'h'):
                    case PAIR('i', 'h'):
                    case PAIR('u', 'h'): p[k-1] = toupper(letter); break;
                    case PAIR('e', 'g'): p[k-1] = 'G'; break;
                    case PAIR('e', 'a'): p[k-1] = 'F'; break;
                    case PAIR('i', 'u'): p[k-1] = 'J'; break;
                } j++;
            } else p[k++] = ch;
        }
    }
    z->k = k;

    {   const char * t = search_pool(z->irregulars, k, p);
        if (t != 0) return t;
    }

    measure(z);

    {   int plural = false;
        if (ends(z, "s")) { plural = true; z->k--; }
        if (remove_suffix(z))
        {   if (ends(z, "y")) attachV(z, "i");
        }
        else
        {   if (plural) z->k++;
            if (verb_ending(z))
            {   if (ends(z, "y")) attachV(z, "i");
            }
            else residual_ending(z);
        }
    }
    if (double_letter(z)) z->k--;

    /* get rid of accents (or should ^h be restored?) */
    k = z->k;

    /* POINT B:
       Translate the internal representation of accents in p. This
       may need to be reassessed given the comment at POINT A above.
    */


    {   int i = 0;
        while (i != k)
        {   int ch = p[i];
            if (isupper(ch)) switch (ch)
            {   case 'F':
                case 'G': p[i] = 'e'; break;
                case 'J': p[i] = 'i'; break;
                default:  p[i] = tolower(ch);
            }
            i++;
        }
    }

    {   const char * t = search_pool(z->irregulars2, k, p);
        if (t != 0) return t;
    }

    p[k] = 0; /* C string form for now */
    return p;
}

/* This table of irregular forms should grow into a more cosidered
   list after an analysis over time of frequently reported errors.
   It is presented here with the irregular forms of the verbs Etre,
   avoir and aller just to show how it works, not because of any
   critical importance of these verbs. The form of the table is:

     "p1" "s11/s12/s13/ ... /"
     "p2" "s21/s22/s23/ ... /"
     ...
     "pn" "sn1/sn2/sn3/ ... /"
     0, 0

   String sij is mapped to paradigm form pi, and the main stemming
   process is then bypassed.
*/

static const char * irregular_forms[] = {

    "etr" ,
    "Etre/FtF/FtFe/FtFes/FtFs/Ftant/Ftante/Ftants/Ftantes/suis/es/"
    "est/sommes/Etes/sont/serai/seras/sera/serons/serez/seront/serais/"
    "serait/serions/seriez/seraient/Ftais/Ftait/Ftions/Ftiez/Ftaient/"
    "fus/fut/fUmes/fUtes/furent/sois/soit/soyons/soyez/soient/fusse/"
    "fusses/fUt/fussions/fussiez/fussent/"
  ,
    "avoir" ,
    "avoir/ayant/ayante/ayantes/ayants/eu/eue/eues/eus/ai/as/avons/"
    "avez/ont/aurai/auras/aura/aurons/aurez/auront/aurais/aurait/"
    "aurions/auriez/auraient/avais/avait/avions/aviez/avaient/"
    "eut/eUmes/eUtes/eurent/aie/aies/ait/ayons/ayez/aient/eusse/"
    "eusses/eUt/eussions/eussiez/eussent/"
  ,

    "aller" ,
    "aller/allant/allante/allants/allantes/allF/allFs/allFe/allFes/"
    "vais/vas/va/allons/allez/vont/irai/iras/ira/irons/irez/iront/"
    "irais/irait/irions/iriez/iraient/allais/allait/allions/"
    "alliez/allaient/allai/allas/alla/allAmes/allAtes/allGrent/aille/"
    "ailles/aillent/allasse/allasses/allAt/allassions/"
    "allassiez/allassent/"
  ,
  0, 0  /* terminator */
};

/* And now a new idea: a list of irregulars to be applied
   AFTER the stemming process has been completed. This can handle
   stem changes in irregular verbs, among other things. Again, this
   should be elaborated further.

   Only in French at the moment (4 Feb 00) but it would be easy to
   extend it to the other algorithms.
*/

static const char * irregular_stems[] = {

    "vend" ,     "vendr/vendu/",
    "prend" ,    "prendr/",
    "comprend" , "comprendr/comprendu/compr/",

    /* etcetera */

    0, 0  /* terminator */
};

extern struct french_stemmer * setup_french_stemmer()
{  struct french_stemmer * z = (struct french_stemmer *) malloc(sizeof(struct french_stemmer));
   z->p = 0; z->p_size = 0;
   z->irregulars = create_pool(irregular_forms);
   z->irregulars2 = create_pool(irregular_stems);
   return z;
}

extern void closedown_french_stemmer(struct french_stemmer * z)
{  free_pool(z->irregulars);
   free(z->p);
   free(z);
}

