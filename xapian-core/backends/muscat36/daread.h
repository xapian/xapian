/* daread.h: Header file for DA reading code
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

#ifndef _daread_h_
#define _daread_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "3point6.h"

struct DA_file
{   filehandle locator;
                      /* DA file locator */
    int o;            /* is used internally in the "record" case */
                      /* vector giving D -> b map for recent Ds I've removed */
    byte * * buffers; /* address of vector of buffers */
    int * buffuse;    /* address of vector giving actual buffer use */
    int pblockno;     /* the number of the block of postings pointed
                         to by p->next, when p->next ne 0 */

    int codeword;
    int blocksize;
    int type;
    int levels;
    int blockcount;
    int itemcount;
    int firsttermblock;
    int lasttermblock;

    byte * next;        /* 0, or block of postings */
    int heavy_duty;     /* 1 or 0 according as heavy duty or flimsy */
};

struct DA_term_info
{

    /* When a term is looked up in a DA index, a terminfo structure
     *    is filled in with this information: */

    int freq;     /* term frequency */
    int pn;       /* posting-block number */
    int po;       /* posting-block offset */
    int psize;    /* postings size */
    int shsize;   /* size of shortcut vector */
    int shcount;  /* number of items in shortcut vector */

    byte *p;      /* term-block pointer (transitory) */
    byte *term;   /* term pointer (transitory) */
    /*int c;        -- term-block offset */
    int o;        /* term-block index offset */
    int n;        /* term-block number */
    int termno;   /* 1 for first term */
};

struct DA_postings
{
    /* after q = DAopenpostings(t, p), members of q are */

    int o;        /* offset down the current block */
    int blocknum; /* the number of the first block */
    int blockinc; /* the current block is the first block plus blockinc
                     (blockinc is -1 when posting pre-read) */
    struct DA_file * p;
    /* p, the DAfile struct */
    byte * b;     /* a buffer for the input */
    int D;        /* key, and */
    int E;        /* range end, for identity ranges */
    int Doc;      /* externally, the doc number delivered */
    int F;        /* picked up after D in the compacted ranges */
                  /* (It should be possible to eliminate F - daread.c code
                     needs reworking to be like dbread.c) */
    int wdf;      /* within-doc-frequency */
    int * shortcut;
                  /* the 'short cut' vector for skipping blocks
                     that don't need to be read */
};

extern struct DA_file *     DA_open(const char * s, int type, int heavy_duty);
extern void                 DA_close(struct DA_file * p);
extern int                  DA_term(const byte * k, struct DA_term_info * t, struct DA_file * p);
extern struct DA_postings * DA_open_postings(struct DA_term_info * t, struct DA_file * p);
extern void                 DA_read_postings(struct DA_postings * q, int style, int Z);
extern void                 DA_close_postings(struct DA_postings * q);

/* -- don't compile the next two until needed
extern int                  DA_next_term(struct DA_term_info * v, struct DA_file * p);
extern int                  DA_prev_term(struct DA_term_info * v, struct DA_file * p);
*/

extern int                  DA_get_record(struct DA_file * p, int n, struct record * r);

extern int                  DA_get_termvec(struct DA_file * p, int n, struct termvec * tv);

#ifdef __cplusplus
}
#endif

#endif /* daread.h */
