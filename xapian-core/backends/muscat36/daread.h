/* daread.h: Header file for DA reading code
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef _daread_h_
#define _daread_h_

#include "3point6.h"

/* DA files
 * --------
 * Functions for handling DA record and DA term files:
 * 
 * struct DA_file *     DA_open(const char * s, int type, int heavy_duty)
 * void                 DA_close(struct DAfile * p)
 * 
 * To open DA record and/or DA term files for example:
 * 
 *         struct DA_file * DA_r;
 *         struct DA_file * DA_t;
 *         DA_r = DA_open("/home/richard/test/d/R", DA_RECS, FLIMSY);
 *         DA_t = DA_open("/home/richard/test/d/T", DA_TERMS, FLIMSY);
 * 
 * The 3rd argument is either FLIMSY or HEAVY_DUTY - values 0 and 1.
 * 
 * And to close:
 * 
 *         DA_close(DA_r); DA_close(DA_t);
 * 
 * The total number of documents (or terms) in the DA file is given
 * by
 * 
 *         DA_r->itemcount       (DA_t->itemcount)
 * 
 * DA term access:
 * ---------------
 * The procedures defined as extern are:
 * 
 * int  DA_term(byte * k, struct DA_term_info * t, struct DA_file * p)
 * struct DA_postings *
 *      DA_open_postings(struct DA_terminfo * t, struct DA_file * p)
 * void DA_read_postings(struct DA_postings * q, int style, int Z)
 * void DA_close_postings(struct DA_postings * q)
 * int  DA_get_record(struct DA_file * p, int n, struct record * r)
 * int  DA_get_termvec(struct DA_file * p, int n, struct termvec * tv)
 * 
 * To look up a term k in DA_t using a DA_term_info structure:
 * 
 *        struct DA_term_info * t;
 *        found = DA_term(k, &t, DA_t);   // k is a k-sting of course
 * 
 * 'found' is 1 if found, 0 if not found.
 * 
 * [If not found 'DAterminfo' is filled in with info about the 'nearest'
 * term, but we won't be concerned with this case.]
 * 
 * t->freq is the term frequency. Immediately following a DAterm call, t
 * can be used to open a posting list:
 * 
 *        string DA_postings * q;
 *        q = DA_open_postings(&t, DA_t);
 * 
 * (thereafter t can be reused to look for other terms), and read from with
 * 
 *        DA_read_postings(q, style, Z);
 * 
 * If style == 0 each call delivers a doc number in q->Doc and a wdf number in
 * q->wdf. Termination occurs when q->Doc == MAXINT, after which MAXINT is
 * repeatedly delivered. Z == 0 usually, but skipping forward can be done by
 * setting Z > 0. Then the next q->Doc to be delivered will be the first one >=
 * Z.
 * 
 * If style == 1, doc numbers are delivered back in ranges:
 * 
 *       q->Doc to q->E  (with a common q->wdf)
 * 
 * Z is correctly interpreted, so if the next range was 100 to 200 and the call
 * was DA_read_postings(q, 1, 137), q->Doc would be 137 and q->E 200.
 * 
 * styles 0 and 1 can't be mixed in successive calls.
 * 
 * Finally close with
 * 
 *     DA_close_postings(q)
 * 
 * (you can close before hitting MAXINT.)
 * 
 *     [This bracketed section is coded but not compiled up. Once a term has
 *      been found:
 * 
 *            struct DA_term_info * t;
 *            found = DA_term(k, d, &t, DA_t);
 * 
 *      The next/previous term can be put into t with the calls
 * 
 *            found = DA_next_term(&t, DA_t);
 *            found = DA_prev_term(&t, DA_t);
 * 
 *      found is 0 when we hit the end/beginning of the term list. t->term
 *      gives the term (as a k-string) and t->freq gives its frequency.]
 * 
 * DA record access:
 * -----------------
 * 
 * The procedures defines as extern are
 * 
 * int DA_get_record(struct DA_file * p, int n, struct record * r)
 * int DA_get_termvec(struct DA_file * p, int n, struct termvec * tv)
 * 
 * To get records 148, 241 in turn, do this:
 * 
 *     record * r = M_make_record();
 *     found = DA_get_record(DA_r, 148, r); // record is at r->p
 *     ...
 *     found = DA_get_record(DA_r, 241, r); // record is at r->p
 *     ...
 *     M_lose_record(r);
 * 
 * found is true/false according as found/not found. r keeps a single buffer for
 * the record read, so the second DA_get_record overwrites the first.
 * 
 * To get the records together:
 * 
 *     record * r1 = M_make_record();
 *     record * r2 = M_make_record();
 *     found = DA_get_record(DA_r, 148, r1); // record is at r1->p
 *     ...
 *     found = DA_get_record(DA_r, 241, r2); // record is at r2->p
 *     ...
 *     M_lose_record(r1); M_lose_record(r2);
 * 
 * Exactly the same principle applies to termvecs:
 * 
 *     termvec * tv = M_make_termvec();
 *     found = DA_get_termvec(DA_r, 148, tv);
 *     ...
 *     found = DA_get_termvec(DA_r, 241, tv);
 *     ...
 *     M_lose_termvec(tv);
 * 
 * or
 * 
 *     termvec * tv1 = M_make_termvec();
 *     termvec * tv2 = M_make_termvec();
 *     found = DA_get_termvec(DA_r, 148, tv1);
 *     ...
 *     found = DA_get_termvec(DA_r, 241, tv2);
 *     ...
 *     M_lose_termvec(tv1); M_lose_termvec(tv2);
 * 
 * A termvec can be read sequentially with
 * 
 *     M_open_terms(tv);
 *     M_read_terms(tv); M_read_terms(tv);
 */

struct DA_file
{
    filehandle locator;
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

extern int                  DA_next_term(struct DA_term_info * v, struct DA_file * p);
extern int                  DA_prev_term(struct DA_term_info * v, struct DA_file * p);

extern int                  DA_get_record(struct DA_file * p, int n, struct record * r);

extern int                  DA_get_termvec(struct DA_file * p, int n, struct termvec * tv);

#endif /* daread.h */
