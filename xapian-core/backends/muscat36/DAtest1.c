/* DAtest1.c: Test harness for old muscat compatibility code
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

#include <stdio.h>   /* main etc */
#include <stdlib.h>  /* exit etc */
#include <string.h>  /* memmove, memcmp */
#include "3point6.h"
#include "daread.h"

void printkform(char * k)
{   printf("[");
    {  int i; for (i=1; i < k[0]; i++) printf("%c", k[i]);  }
    printf("]");
}

void findterm(struct DA_file * p, struct DA_term_info * t, byte * k, int compact)
{   int found = DA_term(k, t, p);
    printkform(k);
    printf(" %d\n", found);
    if (!found) return;

    printkform(t->term);
    printf("  term frequency %d\n", t->freq);
    printf("posting-block number %d\n", t->pn);
    printf("posting-block offset %d\n", t->po);
    printf("postings size %d\n", t->psize);
    printf("size of shortcut vector %d\n", t->shsize);
    printf("number of items in shortcut vector %d\n", t->shcount);

    printf("term-block index offset %d\n", t->o);
    printf("term-block number %d\n", t->n);
    printf("1 for first term %d\n", t->termno);

    {   struct DA_postings * q = DA_open_postings(t, p);
        int i; for(i=1;;i++)
        {   DA_read_postings(q, compact, 0);
            /* (q, 1, 0) for compact ranges */
            /* (q, 0, 0) for expanded ranges */
            if (q->Doc == MAXINT) break;
            if (q->Doc == q->E) printf("%d (%d) ", q->Doc, q->wdf);
                           else printf("%d-%d (%d) ", q->Doc, q->E, q->wdf);
            if (i % 5 == 0) printf("\n");
        }
        printf("\n");
        DA_close_postings(q);
    }

}

void makekform(char * s, int n, byte * b)
{   b[0] = sprintf(b + 1, "%s%d", s, n) + 1;
    printf("%d[%s]\n", b[0], b+1);
}

int main(int argc, byte * argv[])
{
    int x = HEAVY_DUTY;
    if (argc == 1) { printf("No argument\n"); exit(1); }
    printf("[%s]\n", argv[1]);
    {   struct DA_file * p;
        struct DA_term_info t;
        byte * b = (byte *) malloc(1000);
        p = DA_open(argv[1], DA_TERMS, x);
        if (p == NULL) { printf("Can't open %s\n", argv[1]); exit(1); }

        makekform("p", 7, b); findterm(p, &t, b, 1);
        makekform("s", 121, b); findterm(p, &t, b, 0);
        makekform("s", 777, b); findterm(p, &t, b, 1);
        makekform("s", 678, b); findterm(p, &t, b, 0);
        makekform("s", 11, b); findterm(p, &t, b, 1);
        makekform("s", 13, b); findterm(p, &t, b, 0);

        free(b);
        DA_close(p);
    }
    return 0;
}

