/* DBtest6.c: Test harness for old muscat compatibility code
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

/* dump(r, 0, x) prints r to stdout in approximate u-dump style */

static void dump(byte * p, int c, int x, int margin)
{
    int limit = c + LENGTH_OF(p, c, x);

    { int i; for (i = 0; i < margin; i++) printf(" "); }
    printf("*%d", CODE_OF(p, c, x));
    switch(TYPE_OF(p, c, x))
    {   case GROUP_FIELD:
            printf("\n");
            {   c += HEAD_LENGTH(x);
                margin++;
                while (c < limit)
                {   dump(p, c, x, margin);
                    c += LENGTH_OF(p, c, x);
                }
            }
            return;
        case STRING_FIELD:
            {   c += HEAD_LENGTH(x);
                printf(" ");
                while (c < limit) printf("%c",p[c++]);
            }
            break;
        case INTEGER_FIELD:
            printf(" %d", INTEGER_AT(p, c, x));
            while(1)
            {   c += 4; /* integers are always 4 bytes */
                if (c + HEAD_LENGTH(x) >= limit) break;
                printf(" %d", INTEGER_AT(p, c, x));  /* extra integers are rare */
            }
            break;
        case BINARY_FIELD:
            printf(" binary");
            break;
    }
    printf("\n");
}



int main(int argc, byte * argv[])
{  int x = FLIMSY;
   if (argc == 1) { printf("No argument\n"); exit(1); }
   {
      struct DB_file * p;
      struct record * r;

      p = DB_open(argv[1], 30, x);
      if (p == NULL) { printf("Can't open %s\n",argv[1]); exit(1); }
      r = M_make_record();
      {  int i; for (i=1; i<MAXINT; i++)
         {  if (i % 1000 == 1) printf("CYCLE %d\n",i);
            if (! DB_get_record(p, i, r)) break;
            dump(r->p, 0, r->heavy_duty, 0);
         }
      }
      M_lose_record(r);
      DB_close(p);
   }
   return 0;
}

