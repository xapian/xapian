/* stem_portuguese.c: Portuguese stemming algorithm.
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
#include "stem_portuguese.h"

struct portuguese_stemmer
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

/*
void debug(struct portuguese_stemmer * z, int n)
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

static int char_type(int ch)
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

static int measure(struct portuguese_stemmer * z)
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

static int aei(int ch) { return (ch == 'a' || ch == 'e' || ch == 'i'); }
static int AEI(int ch) { return (ch == 'A' || ch == 'E' || ch == 'I'); }

static int look_for(struct portuguese_stemmer * z, char * s)
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
    if (firstch == ':')
    {   if (!AEI(p[jbase])) return false;
        o = 1;
    }
    if (memcmp(s+o, p+jbase+o, length-o) != 0) return false;
    z->j -= length;
    return true;
}

/* ends(z, s) applies look_for(z, s) at the end of the word, i.e. with j = k-1. */

static int ends(struct portuguese_stemmer * z, char * s)
{  z->j = z->k - 1;
   return look_for(z, s);
}

/* setto(z, s) sets (j+1), ... k to the characters in the string s, readjusting
   k.
*/

static void setto(struct portuguese_stemmer * z, char * s)
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

*/

/*
static int attachV(struct portuguese_stemmer * z, char * s)
{   if (z->j < z->posV) return false;
    setto(z, s);
    return true;
}
*/

static int after_posV(struct portuguese_stemmer * z)
{   if (z->j < z->posV) return false;
    z->k = z->j+1;
    return true;
}

static int chopV(struct portuguese_stemmer * z, char * s) { return ends(z, s) && after_posV(z); }


/* chopV_not_e is like chopV, and for the endings -eira(s), where presence of
   'e' prevents removal of the following ira(s).
*/

static int chopV_not_e(struct portuguese_stemmer * z, char * s)
{   if (!ends(z, s)) return false;
    if (z->p[z->j + 1] == 'i' && z->p[z->j] == 'e') return false;
    return after_posV(z);
}

static int attach2(struct portuguese_stemmer * z, char * s)
{   if (z->j < z->pos2) return false;
    setto(z, s);
    return true;
}

static int after_pos2(struct portuguese_stemmer * z)
{   if (z->j < z->pos2) return false;
    z->k = z->j+1;
    return true;
}

static int chop2(struct portuguese_stemmer * z, char * s) { return ends(z, s) && after_pos2(z); }

/* attached pronouns can of course double, but apart from
   -se + lo, la, los, las they seem to be v. rare, and are ignored here.

*/

/*
                                           A = a'
   verb_ending(z) is searching for one of the following system of endings,

   1st conjugation

     Infinitive  -ar

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -as     -a      -amos   -ais    -am
     Subjunctive -e      -es     -e      -emos   -eis    -em
     Future      -arei   -arAs   -arA    -aremos -areis  -araNo
     Conditional -aria   -arias  -aria   -arIamos-arIeis -ariam
     Imperfect   -ava    -avas   -ava    -Avamos -Aveis  -avam
     Past        -ei     -aste   -ou     -Amos   -astes  -aram
     Imp.Subj.   -asse   -asses  -asse   -Assemos-Asseis -assem

     Pluperfect  -ara    -aras   -ara    -Aramos -Areis  -aram

     Pres.Part.  -ando
     Past.Part.  -ado -ada -ados -adas

   2nd conjugation

     Infinitive  -er

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -es     -e      -emos   -eis    -em
     Subjunctive -a      -as     -a      -amos   -ais    -am
     Future      -erei   -erAs   -erA    -eremos -ereis  -eraNo
     Conditional -eria   -erias  -eria   -erIamos-erIeis -eriam
     Imperfect   -ia     -ias    -ia     -Iamos  -Ieis   -iam
     Past        -i      -este   -eu     -emos   -estes  -eram
     Imp.Subj.   -esse   -esses  -esse   -Essemos-Esseis -essem

     Pluperfect  -era    -eras   -era    -Eramos -Ereis  -eram

     Pres.Part.  -endo
     Past.Part.  -ido -ida -idos -idas

   3rd conjugation

     Infinitive  -ir

                  1s      2s      3s      1p      2p      3p
                 ----------------------------------------------
     Present     -o      -es     -e      -imos   -is     -em
     Subjunctive -a      -as     -a      -amos   -ais    -am
     Future      -irei   -irAs   -irA    -iremos -ireis  -iraNo
     Conditional -iria   -irias  -iria   -irIamos-irIeis -iriam
     Imperfect   -ia     -ias    -ia     -Iamos  -Ieis   -iam
     Past        -i      -iste   -iu     -imos   -istes  -eram
     Imp.Subj.   -isse   -isses  -isse   -Issemos-Isseis -issem

     Pluperfect  -ira    -iras   -ira    -Iramos -Ireis  -iram

     Pres.Part.  -indo
     Past.Part.  -ido -ida -idos -idas


   Second person plural forms (2p) are included in the algorithm,
   despite being almost obsolete in modern Portuguese.

   [Endings 'Yas', 'Yan' etc are for -ir verbs with stem ending 'u': e.g.
   'construir'. In most grammars these are classed as irregular.]


*/


