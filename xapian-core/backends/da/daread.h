/* daread.h: Header files for martin's code */

#ifndef _daread_h_
#define _daread_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>   /* main etc */
#include "muscat.h"

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
    int c;        /* term-block offset */
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

struct DAfile * DAopen(byte * s, int type);
void   DAclose(struct DAfile * p);
int    DAterm(byte * k, int d, struct terminfo * t, struct DAfile * p);
struct postings * DAopenpostings(struct terminfo * t, struct DAfile * p);
void   DAreadpostings(struct postings * q, int style, int Z0);
void   DAclosepostings(struct postings * q);

#ifdef __cplusplus
}
#endif

#endif /* daread.h */
