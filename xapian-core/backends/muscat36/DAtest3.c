/* DAtest3.c: Test harness for old muscat compatibility code
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
#include "daread.h"

void printkform(char * k)
{  printf("[");
   {  int i; for (i=1; i < k[0]; i++) printf("%c", k[i]);  }
   printf("]");
}

int main(int argc, byte * argv[])
{   int x = HEAVY_DUTY;
    if (argc == 1) { printf("No argument\n"); exit(1); }
    {   struct DA_file * p;
        struct termvec * tv;
        p = DA_open(argv[1], DA_RECS, x);
        if (p == NULL) { printf("Can't open %s\n", argv[1]); exit(1); }
        tv = M_make_termvec();
        {   int i; for (i = 1; i <= 1000; i++)
            {   printf("CYCLE %d\n", i);
                if (! DA_get_termvec(p, i, tv)) break;
                M_open_terms(tv);
                while(1)
                {   M_read_terms(tv);
                    if (!(tv->term)) break;
                    printkform(tv->term);
                    if (tv->rel) printf(" r");
                    if (tv->freq != -1) printf(" freq=%d", tv->freq);
                    if (tv->wdf != 0) printf(" wdf=%d", tv->wdf);

                    if (tv->termp)
                    {   byte * p = tv->termp;
                        int l = L2(p, 0);
                        int i;
                        for (i=2; i<l; i = i + LWIDTH(x) + 1)
                           printf(" %d/%d", LENGTH_OF(p, i, x), p[i + LWIDTH(x)]);
                    }
                    printf("\n");
                }
            }
            printf("\n");
        }
        M_lose_termvec(tv);
        DA_close(p);
    }
   return 0;
}