static int verb_ending(struct portuguese_stemmer * z)
{   switch (z->p[z->k-2])
    {   case 'a':
          if (chopV(z, ".riam") || chopV(z, ".rias") || chopV(z, "avam") ||
              chopV(z, "avas") || chopV_not_e(z, ".ras") || chopV(z, "adas") ||
              chopV(z, ".ram") || chopV(z, "idas") || chopV(z, "iam") ||
              chopV(z, "ias") || chopV(z, "am") || chopV(z, "ar") ||
              chopV(z, "as")) return true;
          break;
        case 'N':
          if (chopV(z, ".raNo")) return true;
          break;
        case 'A':
          if (chopV(z, ".rAs")) return true;
          break;
        case 'v':
          if (chopV(z, "ava")) return true;
          break;
        case 'd':
          if (chopV(z, ".ndo") || chopV(z, "ido") || chopV(z, "ida") ||
              chopV(z, "ado") || chopV(z, "ada")) return true;
          break;
        case 'e':
          if (chopV(z, ".ssem") || chopV(z, ".sses") || chopV(z, ".stes") ||
              chopV(z, ".rdes") || chopV(z, ".rei") || chopV(z, ".res") ||
              chopV(z, ".rem") || chopV(z, "em") || chopV(z, "er") ||
              chopV(z, "es") || chopV(z, "eu") || chopV(z, "ei")) return true;
          break;
        case 'i':
          if (chopV(z, ":sseis") || chopV(z, ".rIeis") || chopV(z, "Aveis") ||
              chopV(z, ".reis") || chopV(z, ":reis") || chopV(z, "Ieis") ||
              chopV(z, ".ria") || chopV(z, "ais") || chopV(z, "eis") ||
              chopV(z, "ir") || chopV(z, "ia") || chopV(z, "is") ||
              chopV(z, "iu")) return true;
          break;
        case 'o':
          if (chopV(z, ".rIamos") || chopV(z, ":ssemos") || chopV(z, "Avamos") ||
              chopV(z, ".remos") || chopV(z, ":ramos") || chopV(z, "Iamos") ||
              chopV(z, ".rmos") || chopV(z, "ados") || chopV(z, "idos") ||
              chopV(z, "Amos") || chopV(z, ".mos") || chopV(z, "ou")) return true;
          break;
        case 'r':
          if (chopV(z, ".rA") || chopV_not_e(z, ".ra")) return true;
          break;
        case 's':
          if (chopV(z, ".sse")) return true;
          break;
        case 't':
          if (chopV(z, ".ste")) return true;
          break;
    } return false;
}

