/* 3point6.c: Basic library for Muscat 3.6
 *
 * ----START-LICENCE----
 * Copyright 1999, 2000 Dialog Corporation
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

#include <stdio.h>     /* printf etc */
#include <stdlib.h>    /* malloc, exit etc */
#include "dbdefs.h"    /* for DB_ constants */
#include "io_system.h" /* for X_ functions */
#include "3point6.h"

extern int M_wordat(const byte * p)
{   return ((((signed char *)p)[0] << 8 | p[1]) << 8 | p[2]) << 8 | p[3];
}

/* the 'signed char *' cast above is for machines with > 32 bit wordsize */

extern void M_put_I(byte * p, int c, int n)
{   p[c+3] = n; n >>= 8;
    p[c+2] = n; n >>= 8;
    p[c+1] = n; n >>= 8;
    p[c] = n;
}

extern int M_compare_bytes(int n, const byte * p, int c, int m, const byte * q, int d)
{  int x = memcmp(p+c, q+d, (n < m) ? n : m);
   if (x) return x;
   return n - m;
}

extern int M_get_block_size(filehandle f, const char * s)
{   byte b[40];
    if (X_read(f, b, 40) == 40) switch (W(b, 1))
    {   case DARECS:           /* DA record file */
        case DATERMS:          /* DA term file */
            return W(b, 2);
        default:               /* DB file */
            if (W(b, DB_BASE) + W(b, DB_BASE2) == 1) return W(b, DB_BLOCK_SIZE);
    }
    printf("Can't read block size of %s\n", s); exit(1);
}


extern struct record * M_make_record()
{  struct record * r = (struct record *) malloc(sizeof(struct record));
   r->size = 0;
   r->p = 0;
   r->number = -1;
   return r;
}

extern void M_lose_record(struct record * r)
{  free(r->p);
   free(r);
}

extern void M_open_terms(struct termvec * tv)
{  byte * p = tv->p;
   int x = tv->heavy_duty;
   tv->nextterm = p+TVSTART(x);
   tv->l = p+TVSIZE(p, 0, x);
}

/* Each item in a termvec is a k-form string with a preceding flag
   byte. If the bottom bit (bit 0) of the flag byte is set, the term is
   marked for relevance feedback.

   If bit 2 is set, the string is followed by a four byte number giving
   term frequency. (bit 2 does not get set in DB files.)

   If bit 1 is set, the string is followed by a one byte count of
   within-document-frequency.

   If bit 3 is set, there finally follows a sequence of the form:
        L o1 w1 o2 w2 ...  oN wN
        <---       L        --->
   o1 o2 ... giving offsets of term positions into the source record, w1
   w2 ... the widths of the terms, L the length. L is 2 bytes; each oi is
   LWIDTH(x) bytes; each wi is 1 byte.
*/

extern void M_read_terms(struct termvec * tv)
{  byte * t = tv->nextterm;
   if (t >= tv->l) { tv->term = 0; return; }
   {  int flags = t[-1];
      tv->term = t;
      t = t+t[0]; /* t points after the k-form */
      tv->rel = flags & 1;
      tv->freq = -1;
      if (flags & 4) { tv->freq = I(t, 0); t += ILEN; }
      tv->wdf = 0;
      if (flags & 2) { tv->wdf = t[0]; t++; }
      tv->termp = 0;
      if (flags & 8) { tv -> termp = t; t += L2(t, 0); }
      tv->nextterm = t+1;
   }
}

extern struct termvec * M_make_termvec()
{  struct termvec * tv = (struct termvec *) malloc(sizeof(struct termvec));
   tv->size = 0;
   tv->p = 0;
   tv->number = -1;
   return tv;
}

extern void M_lose_termvec(struct termvec * tv)
{  free(tv->p);
   free(tv);
}

