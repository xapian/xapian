
/* The English stemming algorithm is essentially the Porter stemming
 * algorithm, and has been coded up by its author. It follows the algorithm
 * presented in
 *
 * Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
 * no. 3, pp 130-137,
 *
 * only differing from it at the points marked -DEPARTURE- and -NEW-
 * below.
 *
 * For a more faithful version of the Porter algorithm, see
 *
 *     http://www.muscat.com/~martin/stem.html
 *
 */

#include <stdio.h>
#include <string.h>

#include "pool.h"
#include "stem_english.h"

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

/* cons(z, i) is true <=> p[i] is a consonant.
*/

int cons(struct stemmer * z, int i)
{   switch (z->p[i])
    {   case 'a': case 'e': case 'i': case 'o': case 'u':
            return false;
        case 'y':
            return (i==0) ? true : !cons(z, i - 1);
        default: return true;
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

int m(struct stemmer * z)
{   int n = 0;
    int i = 0;
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

/* vowelinstem(z) is true p[0], ... p[j] contains a vowel
*/

int vowelinstem(struct stemmer * z)
{   int i;
    for (i = 0; i <= z->j; i++) if (! cons(z, i)) return true;
    return false;
}

/* doublec(z, i) is true <=> p[i], p[i - 1] contain a double consonant.
*/

int doublec(struct stemmer * z, int i)
{   if (i < 1) return false;
    if (z->p[i] != z->p[i - 1]) return false;
    return cons(z, i);
}

/* cvc(z, i) is true <=> p[i - 2], p[i - 1], p[i] has the form consonant -
   vowel - consonant and also if the second c is not w, x or y. this is used
   when trying to restore an e at the end of a short word. e.g.

      cav(e), lov(e), hop(e), crim(e), but
      snow, box, tray.

*/

int cvc(struct stemmer * z, int i)
{   if (i < 2 || !cons(z, i) || cons(z, i - 1) || !cons(z, i - 2)) return false;
    {   int ch = z->p[i];
        if (ch == 'w' || ch == 'x' || ch == 'y') return false;
    }
    return true;
}

/* ends(z, s, length) is true <=> p[0], ... p[k] ends with the string s.
*/

int ends(struct stemmer * z, char * s, int length)
{
    if (length > z->k + 1) return false;
    if (memcmp(z->p + z->k - length + 1, s, length) != 0) return false;
    z->j = z->k - length;
    return true;
}

/* setto(z, s, length) sets p[j + 1] ... to the characters in the string s,
   readjusting k.
*/

void setto(struct stemmer * z, char * s, int length)
{
    memmove(z->p + z->j + 1, s, length);
    z->k = z->j + length;
}

/* r(z, s, length) is used further down. */

void r(struct stemmer * z, char * s, int length)
{
    if (m(z) > 0) setto(z, s, length);
}

/* step_1ab(z) gets rid of plurals and -ed or -ing. e.g.

       caresses  ->  caress
       ponies    ->  poni
       sties     ->  sti
       tie       ->  tie       (-NEW-: see below)
       caress    ->  caress
       cats      ->  cat

       feed      ->  feed
       agreed    ->  agree
       disabled  ->  disable

       matting   ->  mat
       mating    ->  mate
       meeting   ->  meet
       milling   ->  mill
       messing   ->  mess

       meetings  ->  meet

*/

void step_1ab(struct stemmer * z)
{   if (z->p[z->k] == 's')
    {   if (ends(z, "sses", 4)) z->k -= 2; else
        if (ends(z, "ies", 3))
            if (z->j == 0) z->k--; else z->k -= 2;

        /* this line extends the original algorithm, so that 'flies'->'fli' but
           'dies'->'die' etc */

        else
            if (z->p[z->k - 1] != 's') z->k--;
    }

    if (ends(z, "ied", 3)) { if (z->j == 0) z->k--; else z->k -= 2; } else

    /* this line extends the original algorithm, so that 'spied'->'spi' but
       'died'->'die' etc */

    if (ends(z, "eed", 3)) { if (m(z) > 0) z->k--; } else
    if ((ends(z, "ed", 2) || ends(z, "ing", 3)) && vowelinstem(z))
    {   z->k = z->j;
        if (ends(z, "at", 2)) setto(z, "ate", 3); else
        if (ends(z, "bl", 2)) setto(z, "ble", 3); else
        if (ends(z, "iz", 2)) setto(z, "ize", 3); else
        if (doublec(z, z->k))
        {   z->k--;
            {   int ch = z->p[z->k];
                if (ch == 'l' || ch == 's' || ch == 'z') z->k++;
            }
        }
        else if (m(z) == 1 && cvc(z, z->k)) setto(z, "e", 1);
    }
}

/* step_1c(z) turns terminal y to i when there is another vowel in the stem.

   -NEW-: This has been modified from the original Porter algorithm so that y->i
   is only done when y is preceded by a consonant, but not if the stem
   is only a single consonant, i.e.

       (*c and not c) Y -> I

   So 'happy' -> 'happi', but
      'enjoy' -> 'enjoy'  etc

   This is a much better rule. Formerly 'enjoy'->'enjoi' and 'enjoyment'->
   'enjoy'. Step 1c is perhaps done too soon; but with this modification that
   no longer really matters.

   Also, the removal of the vowelinstem(z) condition means that 'spy', 'fly',
   'try' ... stem to 'spi', 'fli', 'tri' and conflate with 'spied', 'tried',
   'flies' ...

*/

void step_1c(struct stemmer * z)
{
    if (ends(z, "y", 1) && z->j > 0 && cons(z, z->k - 1)) z->p[z->k] = 'i';
}


/* step_2(z) maps double suffices to single ones. so -ization ( = -ize plus
   -ation) maps to -ize etc. Note that the string before the suffix must give
   m(z) > 0.
*/

void step_2(struct stemmer * z)
{   switch (z->p[z->k - 1])
    {
        case 'a':
            if (ends(z, "ational", 7)) { r(z, "ate", 3); break; }
            if (ends(z, "tional", 6)) { r(z, "tion", 4); break; }
            break;
        case 'c':
            if (ends(z, "enci", 4)) { r(z, "ence", 4); break; }
            if (ends(z, "anci", 4)) { r(z, "ance", 4); break; }
            break;
        case 'e':
            if (ends(z, "izer", 4)) { r(z, "ize", 3); break; }
            break;
        case 'l':
            if (ends(z, "bli", 3)) { r(z, "ble", 3); break; } /*-DEPARTURE-*/

     /* To match the published algorithm, replace this line with
        case 'l':
            if (ends(z, "abli", 4)) { r(z, "able", 4); break; }
     */

            if (ends(z, "alli", 4)) { r(z, "al", 2); break; }
            if (ends(z, "entli", 5)) { r(z, "ent", 3); break; }
            if (ends(z, "eli", 3)) { r(z, "e", 1); break; }
            if (ends(z, "ousli", 5)) { r(z, "ous", 3); break; }
            break;
        case 'o':
            if (ends(z, "ization", 7)) { r(z, "ize", 3); break; }
            if (ends(z, "ation", 5)) { r(z, "ate", 3); break; }
            if (ends(z, "ator", 4)) { r(z, "ate", 3); break; }
            break;
        case 's':
            if (ends(z, "alism", 5)) { r(z, "al", 2); break; }
            if (ends(z, "iveness", 7)) { r(z, "ive", 3); break; }
            if (ends(z, "fulness", 7)) { r(z, "ful", 3); break; }
            if (ends(z, "ousness", 7)) { r(z, "ous", 3); break; }
            break;
        case 't':
            if (ends(z, "aliti", 5)) { r(z, "al", 2); break; }
            if (ends(z, "iviti", 5)) { r(z, "ive", 3); break; }
            if (ends(z, "biliti", 6)) { r(z, "ble", 3); break; }
            break;
        case 'g':
            if (ends(z, "logi", 4)) { r(z, "log", 3); break; } /*-DEPARTURE-*/

     /* To match the published algorithm, delete this line */

    }
}

/* step_3(z) deals with -ic-, -full, -ness etc. Similar strategy to step_2.
*/

void step_3(struct stemmer * z)
{   switch (z->p[z->k])
    {
        case 'e':
            if (ends(z, "icate", 5)) { r(z, "ic", 2); break; }
            if (ends(z, "ative", 5)) { r(z, "", 0); break; }
            if (ends(z, "alize", 5)) { r(z, "al", 2); break; }
            break;
        case 'i':
            if (ends(z, "iciti", 5)) { r(z, "ic", 2); break; }
            break;
        case 'l':
            if (ends(z, "ical", 4)) { r(z, "ic", 2); break; }
            if (ends(z, "ful", 3)) { r(z, "", 0); break; }
            break;
        case 's':
            if (ends(z, "ness", 4)) { r(z, "", 0); break; }
            break;
    }
}

/* step_4() takes off -ant, -ence etc., in context <c>vcvc<v>.
*/

void step_4(struct stemmer * z)
{   switch (z->p[z->k - 1])
    {   case 'a':
            if (ends(z, "al", 2)) break; return;
        case 'c':
            if (ends(z, "ance", 4)) break;
            if (ends(z, "ence", 4)) break; return;
        case 'e':
            if (ends(z, "er", 2)) break; return;
        case 'i':
            if (ends(z, "ic", 2)) break; return;
        case 'l':
            if (ends(z, "able", 4)) break;
            if (ends(z, "ible", 4)) break; return;
        case 'n':
            if (ends(z, "ant", 3)) break;
            if (ends(z, "ement", 5)) break;
            if (ends(z, "ment", 4)) break;
            if (ends(z, "ent", 3)) break; return;
        case 'o':
            if (ends(z, "ion", 3) && (z->p[z->j] == 's' ||
                                    z->p[z->j] == 't')) break;
            if (ends(z, "ou", 2)) break; return;
            /* takes care of -ous */
        case 's':
            if (ends(z, "ism", 3)) break; return;
        case 't':
            if (ends(z, "ate", 3)) break;
            if (ends(z, "iti", 3)) break; return;
        case 'u':
            if (ends(z, "ous", 3)) break; return;
        case 'v':
            if (ends(z, "ive", 3)) break; return;
        case 'z':
            if (ends(z, "ize", 3)) break; return;
        default:
            return;
    }
    if (m(z) > 1) z->k = z->j;
}

/* step_5(z) removes a final -e if m(z) > 1, and changes -ll to -l if
   m(z) > 1.
*/

void step_5(struct stemmer * z)
{   z->j = z->k;
    if (z->p[z->k] == 'e')
    {   int a = m(z);
        if (a > 1 || a == 1 && !cvc(z, z->k - 1)) z->k--;
    }
    if (z->p[z->k] == 'l' && doublec(z, z->k) && m(z) > 1) z->k--;
}

extern char * stem(struct stemmer * z, char * q, int i0, int i1)
{
    int p_size = z->p_size;

    if (i1 - i0 + 50 > p_size)
    {   free(z->p);
        p_size = i1 - i0 + 75; /* ample */ z->p_size = p_size;
        z->p = (char *) malloc(p_size);
    }

    memmove(z->p, q + i0, i1 - i0 + 1);

    z->k = i1 - i0;


    {   char * t = search_pool(z->irregulars, z->k + 1, z->p);
        if (t != 0) return t;
    }

    if (z->k > 1) /*-DEPARTURE-*/

   /* With this line, strings of length 1 or 2 don't go through the
      stemming process, although no mention is made of this in the
      published algorithm. Remove the line to match the published
      algorithm. */

    {   step_1ab(z); step_1c(z);
        step_2(z);
        step_3(z);
        step_4(z);
        step_5(z);
    }

    z->p[z->k + 1] = 0; /* C string form for now */
    return z->p;
}

/* -NEW-
   This is a table of irregular forms. It is quite short, but still
   reflects the errors actually drawn to Martin Porter's attention over
   a 20 year period!

   Extend it as necessary.

   The form of the table is:

     "p1" "s11/s12/s13/ ... /"
     "p2" "s21/s22/s23/ ... /"
     ...
     "pn" "sn1/sn2/sn3/ ... /"
     0, 0

   String sij is mapped to paradigm form pi, and the main stemming
   process is then bypassed.
*/

static char * irregular_forms[] = {

    "sky",     "sky/skies/",
    "die",     "dying/",
    "lie",     "lying/",
    "tie",     "tying/",
    "news",    "news/",
    "inning",  "innings/inning/",
    "outing",  "outings/outing/",
    "canning", "cannings/canning/",
    "howe",    "howe/",
    0, 0  /* terminator */

};

extern struct stemmer * setup_stemmer()
{   struct stemmer * z = (struct stemmer *) malloc(sizeof(struct stemmer));
    z->p = 0; z->p_size = 0;
    z->irregulars = create_pool(irregular_forms);
    return z;
}

extern void closedown_stemmer(struct stemmer * z)
{   free_pool(z->irregulars);
    free(z->p);
    free(z);
}


