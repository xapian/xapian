/* daread.h: Header files for martin's code */

#ifndef _daread_h_
#define _daread_h_

/* Make header file work when included from C++ */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>   /* main etc */
#include "damuscat.h"

struct DAfile
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
};

struct terminfo
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

struct postings
{
    /* after q = DAopenpostings(v,p), members of q are */

    int o;        /* offset down the current block */
    int blocknum; /* the number of the first block */
    int blockinc; /* the current block is the first block plus blockinc
                     (blockinc is -1 when posting pre-read) */
    struct DAfile * p;
    /* p, the DAfile struct */
    byte * b;     /* a buffer for the input */
    int D;        /* key, and */
    int E;        /* range end, for identity ranges */
    int Doc;      /* externally, the doc number delivered */
    int wdf;      /* within-doc-frequency */
};

struct record
{   byte * p;     /* the record */
    int size;     /* size of malloc-ed space */
    int number;   /* its number */
};


struct termvec
{   byte * p;     /* the termvec */
    int size;     /* size of malloc-ed space */
    int number;   /* its number */

    /* termvec inherits record, and is cast to (record) at one
       significant point */

    byte * l;     /* its end point */
    byte * term;  /* current term is tv->term */
    byte * nextterm;
                  /* next term is tv->nextterm */
    int rel;      /* marked for relevance feedback? */
    int wdf;      /* its wdf (or 0) */
    int freq;     /* its frequency (or -1) */
    byte * termp; /* pointer to position info (or 0) */
};

extern struct DAfile * DAopen(byte * s, int type);
extern void   DAclose(struct DAfile * p);
extern int    DAterm(byte * k, struct terminfo * t, struct DAfile * p);
extern struct postings * DAopenpostings(struct terminfo * t, struct DAfile * p);
extern void   DAreadpostings(struct postings * q, int style, int Z0);
extern void   DAclosepostings(struct postings * q);

extern int    DAnextterm(struct terminfo * v, struct DAfile * p);
extern int    DAprevterm(struct terminfo * v, struct DAfile * p);

extern struct record * makerecord();
extern void   loserecord(struct record * r);
extern int    DAgetrecord(struct DAfile * p, int n, struct record * r);

extern struct termvec * maketermvec();
extern void   losetermvec(struct termvec * tv);
extern int    DAgettermvec(struct DAfile * p, int n, struct termvec * tv);

extern struct terms * openterms(struct termvec * tv);
extern void   readterms(struct termvec * tv);

#ifdef __cplusplus
}
#endif

#endif /* daread.h */
