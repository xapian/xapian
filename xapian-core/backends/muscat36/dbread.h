/* dbread.h: Header file for DB reading code
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

#ifndef _dbread_h_
#define _dbread_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include "3point6.h"
#include "musmutex.h"
#include "io_system.h"
#include "dbdefs.h"

struct DB_pool /* block held in memory */
{
    byte * p;       /* block pointer */
    int n;          /* block number */
    long int clock; /* value of the DB->clock when last requested */
};

struct DB_cursor /* pointer into a block */
{
    struct DB_pool * pool; /* pointer to a DB_pool item */
    int c;                 /* block offset */
    int n;                 /* block number */
    int version;           /* block version number */
};

struct DB_file
{
    filehandle locator;

    int block_size;

    int levels;
    int block_count;
    int root;
    int block_offset;

    struct DB_cursor * cursor;

    int pool_size;
    struct DB_pool * pool;

    const byte * p;    /* address of block of `current' item */
    int c;             /* and it's offset */
    const byte * key;  /* address of the key in the current item */
    const byte * tag;  /* and the tag */
    int tag_size;      /* size off the tag in bytes */
    long int clock;    /* Advances 1,2,3 with block requests */

    int doc_count;
    int term_count;

    int heavy_duty;     /* 1 or 0 according as heavy duty or flimsy */
    MUS_PTHREAD_MUTEX(mutex);
};

struct DB_term_info
{

    /* When a term is looked up in a DB index, a terminfo structure
     *    is filled in with this information: */

    int freq;      /* term frequency */
    byte key[257]; /* 'A' + term + 0 key */
};

struct DB_postings
{
    /* after q = DB_open_postings(...), members of q are */

    int freq;
    struct DB_file * DB;
    struct DB_cursor * cursor;
    int buffer_size;
    byte * buffer;
    int i;
    int lim;
    byte * key;
    int Doc;
    int E;
    int wdf;
};

extern struct DB_file *     DB_open(const char * s, int n);
extern void                 DB_close(struct DB_file * p);
extern int                  DB_term(const byte * k, struct DB_term_info * t, struct DB_file * p);
extern struct DB_postings * DB_open_postings(struct DB_term_info * t, struct DB_file * p);
extern void                 DB_read_postings(struct DB_postings * q, int style, int Z);
extern void                 DB_close_postings(struct DB_postings * q);

extern int                  DB_get_record(struct DB_file * p, int n, struct record * r);
extern int                  DB_get_termvec(struct DB_file * p, int n, struct termvec * tv);

#ifdef __cplusplus
}
#endif

#endif /* dbread.h */
