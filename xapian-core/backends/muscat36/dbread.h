/* dbread.h: Header file for DB reading code
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundbtion; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundbtion, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
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
#include "io_system.h"
#include "dbdefs.h"

struct DBpool /* block held in memory */
{
    byte * p;       /* block pointer */
    int n;          /* block number */
    long int clock; /* value of the DB->clock when last requested */
};

struct DBcursor /* pointer into a block */
{
    struct DBpool * pool; /* pointer to a DBpool item */
    int c;                /* block offset */
    int n;                /* block number */
    int version;          /* block version number */
};

struct DBfile
{   filehandle locator;

    int block_size;

    int levels;
    int block_count;
    int root;
    int block_offset;

    struct DBcursor * cursor;

    int pool_size;
    struct DBpool * pool;

    const byte * p;    /* address of block of `current' item */
    int c;             /* and it's offset */
    const byte * key;  /* address of the key in the current item */
    const byte * tag;  /* and the tag */
    int tag_size;      /* size off the tag in bytes */
    long int clock;    /* Advances 1,2,3 with block requests */

    int doc_count;
    int term_count;

    int heavy_duty;     /* 1 or 0 according as heavy duty or flimsy */
};

struct DBterminfo
{

    /* When a term is looked up in a DB index, a terminfo structure
     *    is filled in with this information: */

    int freq;      /* term frequency */
    byte key[257]; /* 'A' + term + 0 key */
};

struct DBpostings
{
    /* after q = DBopenpostings(...), members of q are */

    int freq;
    struct DBfile * DB;
    struct DBcursor * cursor;
    int buffer_size;
    byte * buffer;
    int i;
    int lim;
    byte * key;
    int Doc;
    int E;
    int wdf;
};

extern struct DBfile *     DBopen(const char * s, int n, int heavy_duty);
extern void                DBclose(struct DBfile * p);
extern int                 DBterm(const byte * k, struct DBterminfo * t, struct DBfile * p);
extern struct DBpostings * DBopenpostings(struct DBterminfo * t, struct DBfile * p);
extern void                DBreadpostings(struct DBpostings * q, int style, int Z0);
extern void                DBclosepostings(struct DBpostings * q);

extern int                 DBgetrecord(struct DBfile * p, int n, struct record * r);
extern int                 DBgettermvec(struct DBfile * p, int n, struct termvec * tv);

#ifdef __cplusplus
}
#endif

#endif /* dbread.h */
