/* daread.c: Code to read an old muscat Direct Access file
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "3point6.h"
#include "daread.h"

#define true        1
#define false       0

static int smaller(int a, int b) { return (a < b) ? a : b; }

#define BITSPERBYTE 8
#define M_5 (BITSPERBYTE-3)
#define M_5MASK ((1 << M_5) - 1)

static int unpackint(int * a, const byte * p, int o)
/* unpacks into *a the integer at p[o] */
{   int n = p[o];
    int s = n >> M_5;
    o++;
    if (s < 4) { *a = n; return o; }
    {   int m = n & M_5MASK;
        int i; for (i = 4; i <= s; i++) { m = m << BITSPERBYTE | p[o]; o++; }
        *a = m;
    }
    return o;
}

static int packint(int n, byte * p, int o)
{   p[o] = n;
    return o+1;
} /* so long as n < 128 */

    /*-----------------------
    static int mcount=0;
    void * malloc_(int n)
    {  void * p;
       { int i; for (i=0;i<mcount;i++) printf(" "); } mcount++;
       p = malloc(n);
       printf("get  %d (%d)\n", p, n);
       return p;
    }

    void free_(void * p)
    {  mcount--; { int i; for (i=0;i<mcount;i++) printf(" "); }
       free(p);
       printf("lose %d\n", p);
    }
    void print_kstring(byte * p, int c)
    {  printf("[");
       { int i; for (i = 1; i < p[c]; i++) printf("%c", p[c+i]); }
       printf("]\n");
    }                                  ----debugging stuff
    -----------------------*/


/* All the stuff above is just a reworking of bits of my BCPL */


static void readda(struct DA_file * q, int n, byte * b)
{
    filehandle q_locator = q->locator;
    int q_blocksize = q->blocksize;
    if (X_point(q_locator, q_blocksize, n) >= 0)
       if (X_read(q_locator, b, q_blocksize) == q_blocksize) return;
    fprintf(stderr, "Can't read block %d of DA_ file\n", n); exit(1);
}

extern struct DA_file * DA_open(const char * s, int type, int heavy_duty)
{
    struct DA_file * p;
    filehandle q;
    int bsize;
    byte * b;

    q = X_findtoread(s);
    if (q == -1) return NULL;
    bsize = M_get_block_size(q, s);
    b = malloc(bsize+40);  /* ample */
    p = (struct DA_file *) calloc(1, sizeof(struct DA_file));
    p->locator = q;
    p->blocksize = bsize;
    readda(p, 0, b);

    p->codeword = W(b, 1);
    p->type = W(b, 4);
    p->levels = W(b, 5);
    p->blockcount = W(b, 6);
    p->itemcount = W(b, 7);
    p->firsttermblock = W(b, 8);
    p->lasttermblock = W(b, 9);

    if (p->codeword != type)
    {
	fprintf(stderr, "You are not using a proper DA %s file\n",
               (type == DA_TERMS) ? "term" : "record");
        exit(1);
    }
    free(b);
    {   int bvecsize = p->levels;
        byte * * bvec = (byte * *) calloc(1, (bvecsize+1) * sizeof(byte *));
        int * buse = (int *) calloc(1, (bvecsize+1) * sizeof(int));

        {  int i; for (i = 0; i <= bvecsize; i++)
           {   bvec[i] = (byte *) calloc(1, p->blocksize);
               buse[i] = -1;
           }
        }
        p->buffers = bvec;
        p->buffuse = buse;
    }
    p->pblockno = -1;

    if (heavy_duty != 0 && heavy_duty != 1)
    {
	fprintf(stderr, "3rd arg of DA_open should be 0 or 1\n");
        exit(1);
    }
    p->heavy_duty = heavy_duty;
    MUS_PTHREAD_MUTEX_INIT(p->mutex);
    return p;
}

extern void DA_close(struct DA_file * p)
{
    X_close(p->locator);
    free(p->buffuse);
    {   int i;
        for (i = 0; i <= p->levels; i++) free(p->buffers[i]);
    }
    free(p->buffers);
    free(p->next);
    MUS_PTHREAD_MUTEX_DESTROY(p->mutex);
    free(p);
}


