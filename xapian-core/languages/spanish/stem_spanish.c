/* stem_spanish.c: Spanish stemming algorithm.
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
#include "stem_spanish.h"

#define true 1
#define false 0

#define CON 0
#define VOW 1

/*
void debug(struct spanish_stemmer * z, int n)
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
*/

/* char_type(ch) is CON or VOW according as ch is a consonant or vowel.
   Used in measure().
*/

int char_type(int ch)
{   switch (ch)
    {   case 'a': case 'e': case 'i': case 'o': case 'u':
        case 'A': case 'E': case 'I': case 'O': case 'U':
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

   Here are some examples ('1' '2' indicate posV and pos2):

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

int measure(struct spanish_stemmer * z)
{   int j = 0;
    int k = z->k;
    char * p = z->p;
    z->posV = k; /* default */
    if (k < 2) return;
    while (j < k && char_type(p[j]) == VOW) j++;
    if (j >= 2) z->posV = j; else
    {
        /* now go to a vowel */
        while (!(j == k || char_type(p[j]) == VOW)) j++;
        if (j == 1) j++;  /* to 3rd letter */
        z->posV = j;
    }
    j = 0;
    {   int i; for (i = 0;; i++)
        {   while (j < k && char_type(p[j]) == CON) j++;
            if (i == 2) { z->pos2 = j-1; break; }
            while (j < k && char_type(p[j]) == VOW) j++;
        }
    }
}

/* look_for(z, s) tests that the characters ..., p[j-1], p[j] contain string
   s. If s[0] is '.' the '.' matches 'a' or 'e' or 'i'. If s[0] is 'Y', the
   'Y' matches 'y', so long as the 'y' follows 'u'. The result is true/false,
   and if true, j is decreased by the length of s.
*/

int aei(int ch) { return (ch == 'a' || ch == 'e' || ch == 'i'); }

int look_for(struct spanish_stemmer * z, char * s)
{   char * p = z->p;
    int length = strlen(s);
    int jbase = z->j-length+1;
    int o = 0;
    int firstch = s[0];
    if (jbase < 0) return false;
    if (firstch == '.')
    {   if (!aei(p[jbase])) return false;
        o = 1;
    } else
    if (firstch == 'Y')
    {   if (p[jbase] != 'y') return false;
        if (!(jbase > 0 && p[jbase-1] == 'u')) return false;
        o = 1;
    }
    if (memcmp(s+o, p+jbase+o, length-o) != 0) return false;
    z->j -= length;
    return true;
}

/* ends(z, s) applies look_for(z, s) at the end of the word, i.e. with j = k-1. */

int ends(struct spanish_stemmer * z, char * s)
{  z->j = z->k - 1;
   return look_for(z, s);
}

/* setto(z, s) sets (j+1), ... k to the characters in the string s, readjusting
   k.
*/

void setto(struct spanish_stemmer * z, char * s)
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

   -- note that attachV is not actually used.

   chopV_try_u is like chopV, but also takes off a preceding 'u' after a 'g'.
*/

/*
int attachV(struct spanish_stemmer * z, char * s)
{   if (z->j < z->posV) return false;
    setto(z, s);
    return true;
}
*/

int after_posV(struct spanish_stemmer * z)
{   if (z->j < z->posV) return false;
    z->k = z->j+1;
    return true;
}

int chopV(struct spanish_stemmer * z, char * s) { return ends(z, s) && after_posV(z); }

int chopV_try_u(struct spanish_stemmer * z, char * s)
{   if (!ends(z, s)) return false;
    if (z->p[z->j-1] == 'g') look_for(z, "u");
    return after_posV(z);
}

int attach2(struct spanish_stemmer * z, char * s)
{   if (z->j < z->pos2) return false;
    setto(z, s);
    return true;
}

int after_pos2(struct spanish_stemmer * z)
{   if (z->j < z->pos2) return false;
    z->k = z->j+1;
    return true;
}

int chop2(struct spanish_stemmer * z, char * s) { return ends(z, s) && after_pos2(z); }

/* attached pronouns can of course double, but apart from
   -se + lo, la, los, las they seem to be v. rare, and are ignored here.

*/

int attached_pronoun(struct spanish_stemmer * z)
{   char * p = z->p;
    int b = z->k;
    switch (p[b-2])
    {   case 'a':
          if (!(ends(z, "selas") || ends(z, "las"))) return false;
          break;
        case 'e':
          if (!(ends(z, "les"))) return false;
          break;
        case 'l':
          if (!(ends(z, "sela") || ends(z, "selo") || ends(z, "la") || ends(z, "le") ||
                  ends(z, "lo"))) return false;
          break;
        case 'm':
          if (!(ends(z, "me"))) return false;
          break;
        case 'o':
          if (!(ends(z, "selos") || ends(z, "los") || ends(z, "nos"))) return false;
          break;
        case 's':
          if (!(ends(z, "se"))) return false;
          break;
        default:
          return false;
    }
    z->k = z->j+1;

    {   int v = false;  /* set true if valid ending before the pronoun */
        if (ends(z, "iEndo")) { p[z->j+2] = 'e'; v = true; } else
        if (ends(z, "Ando")) { p[z->j+1] = 'a'; v = true; } else
        if (ends(z, "Ar")) { p[z->j+1] = 'a'; v = true; } else
        if (ends(z, "Er")) { p[z->j+1] = 'e'; v = true; } else
        if (ends(z, "Ir")) { p[z->j+1] = 'i'; v = true; } else
        if (ends(z, "iendo") || ends(z, "Yendo") || ends(z, "ando") || ends(z, ".r")) v = true;

        if (v) { if (z->j >= z->posV) return true; }
    }
    z->k = b; return false;
}

/*

   verb_ending(z) is searching for one of the following system of endings,

   1st conjugation

     Infinitive  -ar

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -as     -a      -amos   -Ais    -an
     Subjunctive -e      -es     -e      -emos   -Eis    -en
     Future      -arE    -arAs   -arA    -aremos -arEis  -arAn
     Conditional -arIa   -arIas  -arIa   -arIamos-arIais -arIan
     Imperfect   -aba    -abas   -aba    -Abamos -abais  -aban
     Past        -E      -aste   -O      -amos   -asteis -aron
     Imp.Subj.   -ara    -aras   -ara    -Aramos -arais  -aran
                 -ase    -ases   -ase    -Asemos -aseis  -asen

     Pres.Part.  -ando
     Past.Part.  -ado -ada -ados -adas

     Imperative  -ad


   2nd conjugation

     Infinitive  -er

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -es     -e      -emos   -Eis    -en
     Subjunctive -a      -as     -a      -amos   -Ais    -an
     Future      -erE    -erAs   -erA    -eremos -erEis  -erAn
     Conditional -erIa   -erIas  -erIa   -erIamos-erIais -erIan
     Imperfect   -Ia     -Ias    -Ia     -Iamos  -Iais   -Ian
     Past        -I      -iste   -iO     -imos   -isteis -ieron
     Imp.Subj.   -iera   -ieras  -iera   -iEramos-ierais -ieran
                 -iese   -ieses  -iese   -iEsemos-ieseis -iesen

     Pres.Part.  -iendo
     Past.Part.  -ido -ida -idos -idas

     Imperative  -ed

   3rd conjugation

     Infinitive  -ir

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -es     -e      -imos   -Is     -en
     Subjunctive -a      -as     -a      -amos   -Ais    -an
     Future      -irE    -irAs   -irA    -iremos -irEis  -irAn
     Conditional -irIa   -irIas  -irIa   -irIamos-irIais -irIan
     Imperfect   -Ia     -Ias    -Ia     -Iamos  -Iais   -Ian
     Past        -I      -iste   -iO     -imos   -isteis -ieron
     Imp.Subj.   -iera   -ieras  -iera   -iEramos-ierais -ieran
                 -iese   -ieses  -iese   -iEsemos-ieseis -iesen

     Pres.Part.  -iendo
     Past.Part.  -ido -ida -idos -idas

     Imperative  -id

   Endings 'Yas', 'Yan' etc are for -ir verbs with stem ending 'u': e.g.
   'construir'. In most grammars these are classed as irregular.


*/


int verb_ending(struct spanish_stemmer * z)
{   switch (z->p[z->k-2])
    {   case 'a':
          if (chopV(z, "ieran") || chopV(z, "ieras") || chopV(z, ".rIan") ||
              chopV(z, ".rIas") || chopV(z, "aban") || chopV(z, "aran") ||
              chopV(z, "abas") || chopV(z, "adas") || chopV(z, "idas") ||
              chopV(z, "aras") || chopV(z, "Ian") || chopV(z, "Ias") ||
              chopV(z, "Yan") || chopV(z, "Yas") || chopV(z, "ad") ||
              chopV(z, "an") || chopV(z, "ar") || chopV(z, "as")) return true;
          break;
        case 'A':
          if (chopV(z, ".rAn") || chopV(z, ".rAs")) return true;
          break;
        case 'b':
          if (chopV(z, "aba")) return true;
          break;
        case 'd':
          if (chopV(z, "iendo") || chopV(z, "Yendo") || chopV(z, "ando") ||
          chopV(z, "ido") || chopV(z, "ida") || chopV(z, "ado") ||
          chopV(z, "ada")) return true;
          break;
        case 'e':
          if (chopV(z, "iesen") || chopV(z, "ieses") || chopV(z, "asen") ||
              chopV(z, "ases") || chopV(z, "Yen") || chopV(z, "Yes") ||
              chopV(z, "ed") || chopV_try_u(z, "en") || chopV(z, "er") ||
              chopV_try_u(z, "es")) return true;
          break;
        case 'i':
          if (chopV(z, "ierais") || chopV(z, "ieseis") || chopV(z, "asteis") ||
              chopV(z, "isteis") || chopV(z, ".rIais") || chopV(z, "abais") ||
              chopV(z, "arais") || chopV(z, ".rEis") || chopV(z, "aseis") ||
              chopV(z, "Iais") || chopV(z, "Yais") || chopV(z, "Ais") ||
              chopV_try_u(z, "Eis") || chopV(z, "id") || chopV(z, "ir")||
              chopV(z, "iO")) return true;
          break;
        case 'I':
          if (chopV(z, ".rIa") || chopV(z, "Ia") || chopV(z, "Is")) return true;
          break;
        case 'o':
          if (chopV(z, "iEramos") || chopV(z, ".rIamos") || chopV(z, "iEsemos") ||
              chopV(z, "Abamos") || chopV(z, "Aramos") || chopV(z, ".remos") ||
              chopV(z, "Asemos") || chopV(z, "ieron") || chopV(z, "Yeron") ||
              chopV(z, "Iamos") || chopV(z, "Yamos") || chopV(z, "aron") ||
              chopV(z, "ados") || chopV(z, "idos") || chopV(z, "amos") ||
              chopV_try_u(z, "emos") || chopV(z, "imos")) return true;
          break;
        case 'r':
          if (chopV(z, "iera") || chopV(z, "ara") || chopV(z, ".rA") ||
              chopV(z, ".rE")) return true;
          break;
        case 's':
          if (chopV(z, "iese") || chopV(z, "ase")) return true;
          break;
        case 't':
          if (chopV(z, "aste") || chopV(z, "iste")) return true;
          break;
        case 'y':
          if (chopV(z, "Ya") || chopV(z, "Ye") || chopV(z, "Yo") ||
              chopV(z, "YO")) return true;
          break;
    } return false;
}

int remove_suffix(struct spanish_stemmer * z)
{   char * p = z->p;
    int ess = p[z->k-1] == 's';
    if (ess) z->k--;
    switch (p[z->k-2])
    {   case 'a':
          if (ess) break;
          if (chop2(z, "idad")) goto lab0;
        case 'd':
          if (!ess) break;
          if (chop2(z, "idade"))
        lab0 :
          {   if (chop2(z, "abil")) return true;
              if (chop2(z, "iv")) return true;
              chop2(z, "ic"); return true;
          }
          break;
        case 'c':
          if (chop2(z, "ico") || chop2(z, "ica")) return true;
          break;
        case 'I':
          if (ends(z, "logIa") && attach2(z, "log")) return true;
          break;
        case 'l':
          if (chop2(z, "able") || chop2(z, "ible")) return true;
          break;
        case 'm':
          if (chop2(z, "ismo")) return true;
          break;
        case 'O':
          if (ess) break;
          if (chop2(z, "aciOn")) { chop2(z, "ic"); return true; }
          if (ends(z, "uciOn") && attach2(z, "u")) return true;
          break;
        case 'n':
          if (!ess) break;
          if (chop2(z, "acione")) { chop2(z, "ic"); return true; }
          if (ends(z, "ucione") && attach2(z, "u")) return true;
          break;
        case 'o':
          if (ess) break;
          if (chop2(z, "ador")) { chop2(z, "ic"); return true; }
          break;
        case 'r':
          if (chop2(z, "adora") || (ess && chop2(z, "adore"))) { chop2(z, "ic"); return true; }
          break;
        case 's':
          if (chop2(z, "oso") || chop2(z, "osa")) return true;
          break;
        case 't':
          if (chop2(z, "ista")) return true;
          if (chop2(z, "amiento") || chop2(z, "imiento")) return true;
          if (ess) break;
          if (chopV(z, "amente"))  /* N.B. chopV here */
          {   if (chop2(z, "os")) return true;
              if (chop2(z, "ativ")) return true;
              if (chop2(z, "ad")) return true;
              if (chop2(z, "ic")) return true;
              chop2(z, "iv"); return true;
          }
          if (chop2(z, "mente"))
          {   if (chop2(z, "able") || chop2(z, "ible")) return true;
              return true;
          }
          break;
        case 'v':
          if (chop2(z, "ivo") || chop2(z, "iva")) { chop2(z, "at"); return true; }
          break;
        case 'z':
          if (chop2(z, "anza")) return true;
    }
    if (ess) z->k++; return false;
}

void tidy_up(struct spanish_stemmer * z)
{   switch (z->p[z->k-1])
    {   case 's':
          if (chopV(z, "os")) return;
          break;
        case 'e': case 'E':
          if (z->k-2 >= z->posV)
          {   z->k--;
              if (ends(z, "gu")) if (z->k-2 >= z->posV) z->k--;
          }
          break;
        case 'a': case 'o': case 'A': case 'I': case 'O':
          if (z->k-2 >= z->posV) z->k--;
    }
}


/* In spanish_stem(z, p, i, j), p is a struct spanish_stemmer pointer, and the
   string to be stemmed is from p[i] to p[j] inclusive. Typically i is zero
   and j is the offset to the last character of a string, (p[j+1] == '\0').
   The stemmer return a pointer to the stemmed form of this word in structure
   z.
*/

#define PAIR(a, b)   ((a)<<8|(b))

extern char * spanish_stem(struct spanish_stemmer * z, char * q, int i0, int i1)
{   char * p = z->p;
    int p_size = z->p_size;
    int error = false;
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
                {   case PAIR('a', 'a'):
                    case PAIR('e', 'a'):
                    case PAIR('i', 'a'):
                    case PAIR('o', 'a'):
                    case PAIR('u', 'a'): p[k-1] = toupper(letter); break;
                    case PAIR('n', 't'): p[k-1] = 'N'; break;

                 /* The only other accent combination of Spanish is
                    u-diaeresis, i.e. PAIR('u', 'u'), which is mapped
                    to 'u' at this point. */

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
    k = z->k;
    if (k > p_size) { k = p_size; error = true; }
    {   int i = 0;
        while (i != k)
        {   int ch = p[i];
            if (isupper(ch)) p[i] = tolower(ch);
            i++;
            if (ch == 'N')
            {   memmove(p+i+2, p+i, k-i); k += 2;
                p[i++] = '^'; p[i++] = 't';
            }
        }
    }
    p[k] = 0; /* C string form for now */
    z->k = k;
    if (error) printf("Term %s truncated\n", p);
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

    "dar" , "dar/"
    "doy/das/da/damos/dais/dan/dE/des/deis/den/darE/darAs/darA/"
    "daremos/darEis/darAn/darIa/darIas/darIamos/darIais/darIan/"
    "daba/dabas/dAbamos/dabais/daban/di/diste/dio/dimos/disteis/"
    "dieron/diera/dieras/diEramos/dierais/dieran/diese/dieses/"
    "diEsemos/dieseis/diesen/dando/dado/dada/dados/dadas/dad/"

            /* demos = towns */
  ,
    "estar" , "estar/"
    "estoy/estAs/estA/estamos/estAis/estAn/estE/estEs/estemos/estEis/"
    "estEn/estarE/estarAs/estarA/estaremos/estarEis/estarAn/estarIa/"
    "estarIas/estarIamos/estarIais/estarIan/estaba/estabas/"
    "estAbamos/estabais/estaban/estuve/estuviste/estuvo/estuvimos/"
    "estuvisteis/estuvieron/estuviera/estuvieras/estuviEramos/"
    "estuvierais/estuvieran/estuviese/estuvieses/estuviEsemos/"
    "estuvieseis/estuviesen/estando/estado/estada/estados/estadas/"
    "estad/"
  ,
    "haber" , "haber/"
    "he/has/ha/hemos/habEis/han/haya/hayas/hayamos/hayAis/hayan/habrE/"
    "habrAs/habrA/habremos/habrEis/habrAn/habrIa/habrIas/"
    "habrIamos/habrIais/habrIan/habIa/habIas/habIamos/habIais/habIan/"
    "hube/hubiste/hubo/hubimos/hubisteis/hubieron/hubiera/hubieras/"
    "hubiEramos/hubierais/hubieran/hubiese/hubieses/hubiEsemos/"
    "hubieseis/hubiesen/habiendo/habido/habida/habidos/habidas/"
  ,
    "hacer" , "hacer/"
    "hago/haces/hace/hacemos/hacEis/hacen/haga/hagas/hagamos/hagAis/"
    "hagan/harE/harAs/harA/haremos/harEis/harAn/harIa/harIas/harIamos/"
    "harIais/harIan/hacIa/hacIas/hacIamos/hacIais/hacIan/hice/"
    "hiciste/hizo/hicimos/hicisteis/hicieron/hiciera/hicieras/"
    "hiciEramos/hicierais/hicieran/hiciese/hicieses/hiciEsemos/"
    "hicieseis/hiciesen/haciendo/hecho/hecha/hechos/hechas/"

            /* haz = bunch / face */
  ,
    "ir" , "ir/"
    "voy/vas/va/vamos/vais/van/vaya/vayas/vayamos/vayAis/vayan/irE/"
    "irAs/irA/iremos/irEis/irAn/irIa/irIas/irIamos/irIais/irIan/iba/"
    "ibas/Ibamos/ibais/iban/"
    "yendo/"

            /* fui etc reappear in ser and are omitted from ir */
            /*ido ida idos idas = mad; ve id also troublesome */
  ,
    "saber" , "saber/"
    "sE/sabes/sabe/sabemos/sabEis/saben/sepa/sepas/sepamos/sepAis/"
    "sepan/sabrE/sabrAs/sabrA/sabremos/sabrEis/sabrAn/sabrIa/sabrIas/"
    "sabrIamos/sabrIais/sabrIan/sabIa/sabIas/sabIamos/sabIais/"
    "sabIan/supe/supiste/supo/supimos/supisteis/supieron/supiera/"
    "supieras/supiEramos/supierais/supieran/supiese/supieses/"
    "supiEsemos/supieseis/supiesen/sabiendo/sabido/sabida/sabidos/"
    "sabidas/sabed/"
  ,
    "ser" , "ser/"
    "soy/eres/es/somos/sois/son/sea/seas/seamos/seAis/sean/serE/serAs/"
    "serA/seremos/serEis/serAn/serIa/serIas/serIamos/serIais/serIan/"
    "era/eras/Eramos/erais/eran/fui/fuiste/fuimos/fuisteis/"
    "fueron/fuera/fueras/fuEramos/fuerais/fueran/fuese/fueses/"
    "fuEsemos/fueseis/fuesen/sintiendo/sentido/sentida/sentidos/"
    "sentidas/siente/sentid/"
  ,
    "tener" , "tener/"
    "tengo/tienes/tiene/tenemos/tenEis/tienen/tenga/tengas/tengamos/"
    "tengAis/tengan/tendrE/tendrAs/tendrA/tendremos/tendrEis/tendrAn/"
    "tendrIa/tendrIas/tendrIamos/tendrIais/tendrIan/tenIa/tenIas/"
    "tenIamos/tenIais/tenIan/tuve/tuviste/tuvo/tuvimos/"
    "tuvisteis/tuvieron/tuviera/tuvieras/tuviEramos/tuvierais/"
    "tuvieran/tuviese/tuvieses/tuviEsemos/tuvieseis/tuviesen/teniendo/"
    "tenido/tenida/tenidos/tenidas/tened/"
  ,
  0, 0  /* terminator */
};

extern struct spanish_stemmer * setup_spanish_stemmer()
{  struct spanish_stemmer * z = (struct spanish_stemmer *) malloc(sizeof(struct spanish_stemmer));
   z->p = 0; z->p_size = 0;
   z->irregulars = create_pool(irregular_forms);
   return z;
}

extern void closedown_spanish_stemmer(struct spanish_stemmer * z)
{  free_pool(z->irregulars);
   free(z->p);
   free(z);
}

