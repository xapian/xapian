/* DBtest5.c: Test harness for old muscat compatibility code
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
#include <fcntl.h>   /* O_RDONLY etc */
#include <stdlib.h>  /* exit etc */
#include "3point6.h"
#include "dbread.h"

#define true 1
#define false 0

void print_kform(char * k)
{
    {  int i; for (i=1; i < k[0]; i++) printf("%c",k[i]);  }
    printf("; ");
}


static void check_terms_of(struct DB_file * DB, struct termvec * tv)
{

    struct DB_term_info t;

    struct DB_postings * q[50];
    int i = 0;
    int j;

    M_open_terms(tv);

    while(true)
    {
        M_read_terms(tv);
        if (!(tv->term)) break;
        if (i == 50) break;
        if (! DB_term(tv->term, &t, DB))
        {   fprintf(stderr, "Cant find %s\n", tv->term);
            exit(1);
        }

        q[i++] = DB_open_postings(&t, DB);
        print_kform(tv->term);
    }
    printf("\n");

    while(true)
    {   int all_done = true;
        for (j = 0; j < i; j++)
        {   DB_read_postings(q[j], 0, 0);
            if (q[j]->Doc != MAXINT)
            {   all_done = false;
                printf("%d(%d); ", q[j]->Doc, q[j]->wdf);
            }
            else printf("-; ");
            fflush(stdout);
        }
        printf("\n");
        if (all_done) break;
    }
    printf("\n");

    for (j = 0; j < i; j++) DB_close_postings(q[j]);

}

int main(int argc, byte * argv[])
{   int x = HEAVY_DUTY;
    if (argc == 1) { printf("No argument\n"); exit(1); }
    {   struct DB_file * p;
        struct termvec * tv;
        p = DB_open(argv[1], 30, x);
        if (p == NULL) { printf("Can't open %s\n",argv[1]); exit(1); }
        tv = M_make_termvec();
        {   int i; for (i = 1024; i <= 1027; i++)
            {   printf("CYCLE %d\n",i);
                if (! DB_get_termvec(p, i, tv)) break;
                check_terms_of(p, tv);
            }
        }
        M_lose_termvec(tv);
        DB_close(p);
    }
    return 0;
}