/* DA_term(k, v, p) looks up term k in DA_ term file p, putting the result in v.
   gives true/false if the term is found/not found.
   The last index term <= the search term (or the 1st term) has
   information placed in v as in "tihdr".
*/

static void putin(struct DA_term_info * v, byte * b, int i, int o, int blockno, struct DA_file * p)
{
    v->p = b; v->term = b+o; v->o = i; v->n = blockno;
    o += b[o];
    o = unpackint(& v->po, b, o);
    o = unpackint(& v->psize, b, o);
    if (v->psize == 0)
    {   o = unpackint(& v->shsize, b, o);
        o = unpackint(& v->shcount, b, o);
        o = unpackint(& v->psize, b, o);
    } else { v->shsize = 0; v->shcount = 0; }
    o = unpackint(& v->freq, b, o);
    o = unpackint(& v->pn, b, o);
    v->pn += I(b, p->blocksize-1-ILEN);
    /* Messiness of next 2 lines caused by expectations of QD2 */
    v->termno = 2;
    if (blockno == p->firsttermblock && i == 2) v->termno = 1;
}

static int fstring(const byte * k, int d, const byte * b, int o, int klen)
{   return M_compare_bytes(klen-1, k, d+1, b[o]-1, b, o+1);
}

extern int DA_term(const byte * k, struct DA_term_info * v, struct DA_file * p)
{
    int retval;
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    {
	int klen = k[0];
	byte * * bvec = p->buffers;
	int * buse = p->buffuse;
	int blockno = p->blockcount;  /* root block of index */
	byte * b;
	int i, j, o;
	int lev = 0;
	while(true)
	{   b = bvec[lev];
	    if (blockno != buse[lev])
	    {   readda(p, blockno, b);
		buse[lev] = blockno;
	    }
	    i = 2; j = L2(b, 0);   /* i indexes an index entry */
	    while (j-i > 2)
	    {   int h = (i+j)/4*2;
		int o = L2(b, h);
		if (fstring(k, 0, b, o, klen) < 0) j = h; else i = h;
	    }
	    o = L2(b, i);
	    if (lev == p->levels) break;
	    unpackint(& blockno, b, o+b[o]);
	    blockno += I(b, p->blocksize-ILEN-1);
	    lev++;
	}
	putin(v, b, i, o, blockno, p);
	retval = (fstring(k, 0, b, o, klen) == 0);
    }
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return retval;
}

static void next_posting(struct DA_postings * q, int Z)
{   byte * b = q->b;
    int o = q->o;
    while(true)
    {   o = unpackint(& q->wdf, b, o);
        if (q->wdf < 8) switch (q->wdf)
        {   case 0: /* terminator */
               q->Doc = MAXINT; q->F = MAXINT; return;
            case 2: /* padder */
               {   int inc = q->blockinc;
                   if (inc >= 0)
                   {   inc++;
                       if (Z == MAXINT)
                       {   q->Doc = MAXINT; q->F = MAXINT; return;
                       }
                       /* try the shortcut: */
                       {   int * sh = q->shortcut;
                           if (sh != 0)
                           {   int j = -1;
                               int i; for (i = 1; i <= sh[0]; i++)
                               {   if (sh[i] >= Z) break;
                                   j = i;
                               }
                               if (inc < j+1) { inc = j+1; q->E = sh[j]; }
                           }
                       }
                       readda(q->p, q->blocknum+inc, b);
                       q->blockinc = inc; o = 0;
                   }
               }
               continue;
           case 3: /* startoff */
               readda(q->p, q->blocknum, b);
               o = q->o; continue;
        }
        o = unpackint(& q->Doc, b, o); q->Doc += q->E;
        if ((q->wdf & 0X1) != 0)
        {
           o = unpackint(& q->F, b, o);
           q->F += q->Doc;
        }
        else
           q->F = q->Doc;

        q->wdf = (q->wdf >> 1) - 4;
        q->o = o; return;
    }
}

