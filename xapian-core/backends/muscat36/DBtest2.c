/* DBtest2.c: Test harness for old muscat compatibility code
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
#include <fcntl.h>   /* O_RDONLY etc */
#include <stdlib.h>  /* exit etc */
#include "3point6.h"
#include "dbread.h"

int X_findtowrite(char * s)
{  filehandle h = open(s, O_WRONLY | O_CREAT | O_TRUNC, 0666);
   if (h < 0) { printf("Can't open %s\n",s); exit(1); }
   return h;
}

int X_write(filehandle h, byte * b, int n)
{  if (write(h, b, n) != n) { printf("write failure\n"); exit(1); }
}

int main(int argc, byte * argv[])
{  int x = HEAVY_DUTY;
   if (argc == 1) { printf("No argument\n"); exit(1); }
   {
      struct DBfile * p;
      struct record * r;
      filehandle h;

      p = DBopen(argv[1],30,x);
      if (p == NULL) { printf("Can't open %s\n",argv[1]); exit(1); }
      r = M_make_record();
      h = X_findtowrite("TEMP");
      {  int i; for (i=1; i<MAXINT; i++)
         {  if (i % 1000 == 1) printf("CYCLE %d\n",i);
            if (! DBgetrecord(p, i, r)) break;
            X_write(h, r->p, LENGTH_OF(r->p, 0, x));
         }
      }
      close(h);
      M_lose_record(r);
      DBclose(p);
   }
   return 0;
}

