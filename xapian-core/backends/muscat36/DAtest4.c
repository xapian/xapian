/* DAtest4.c: Test harness for old muscat compatibility code
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

/* this is for testing the shortcut feature */

#include <stdio.h>   /* main etc */
#include <stdlib.h>  /* exit etc */
#include <string.h>  /* memmove, memcmp */
#include "3point6.h"
#include "daread.h"

void printkform(char * k)
{  printf("[");
   {  int i; for (i=1; i < k[0]; i++) printf("%c",k[i]);  }
   printf("]");
}

void checkterm(struct DAfile * p, struct DAterminfo * t, int i)
{     if (t->shsize == 0) return;
      printf("Cycle %d\n",i);
      printkform(t->term);
      printf("  term frequency %d\n",t->freq);
      printf("size of shortcut vector %d\n",t->shsize);
      printf("number of items in shortcut vector %d\n",t->shcount);

      printf("posting-block number %d\n",t->pn);
      printf("posting-block offset %d\n",t->po);
      printf("postings size %d\n",t->psize);

      printf("term-block index offset %d\n",t->o);
      printf("term-block number %d\n",t->n);
      printf("1 for first term %d\n",t->termno);

      {  struct DApostings * q = DAopenpostings(t,p);
         int i; for(i=1;;i++)
         {  DAreadpostings(q,1,34000);      /* or (q,0,0) for expanded ranges */
            if (q->Doc == MAXINT) break;
            if (q->Doc == q->E) printf("%d (%d) ",q->Doc, q->wdf);
                           else printf("%d-%d (%d) ",q->Doc, q->E, q->wdf);
            if (i % 5 == 0) printf("\n");
         }
         printf("\n");
         DAclosepostings(q);
      }

}


void makekform(char * s, byte * b)
{  int len = strlen(s);
   memmove(b+1,s,len); b[0] = len+1;
}

int main(int argc, byte * argv[])
{  int x = FLIMSY;
   if (argc == 1) { printf("No argument\n"); exit(1); }
   printf("[%s]\n",argv[1]);
   {  struct DAfile * p;
      struct DAterminfo t;
      byte * b = (byte *) malloc(1000);
      p = DAopen(argv[1], DATERMS, x);
      if (p == NULL) { printf("Can't open %s\n",argv[1]); exit(1); }


      makekform("compani",b); DAterm(b,&t,p); checkterm(p,&t,1);
      makekform("new",b);     DAterm(b,&t,p); checkterm(p,&t,2);
      makekform("year",b);    DAterm(b,&t,p); checkterm(p,&t,3);


      free(b);
      DAclose(p);
   }
   return 0;
}