static int remove_suffix(struct portuguese_stemmer * z)
{   char * p = z->p;
    int ess = p[z->k-1] == 's';
    if (ess) z->k--;
    switch (p[z->k-2])
    {   case 'd':
          if (chop2(z, "idade"))                       /* -ITY */
          {   if (chop2(z, "abil")) return true;       /* -ABILITY */
              if (chop2(z, "iv")) return true;         /* -IVITY */
              chop2(z, "ic"); return true;             /* -ICITY */
          }
          break;
        case 'c':                                      /* -IC */
          if (chop2(z, "ico") || chop2(z, "ica")) return true;
          break;
        case 'e':
          if (chop2(z, "Avel") ||                      /* -ABLE */
              chop2(z, "Ivel")) return true;           /* -IBLE */
          break;
        case 'm':
          if (chop2(z, "ismo")) return true;           /* -ISM */
          break;
        case 'N':
          if (ess)
          {    if (chop2(z, "aCoNe")) return true;     /* -ATIONS */
          }
          else
          {    if (chop2(z, "aCaNo")) return true;     /* -ATION */
          }
          /* no -ICATION test */
          break;
        case 'o':
          if (ess) break;
          if (chop2(z, "ador")) return true;           /* -ATOR */
          /* no -ICATOR test */
          break;
        case 'r':
          if (chop2(z, "adora") ||
              (ess && chop2(z, "adore"))) return true; /* -ATOR */
          break;
        case 's':
          if (chop2(z, "oso") ||                       /* -OUS */
              chop2(z, "osa")) return true;
          break;
        case 't':
          if (chop2(z, "ista")) return true;           /* -IST */
          if (chop2(z, "amento") ||                    /* -AMENT */
              chop2(z, "imento")) return true;         /* -IMENT */
          if (ess) break;
          if (chopV(z, "amente"))                      /* -ALLY */
             /* N.B. chopV here */
          {   if (chop2(z, "os")) return true;         /* -OUSLY */
              if (chop2(z, "ativ")) return true;       /* -ATIVELY */
              if (chop2(z, "ad")) return true;
              if (chop2(z, "ic")) return true;         /* -ICALLY */
              chop2(z, "iv"); return true;             /* -IVELY */
          }
          if (chop2(z, "mente"))                       /* -LY */
          {   if (chop2(z, "avel") ||                  /* -ABLY */
                  chop2(z, "Ivel")) return true;       /* -IBLY */
              return true;
          }
          break;
        case 'v':
          if (chop2(z, "ivo") || chop2(z, "iva"))      /* -IVE */
          {   chop2(z, "at"); return true;             /* -ATIVE */
          }
          break;
        case 'z':
          if (chop2(z, "eza")) return true;            /* -ANCE */
    }
    if (ess) z->k++; return false;
}

static void tidy_up(struct portuguese_stemmer * z, int suffix_removed)
{   switch (z->p[z->k-1])
    {   case 's':
          if (!suffix_removed && chopV(z, "os")) return;
          break;
        case 'e': case 'E':
          if (z->k-2 >= z->posV)
          {   z->k--;
              if (ends(z, "gu") ||
                  ends(z, "ci")) if (z->k-2 >= z->posV) z->k--;
          }
          break;
        case 'i':
          /* remove terminal -i or -i after -c */
          if ((!suffix_removed || ends(z, "ci")) && z->k-2 >= z->posV) z->k--;
          break;
          /* remove terminal vowels */
        case 'a': case 'o': case 'A': case 'I': case 'O':
          if (!suffix_removed && z->k-2 >= z->posV) z->k--;
    }
}


/* In portuguese_stem(z, p, i, j), p is a struct portuguese_stemmer pointer, and the
   string to be stemmed is from p[i] to p[j] inclusive. Typically i is zero
   and j is the offset to the last character of a string, (p[j+1] == '\0').
   The stemmer return a pointer to the stemmed form of this word in structure
   z.
*/

#define PAIR(a, b)   ((a)<<8|(b))

