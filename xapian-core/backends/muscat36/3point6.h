/* 3point6.h: header containing various definitions for old muscat 3.6 code.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003 Olly Betts
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

#ifndef _3point6_h_
#define _3point6_h_

#include "io_system.h"

#define BYTERANGE   256
#define MAXINT      0X7FFFFFFF

/* Record unpicking
 * ----------------
 * 
 * This is best explained through an example. Suppose a record has been read
 * into
 * 
 *     struct record r;
 * 
 * Then r->p points to the record, and r->heavy_duty is 0 or 1 according as r is
 * in flimsy or heavy duty style.
 * 
 * Calling dump(r->p, 0, r->heavy_duty, 0) prints the record out in an
 * approximate u-dump style (u-dump is a Muscat3.6 command).
 * 
 * Here is the definition of dump:
 * 
 *     static void dump(byte * p, int c, int x, int margin)
 *     {
 *         // c is a cursor that moves down the record,
 *         // x is 0 or 1 according as flimsy or heavy duty,
 *         // margin measures the width of the left-margin.
 *         int limit = c + LENGTH_OF(p, c, x);
 * 
 *         // print the margin
 *         {   int i;
 *             for (i = 0; i < margin; i++) printf(" ");
 *         }
 * 
 *         printf("*%d", CODE_OF(p, c, x));  // print the code
 * 
 *         switch(TYPE_OF(p, c, x))  // switch on the field type
 *         {   case GROUP_FIELD:
 *                 printf("\n");
 *                 {   c += HEAD_LENGTH(x);
 *                     margin++;
 *                     while (c < limit)
 *                     {   dump(p, c, x, margin);
 *                         c += LENGTH_OF(p, c, x);
 *                     }
 *                 }
 *                 return;
 *             case STRING_FIELD:
 *                 {   c += HEAD_LENGTH(x);
 *                     printf(" ");
 *                     while (c < limit) printf("%c",p[c++]);
 *                 }
 *                 break;
 *             case INTEGER_FIELD:
 *                 printf(" %d", INTEGER_AT(p, c, x));
 *                 while(1)
 *                 {   c += 4; // integers are always 4 bytes
 *                     if (c + HEAD_LENGTH(x) >= limit) break;
 *                     printf(" %d", INTEGER_AT(p, c, x));
 *                         // extra integers are rare/
 *                 }
 *                 break;
 *             case BINARY_FIELD:
 *                 printf(" binary");
 *                 break;
 *         }
 *         printf("\n");
 *     }
 */

#define FLIMSY 0
#define HEAVY_DUTY 1

/* In next few defs x should be either FLIMSY or HEAVY_DUTY */

#define LENGTH_OF(p,c,x)  (((x) ? (p)[(c)+2] << 16 : 0) | (p)[(c)+1] << 8 | (p)[(c)])
#define TYPE_OF(p,c,x)    ((p)[(c)+(x)+2])
#define FLAGS_OF(p,c,x)   ((p)[(c)+(x)+3])
#define CODE_OF(p,c,x)    ((p)[(c)+(x)+4])
#define LEVEL_OF(p,x)     ((p)[(x)+3] >> 4)

#define HEAD_LENGTH(x) (5+(x))

#define INTEGER_AT(p,c,x) M_wordat((p) + (c) + HEAD_LENGTH(x))

#define GROUP_FIELD 1
#define STRING_FIELD 2
#define INTEGER_FIELD 3
#define BINARY_FIELD 4

#define LWIDTH(x)   (2+(x))  /* bytes in a Muscat length */

#define DA_TERMS   10101     /* word used to identify DA index files */
#define DA_RECS    23232     /* word used to identify DA record files */
#define TVSTART(x) (LWIDTH(x)+1)
#define TVSIZE(p,c,x) (LENGTH_OF(p,c,x)+1)
#define ILEN 4
#define L2(p,c) ((p)[(c)+1] << 8 | (p)[(c)])

#define W(p, c) M_wordat((p) + 4*(c))
#define I(p, c) M_wordat((p) + (c))

#define MOVEBYTES(n, p, c, q, d) memmove((q) + (d), (p) + (c), (n))

struct record
{
    int heavy_duty;     /* 1 or 0 according as heavy duty or flimsy */

    byte * p;           /* the record */
    int size;           /* size of malloc-ed space */
    int number;         /* its number */
};

struct termvec
{
    int heavy_duty;     /* 1 or 0 according as heavy duty or flimsy */

    byte * p;           /* the termvec */
    int size;           /* size of malloc-ed space */
    int number;         /* its number */

    /* termvec inherits record, and is cast to (record) at one
       significant point */

    byte * l;           /* its end point */
    byte * term;        /* current term is tv->term */
    byte * nextterm;    /* next term is tv->nextterm */
    int rel;            /* marked for relevance feedback? */
    int wdf;            /* its wdf (or 0) */
    int freq;           /* its frequency (or -1) */
    byte * termp;       /* pointer to position info (or 0) */
};

extern int M_wordat(const byte * p);
extern void M_put_I(byte * p, int c, int n);
extern int M_compare_bytes(int n, const byte * p, int c, int m, const byte * q, int d);
extern int M_get_block_size(filehandle f, const char * s);

/* Functions for handling Muscat 3.6 records and term vectors:
 * 
 *     struct record * r = M_make_record();    // creates a record structure
 *     M_lose_record(r);                       // - and loses it
 * 
 *     struct termvec * tv = M_make_termvec(); // makes a termvec structure
 *     M_lose_termvec(struct termvec * tv);    // - and loses it
 * 
 * A Muscat 3.6 term is expected to be a 'k-string', with length L in k[0], and
 * characters in k[1] ... k[L-1]. This is how they come in the packed term
 * vectors. Obviously it's easy to cast a C string into this form:
 * 
 *        {  int len = strlen(s);
 *           memmove(k+1, s, len); k[0] = len+1;  // k-string in [k, 0]
 *        }
 * 
 * A termvec can be set up for term-by-term reading with
 * 
 *     M_open_terms(tv);
 * 
 * after which
 * 
 *     M_read_terms(tv);
 * 
 * reads successive terms.
 * 
 * Each call of M_read_terms(tv) gives
 * 
 *     tv->term  - the term (as k-string), or 0 when the list runs out
 *     tv->rel   - true/false according as term is/is not marked for
 *                 relevance feedback
 *     tv->freq  - the term frequency, or -1 if this info is absent
 *     tv->wdf   - the wdf, or 0 id this info is absent
 *     tv->termp - the term's positional information, or 0 if absent.
 *                 This can be unpicked by,
 * 
 *     int x = tv->heavy_duty; // x is 0 or 1
 *     if (tv->termp)
 *     {  byte * p = tv->termp;
 *        int l = L2(p, 0);
 *        int i;
 *        for (i=2; i < l; i += LWIDTH(x) + 1)
 *        printf(" offset=%d; width=%d", LENGTH_OF(p, i, x), p[i + LWIDTH(x)]);
 *     }
 * 
 *     L2, LWIDTH and LENGTH_OF are defined above.
 * 
 *     The term occurs at the given offsets in the record, with the given
 *     widths.
 */

extern struct record * M_make_record();
extern void M_lose_record(struct record * r);
extern void M_open_terms(struct termvec * tv);
extern void M_read_terms(struct termvec * tv);
extern struct termvec * M_make_termvec();
extern void M_lose_termvec(struct termvec * tv);

#endif /* 3point6.h */
