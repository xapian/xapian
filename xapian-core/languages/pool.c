/* pool.c: Code for managing exception lists for stemming algorithms
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
 *
 * This code may well be replaced at some future point: it is slightly
 * messy and unsatisfactory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pool.h"

#define true 1
#define false 0

/*  This is used as a library to resolve exceptions in the various
    stemming algorithms. Typical use is,

        struct pool * p = create_pool(t);
        char * s_translated = search_pool(p, strlen(s), s);
        ...
        free_pool(p);

    t is an array of strings, e.g.

        static char * t[] = {

            "sky",     "sky/skies/",
            "die",     "dying/",
            "lie",     "lying/",
            "tie",     "tying/",
            ....
            0, 0

        };

    if s is "sky", "skies", "dying" etc., translated_s is becomes "sky",
    "sky", "die" etc.

    The code includes a sort/merge capability which may be turned into
    (or replaced by) something more general later on.

*/

/*  merge(n, p, q, r, l, k, f) repeatedly merges n-byte sequences of items of
    size k from addresses p and q into r. f is the comparison routine and
    l is the limit point for q.
*/

static void merge(int n, char * p, char * q, char * r, char * l, int k,
                  int (*f)(char *, char *))
{  char * q0 = q;
   if (q0 > l) { memmove(r, p, l-p); return; }
   while (p < q0)
   {  char * pl = n+p;
      char * ql = n+q;
      if (ql > l) ql = l;
      while(true)
      {  if (p >= pl) { memmove(r, q, ql-q); r += ql-q; q = ql; break; }
         if (q >= ql) { memmove(r, p, pl-p); r += pl-p; p = pl; break; }
         if (f(p, q)) { memmove(r, p, k); p += k; }
                else { memmove(r, q, k); q += k; }
         r += k;
      }
   }
   memmove(r, q, l-q);
}

/*  In sort(p, c, k, f), p+c is a byte address at which begin a sequence of
    items of size k to be sorted. p+l is the address of the byte after the
    last of these items, so l - c is divisible by k. f is a comparison function
    for a pair of these items: f(p+i, q+j) is true if the item at p+i is before
    the item at q+j, false if it is equal to or after it.
*/

static void sort(char * p, int c, int l, int k,
                 int (*f)(char *, char *))
{
    char * q = malloc(l-c);  /* temporary work space */
    int j = k;
    int w = l-c;
    while (j < w)
    {   int cycle;
        for (cycle = 1; cycle <= 2; cycle++)
        {   int h = (w+j-1) / j / 2 * j;     /* half way */
            if (cycle == 1) merge(j, p+c, p+c+h, q, p+l, k, f);
                       else merge(j, q, q+h, p+c, q+w, k, f);
            j *= 2;
        }
    }
    free(q);
}

struct pool_entry {

    const char * translation;
    const char * pointer;
    int length;

};

static void print_entry(struct pool_entry * p)
    {
       { int j; for (j=0;j<p->length;j++) fprintf(stderr, "%c", (p->pointer)[j]); }
       fprintf(stderr, " --> %s\n", p->translation);
    }

/*  - debugging aid
    static void print_pool(struct pool * p)
    {   int i;
        int size = p->size;
        struct pool_entry * q = p->entries;
        fprintf(stderr, "\nPool:\n");
        for (i = 0; i < size; i++) print_entry(q+i);
    }
*/

/* compare(p, q) is our comparison function, used for f above
*/

static int compare(char * char_p, char * char_q)
{   struct pool_entry * p = (struct pool_entry *) char_p;
    struct pool_entry * q = (struct pool_entry *) char_q;
    if (p->length == q->length) return memcmp(p->pointer, q->pointer, p->length) < 0;
    return p->length < q->length;
}

static int count_slashes(const char * s[])
{   int slash_count = 0;
    int i;
    for (i = 1; s[i] != 0; i += 2)
    {   const char * p = s[i];
        int j = 0;
        while (p[j] != 0) if (p[j++] == '/') slash_count++;
    }
    return slash_count;
}

extern struct pool * create_pool(const char * s[])
{   int size = count_slashes(s);
    struct pool_entry * z = (struct pool_entry *) malloc(size * sizeof(struct pool_entry));
    struct pool_entry * q = z;
    int i;
    for (i = 1; s[i] != 0; i += 2)
    {   const char * p = s[i];
        int j = 0;
        int j0 = 0;
        while(true)
        {   if (p[j] == 0)
            {  if (j0 != j) { fprintf(stderr, "%s lacks final '/'\n", p); exit(1); }
               break;
            }
            if (p[j] == '/')
            {
                q->translation = s[i-1];
                q->pointer = p+j0; q->length = j-j0;
                q++;
                j0 = j+1;
            }
            j++;
        }
    }
    sort((char *) z, 0, size * sizeof(struct pool_entry), sizeof(struct pool_entry), compare);

    /* now validate the contents */

    for (i = 1; i < size; i++)
    {   struct pool_entry * p = z+i;
        struct pool_entry * q = z+i-1;
        if (p->length == q->length && memcmp(p->pointer, q->pointer, p->length) == 0)
        {   fprintf(stderr, "warning: "); print_entry(p);
            fprintf(stderr, " and "); print_entry(q);
        }
    }

    {   struct pool * p = (struct pool *) malloc(sizeof(struct pool));
        p->entries = z;
        p->size = size;
        return p;
    }
}

static int compare_to_pool(int length, const char * s, int length_p, const char * s_p)
{   if (length != length_p) return length-length_p;
    return memcmp(s, s_p, length);
}

extern const char * search_pool(struct pool * p, int length, char * s)
{   int i = 0;
    int j = p->size;
    struct pool_entry * q = p->entries;
    if (j == 0) return 0;  /* empty pool */
    if (compare_to_pool(length, s, q->length, q->pointer) < 0) return 0;
    while(true)
    {
        int h = (i+j)/2;
        int diff = compare_to_pool(length, s, (q+h)->length, (q+h)->pointer);
        if (diff == 0) return (q+h)->translation;
        if (j-i <= 1) return 0;
        if (diff < 0) j = h; else i = h;
    }
}

extern void free_pool(struct pool * p)
{   free(p->entries);
    free(p);
}