static byte * copybytes(int k, struct DA_file * p, int n, int o)
/* copy k bytes from block n offset o in DA_ file p */
{   int l = p->blocksize;
    byte * b = (byte *) malloc(k);
    int i = 0;
    byte * r = p->next;
    if (r == 0) { r = (byte *) malloc(l); p->next = r; p->pblockno = -1; }
    while (k != 0)
    {   int x = smaller(k, l-o);
        if (p->pblockno != n) { readda(p, n, r); p->pblockno = n; }
        MOVEBYTES(x, r, o, b, i); k = k-x; i = i+x; o = 0; n++;
    }
    return b;
}


static int * read_shortcut(struct DA_file * p, int n, int o, int shsize, int shcount)
{   n = n+o/p->blocksize; o = o % p->blocksize;
    {   byte * b = copybytes(shsize, p, n, o);
        int * v = (int *) malloc((shcount+1) * sizeof(int));
        int c = unpackint(v+1, b, 0);
        int i; for (i = 1; i < shcount; i++)
        {  c = unpackint(v+i+1, b, c); v[i+1] += v[i];  }
        v[0] = shcount;
        free(b);
        return v;
    }
}

extern struct DA_postings *
DA_open_postings(struct DA_term_info * v, struct DA_file * p)
{
    struct DA_postings * q =
	    (struct DA_postings *) calloc(1, sizeof(struct DA_postings));
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    {
	q->p = p; q->D = 1; q->E = 0; q->wdf = 0; q->shortcut = 0;

	if (v->freq == 0) {
	    byte * b = (byte *) calloc(1, sizeof(byte));
	    q->b = b; q->o = 0;
	    b[0] = 0;  /* terminator */
	} else {
	    int l = p->blocksize;
	    int size = v->psize;
	    int blocknum = v->pn;
	    if (p->next == 0) { p->next = (byte *) calloc(1, l); p->pblockno = -1; }
	    if (l > size)
	    {   q->b = copybytes(size, p, blocknum, v->po);
		q->blockinc = -1; q->o = 0;
	    }
	    else
	    {   q->b = p->next;
		p->next = 0;
		q->blockinc = 0;
		q->blocknum = blocknum; q->o = v->po;

		if (v->shsize > 0)
		{   q->shortcut = read_shortcut(p, blocknum, q->o+size,
						v->shsize, v->shcount);
		packint(3, q->b, q->o); /* startoff */
		}
		else
		{   if (p->pblockno != blocknum)
		    {   readda(p, blocknum, q->b);
			p->pblockno = blocknum;
		    }
		}
	    }
	}
    }
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return q;
}

extern void DA_read_postings(struct DA_postings * q, int style, int Z)
{
    MUS_PTHREAD_MUTEX_LOCK(q->p->mutex);
    if (style > 0) {
        do {
	    next_posting(q, Z); q->E = q->F;
	} while (q->F < Z);
    } else {
	/* interpret ranges if style == 0 */
	q->Doc = q->D; q->F = q->E;
	while (q->F < Z || q->F < q->Doc) {
	    next_posting(q, Z);
	    q->E = q->F;
	}

	if (q->Doc < Z) q->Doc = Z;
	q->D = q->Doc+1;
    }
    MUS_PTHREAD_MUTEX_UNLOCK(q->p->mutex);
    return;
}

extern void DA_close_postings(struct DA_postings * q)
{
    free(q->b);
    free(q->shortcut);
    free(q);
}

#if 0
/* The following works, but is not currently needed */

extern int DA_next_term(struct DA_term_info * v, struct DA_file * p)
{
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    {
	byte * b = v->p;
	int i = v->o + 2;
	int blockno = v->n;
	if (i == L2(b, 0)) {
	    do {
		blockno++;
		if (blockno > p->lasttermblock) {
		    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
		    return false;
		}
		readda(p, blockno, b);
	    } while (b[p->blocksize-1] != 0);
	    i = 2;
	    (p->buffuse)[p->levels] = blockno;
	}
	putin(v, b, i, L2(b, i), blockno, p);
    }
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return true;
}

extern int DA_prev_term(struct DA_term_info * v, struct DA_file * p)
{
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    {
	byte * b = v->p;
	int i = v->o - 2;
	int blockno = v->n;
	if (i == 0) {
	    do {
		blockno--;
		if (blockno < p->firsttermblock) {
		    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
		    return false;
		}
		readda(p, blockno, b);
	    } while (b[p->blocksize-1] != 0);
	    i = L2(b, 0)-2;
	    (p->buffuse)[p->levels] = blockno;
	}
	putin(v, b, i, L2(b, i), blockno, p);
    }
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return true;
}
#endif

