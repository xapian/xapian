/* DBtest1.c: Test harness for old muscat compatibility code
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

#include <stdio.h>   /* main etc */
#include <stdlib.h>  /* exit etc */
#include <string.h>  /* memmove, memcmp */
#include "3point6.h"
#include "dbread.h"

void printkform(byte * k)
{  printf("[");
   {  int i; for (i=1; i < k[0]; i++) printf("%c",k[i]);  }
   printf("]"); fflush(stdout);
}

void checkterm(struct DBfile * DB, struct DBterminfo * t, int style)
{     printf("[%s]", t->key + 1);
      printf("  term frequency %d\n", t->freq);

#define ZED 5124

      {  struct DBpostings * q = DBopenpostings(t, DB);
         int i; for(i = 1;; i++)
         {  DBreadpostings(q, style, ZED);
            /* style 0 for expanded, 1 for compact, ranges */
            if (q->Doc == MAXINT) break;
            if (q->Doc == q->E) printf("%d (%d) ", q->Doc, q->wdf);
                           else printf("%d-%d (%d) ", q->Doc, q->E, q->wdf);
            if (i % 5 == 0) printf("\n");
         fflush(stdout);
         }
         printf("\n");
         DBclosepostings(q);
      }

}

void findterm(struct DBfile * DB, struct DBterminfo * t, byte * k, int style)
{  int found = DBterm(k, t, DB);
   printkform(k);
   printf(" %d\n", found); fflush(stdout);
   if (!found) return;
   checkterm(DB, t, style);
}

void makekform(char * s, byte * b)
{  int len = strlen(s);
   memmove(b+1, s, len); b[0] = len+1;
}

int main(int argc, char * argv[])
{  int x = HEAVY_DUTY;
   if (argc == 1) { printf("No argument\n"); exit(1); }
   printf("[%s]\n", argv[1]);
   {  struct DBfile * DB;
      struct DBterminfo t;
      byte * b = (byte *) malloc(1000);
      DB = DBopen(argv[1], 20, x);
      if (DB == NULL) { printf("Can't open %s\n", argv[1]); exit(1); }
      {   int style;
          for (style = 0; style <= 1; style++)
          {   makekform("p11", b); findterm(DB, &t, b, style);
              makekform("p3", b); findterm(DB, &t, b, style);
              makekform("p2", b); findterm(DB, &t, b, style);

              makekform("s12", b); findterm(DB, &t, b, style);
          }
      }
      free(b);
      DBclose(DB);
   }
   return 0;
}

