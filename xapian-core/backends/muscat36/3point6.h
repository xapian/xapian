/* 3point6.h: header containing various definitions for old muscat 3.6 code.
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

#ifndef _3point6_h_
#define _3point6_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "io_system.h"

#define BYTERANGE   256
#define MAXINT      0X7FFFFFFF

#define FLIMSY 0
#define HEAVY_DUTY 1

/* In next few defs x should be either FLIMSY or HEAVY_DUTY */

#define LENGTH_OF(p,c,x)  ((p)[(c)+2] * (x) << 16 | (p)[(c)+1] << 8 | (p)[(c)])
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
extern struct record * M_make_record();
extern void M_lose_record(struct record * r);
extern void M_open_terms(struct termvec * tv);
extern void M_read_terms(struct termvec * tv);
extern struct termvec * M_make_termvec();
extern void M_lose_termvec(struct termvec * tv);

#ifdef __cplusplus
}
#endif

#endif /* 3point6.h */