extern char * portuguese_stem(struct portuguese_stemmer * z, char * q, int i0, int i1)
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
                {   case PAIR('c', 'c'):
                    case PAIR('a', 'a'):
                    case PAIR('e', 'h'):
                    case PAIR('e', 'a'):
                    case PAIR('i', 'a'):
                    case PAIR('o', 'h'):
                    case PAIR('o', 'a'):
                    case PAIR('u', 'a'): p[k-1] = toupper(letter); break;
                    case PAIR('a', 't'):
                    case PAIR('o', 't'): p[k++] = 'N'; break;
                } j++;
            } else p[k++] = ch;
        }
    }
    z->k = k;

    {   char * t = search_pool(z->irregulars, k, p);
        if (t != 0) return t;
    }

    measure(z);
    {   int suffix_removed = remove_suffix(z);
        if (!suffix_removed) suffix_removed = verb_ending(z);
        tidy_up(z, suffix_removed);
    }
    k = z->k;
    if (k > p_size) { k = p_size; error = true; }
    {   int i = 0;
        while (i != k)
        {   int ch = p[i];
            if (isupper(ch)) p[i] = tolower(ch);
            i++;
            if (ch == 'N')
            {   memmove(p+i+1, p+i, k-i); k += 1;
                p[i-1] = '^'; p[i++] = 't';
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

    "dar" ,
    "dou/dA/damos/daNo/"
    "dei/deu/demos/deram/"
    "dera/dEramos/"
    "dE/dEem/"
    "desse/dEssemos/dessem/"
    "der/dermos/derem/"
  ,
    "diz" ,
    "digo/diz/dizemos/dizem/"
    "disse/dissemos/disseram/"
    "dissera/dissEramos/"
    "direi/dirA/diremos/diraNo/"
    "diria/dirIamos/diriam/"
  ,
    "est" ,
    "estou/estA/estamos/estaNo/"
    "estive/esteve/estivemos/estiveram/"
    "estivera/estivEramos/"
    "esteja/estejamos/estejam/"
    "estivesse/estivEssemos/estivessem/"
    "estiver/estivermos/estiverem/"
  ,
    "faz" ,   /* forms reduce to 'faz', 'fiz' */
    "faCo/faz/"
    "fez/"
    "farei/farA/faremos/faraNo/"
    "faria/farIamos/fariam/"
    "faCa/faCamos/faCam/"
  ,
    "hav" ,   /* forms reduce to 'hav', 'houv' */
    "hei/hA/haNo/"
    "haja/hajamos/hajam/"
  ,
    "ir" ,
    "vou/vai/vamos/vaNo/"
    "vA/"
  ,
    "pod" ,   /* forms reduce to 'pod', 'pud' */
    "posso/"
    "pOde/"
    "possa/possamos/possam/"
  ,
    "sab" ,   /* forms reduce to 'sab', 'soub' */
    "sei/"
    "saiba/saibamos/saibam/"
  ,
    "ser" ,
    "sou/somos/saNo/"
    "era/Eramos/eram/"
    "fui/foi/fomos/foram/"
    "fora/fOramos/"
    "seja/sejamos/sejam/"
    "fosse/fOssemos/fossem/"
    "for/formos/forem/"
    "serei/serA/seremos/seraNo/seria/serIamos/seriam/"
  ,
    "ter" ,
    "tenho/tem/temos/tEm/"
    "tinha/tInhamos/tinham/"
    "tive/teve/tivemos/tiveram/"
    "tivera/tivEramos/"
    "tenha/tenhamos/tenham/"
    "tivesse/tivEssemos/tivessem/"
    "tiver/tivermos/tiverem/"
    "terei/terA/teremos/teraNo/teria/terIamos/teriam/"
  ,
    "ver" ,
    "vejo/vE/vemos/vEem/"
    "vi/viu/viram/"        /* but 'vimos' -> 'vir' */
    "vira/vIramos/"
    "veja/vejamos/vejam/"
    "visse/vIssemos/vissem/"
    "vir/virmos/virem/"
    "verei/verA/veremos/veraNo/veria/verIamos/veriam/"
  ,
    "vir" ,
    "venho/vem/vimos/vEm/"
    "vinha/vInhamos/vinham/"
    "vim/veio/viemos/vieram/"
    "viera/viEramos/"
    "venha/venhamos/venham/"
    "viesse/viEssemos/viessem/"
    "vier/viermos/vierem/"
    "virei/virA/viremos/viraNo/viria/virIamos/viriam/"
  ,

  0, 0  /* terminator */
};

extern struct portuguese_stemmer * setup_portuguese_stemmer()
{  struct portuguese_stemmer * z = (struct portuguese_stemmer *) malloc(sizeof(struct portuguese_stemmer));
   z->p = 0; z->p_size = 0;
   z->irregulars = create_pool(irregular_forms);
   return z;
}

extern void closedown_portuguese_stemmer(struct portuguese_stemmer * z)
{  free_pool(z->irregulars);
   free(z->p);
   free(z);
}