static void DA_read_bytes(struct DA_file * p, int l, struct record * r, int notskipping)
{   int lev = p->levels;
    byte * b = p->buffers[lev];
    int blockno = p->buffuse[lev];
    int bsize = p->blocksize-1;
    int o = p->o;
    int d = 0;
    if (notskipping && (r->p == 0 || l > r->size))
    {   free(r->p);
        r->p = (byte *) malloc(l+100);
        r->size = l+100;
    }
    while (l > bsize-o)
    {   if (notskipping) MOVEBYTES(bsize-o, b, o, r->p, d);
        d += bsize-o; l -= bsize-o;
        do
        {   blockno++;
            readda(p, blockno, b);
        } while (b[p->blocksize-1] != BYTERANGE-1);
        o = 2;
    }
    if (notskipping) MOVEBYTES(l, b, o, r->p, d);
    p->buffuse[lev] = blockno; p->o = o+l;
}

static void DA_next_unit(struct DA_file * p, int m, int n, struct record * r)
{

    int x = p->heavy_duty;

#define RECHEADSIZE  (LWIDTH(x)+2*ILEN)
#define ROFFSET      (LWIDTH(x)+ILEN)
#define SHIFTUP      (LWIDTH(x)*BITSPERBYTE)

    byte * b;
    int l, number;
    r->heavy_duty = x;
    r->number = -1; /* error condition */
    while(true)
    {   DA_read_bytes(p, RECHEADSIZE, r, true); b = r->p;
        l = LENGTH_OF(b, 0, x) | b[ROFFSET] << SHIFTUP; /* old bug 9 */
        if (l == 0) return;
        l -= RECHEADSIZE;
        number = I(b, LWIDTH(x));  /* the record number */
        if (n < number) return;
        if (m <= number) { DA_read_bytes(p, l, r, true); break; }
        DA_read_bytes(p, l, r, false);
    }
    r->number = number/2;
}

/* DA_read_unit(p, key, range, r) reads one unit from the DA unit file
   p into r. The unit is specified by 'key, range', and the
   unit read will in fact be the first unit which has some overlap
   with this range. If there is no such unit r->number is set to -1,
   otherwise the number of the unit read.
   Following a DA_read_unit(...) sequential reading can be done with
   DA_next_unit(...).
*/

static void DA_read_unit(struct DA_file * p, int m, int n, struct record * r)
{

#define KBLEN (2*ILEN)
#define KBLEN2 (2*KBLEN)

    byte * * bvec = p->buffers;
    int * buse = p->buffuse;
    int blockno = p->blockcount;  /* root block of index */
    byte * b;
    int i, j;
    int lev = 0;
    if (p->itemcount == 0) return; /* empty DA file */
    while(true)
    {   b = bvec[lev];
        if (blockno != buse[lev]) { readda(p, blockno, b); buse[lev] = blockno; }
        if (lev == p->levels) break;
        i = 0; j = L2(b, 0)-2;
        while (j-i > KBLEN)
        {   int h = (i+j)/KBLEN2 * KBLEN;
            if (m < I(b, h+2)) j = h; else i = h;
        }
        i += 2+ILEN;
        blockno = I(b, i);
        lev++;
    }
    p->o = L2(b, 0);
    if (p->o == 0) {
	fprintf(stderr, "STRUCTURE ERROR\n");
	exit(1);
    }
    DA_next_unit(p, m, n, r);
}

extern int DA_get_record(struct DA_file * p, int n, struct record * r)
{
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    DA_read_unit(p,
		 2 * n,
		 2 * n,
		 r);
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return (r->number == n);
}

extern int DA_get_termvec(struct DA_file * p, int n, struct termvec * tv)
{
    MUS_PTHREAD_MUTEX_LOCK(p->mutex);
    DA_read_unit(p,
		 2 * n + 1,
		 2 * n + 1,
		 (struct record *) tv);
    MUS_PTHREAD_MUTEX_UNLOCK(p->mutex);
    return (tv->number == n);
}

