
#include <stdio.h>   /* main etc */
#include <stdlib.h>  /* exit etc */
#include <fcntl.h>   /* O_RDONLY etc */
#include <string.h>  /* memmove, memcmp */
#include "daread.h"

int wordat(byte * p) { return ((p[0]<<8|p[1])<<8|p[2])<<8|p[3]; }

#define W(p,c) wordat((p)+4*(c))
#define I(p,c) wordat((p)+(c))

#define mb(n,p,c,q,d) memmove((q)+(d),(p)+(c),(n))

int cb(int n, byte * p, int c, int m, byte * q, int d)
{  int x = memcmp(p+c,q+d,(n < m) ? n : m);
   if (x) return x;
   return n-m;
}

int smaller(int a, int b) { return (a < b) ? a : b; }
int larger(int a, int b) { return (a > b) ? a : b; }

#define BITSPERBYTE 8
#define M_5 (BITSPERBYTE-3)
#define M_5MASK ((1 << M_5) - 1)

int unpackint(int * a, byte * p, int o)
/* unpacks into *a the integer at p[o] */
{  int n = p[o];
   int s = n >> M_5;
   o++;
   if (s < 4) { *a = n; return o; }
   {  int m = n & M_5MASK;
      int i; for (i = 4; i <= s; i++) { m = m << BITSPERBYTE | p[o]; o++; }
      *a = m;
    }; return o;
}

int packint(int n, byte * p, int o)
{  p[o] = n; return o+1;  } /* so long as n < 128 */

    /*-----------------------
    static int mcount=0;
    void * malloc_(int n)
    {  void * p;
       { int i; for (i=0;i<mcount;i++) printf(" "); } mcount++;
       p = malloc(n);
       printf("get  %d (%d)\n",p,n);
       return p;
    }

    void free_(void * p)
    {  mcount--; { int i; for (i=0;i<mcount;i++) printf(" "); }
       free(p);
       printf("lose %d\n",p);
    }
    void print_kstring(byte * p, int c)
    {  printf("[");
       { int i; for (i = 1; i < p[c]; i++) printf("%c",p[c+i]); }
       printf("]\n");
    }                                  ----debugging stuff
    -----------------------*/

int X_findtoread(char * s)
{  filehandle h = open(s, O_RDONLY, 0666);
   if (h < 0) { printf("Can't open %s\n",s); exit(1); }
   return h;
}

int X_point(filehandle h, int n, int m)
{ return lseek(h, (long)n*m, 0); } /* <0 is error */

int X_close(filehandle h)
{ return close(h); } /* non-zero is error */

int X_read(filehandle h, byte * b, int n)
{ return read(h, b, n); }

/* All the stuff above is just a reworking of bits of my BCPL */


void readda(struct DAfile * q, int n, byte * b)
{  filehandle q_locator = q->locator;
   int q_blocksize = q->blocksize;
/* printf("READING %d\n",n); */ /* for debugging */
   if (X_point(q_locator, q_blocksize, n) >= 0)
      if (X_read(q_locator, b, q_blocksize) eq q_blocksize) return;
   printf("Can't read block %d of DA file\n", n); exit(1);
}

int block_size(filehandle f, byte * s)
{ byte b[40];
  if (X_read(f,b,40) eq 40) switch (W(b,1))
  { case DARECS: case DATERMS: return W(b,2);
    default: if (W(b,1)+W(b,2) eq 1) return W(b,7); /* DB file */
  }
  printf("Can't read block size of %s\n",s); exit(1);
}

extern struct DAfile * DAopen(byte * s, int type)
{  struct DAfile * p = (struct DAfile *) malloc(sizeof(struct DAfile));
   filehandle q = X_findtoread(s);
   int bsize = block_size(q,s);
   byte * b = malloc(bsize+40);  /* ample */
   byte * s_type = (type eq DATERMS) ? "term" : "record";
   p->locator = q;
   p->blocksize = bsize;
   readda(p,0,b);

   p->codeword = W(b,1);
   p->type = W(b,4);
   p->levels = W(b,5);
   p->blockcount = W(b,6);
   p->itemcount = W(b,7);
   p->firsttermblock = W(b,8);
   p->lasttermblock = W(b,9);

   unless (p->codeword eq type)
   {  printf("You are not using a proper DA %s file\n",s_type);
      exit(1);
   }
   free(b);
   {  int bvecsize = p->levels;
      byte * * bvec = (byte * *) malloc((bvecsize+1) * sizeof(byte *));
      int * buse = (int *) malloc((bvecsize+1) * sizeof(int));

      { int i; for (i = 0; i <= bvecsize; i++)
        {  bvec[i] = (byte *) malloc(p->blocksize);
           buse[i] = -1;
        }
      }
      p->buffers = bvec;
      p->buffuse = buse;
    }
    p->pblockno = -1;
    return p;
}

extern void DAclose(struct DAfile * p)
{  X_close(p->locator);
   free(p->buffuse);
   { int i; for (i = 0; i <= p->levels; i++) free(p->buffers[i]); }
   free(p->buffers);
   free(p->next);
   free(p);
}


/* DAterm(k,v,p) looks up term k in DA term file p, putting the result in v.
   gives true/false if the term is found/not found.
   The last index term <= the search term (or the 1st term) has
   information placed in v as in "tihdr".
*/

void putin(struct terminfo * v, byte * b, int i, int o, int blockno, struct DAfile * p)
{  v->p = b; v->term = b+o; v->o = i; v->n = blockno;
   o += b[o];
   o = unpackint(& v->po,b,o);
   o = unpackint(& v->psize,b,o);
   if (v->psize eq 0)
   {  o = unpackint(& v->shsize,b,o);
      o = unpackint(& v->shcount,b,o);
      o = unpackint(& v->psize,b,o);
   } else { v->shsize = 0; v->shcount = 0; }
   o = unpackint(& v->freq,b,o);
   o = unpackint(& v->pn,b,o);
   v->pn += I(b,p->blocksize-1-ILEN);
   /* Messiness of next 2 lines caused by expectations of QD2 */
   v->termno = 2;
   if (blockno eq p->firsttermblock && i eq 2) v->termno = 1;
}

int fstring(byte * k, int d, byte * b, int o, int klen)
   { return cb(klen-1,k,d+1,b[o]-1,b,o+1); }

extern int DAterm(byte * k, struct terminfo * v, struct DAfile * p)
{  int klen = k[0];
   byte * * bvec = p->buffers;
   int * buse = p->buffuse;
   int blockno = p->blockcount;  /* root block of index */
   byte * b;
   int i,j,o;
   int lev = 0;
   repeat
   {  b = bvec[lev];
      if (blockno ne buse[lev])
      {  readda(p,blockno,b);
           buse[lev] = blockno;
      }
      i = 2; j = L2(b,0);   /* i indexes an index entry */
      until (j-i <= 2)      /* '<' should never happen */
      {  int h = (i+j)/4*2;
         int o = L2(b,h);
         if (fstring(k,0,b,o,klen) < 0) j = h; else i = h;
      }
      o = L2(b,i);
      if (lev eq p->levels) break;
      unpackint(& blockno,b,o+b[o]);
      blockno += I(b,p->blocksize-ILEN-1);
      lev++;
   }
   putin(v,b,i,o,blockno,p);
   return (fstring(k,0,b,o,klen) eq 0);
}

static int s_D = 0;
static int s_E = 0;
static int s_wdf = 0;
static int Z = 0;

void f(struct postings * q)
{  byte * b = q->b;
   int o = q->o;
   repeat
   {  o = unpackint(& s_wdf,b,o);
      if (s_wdf < 8) switch (s_wdf)
      {  case 0: /* terminator */
            s_D = MAXINT; s_E = MAXINT; return;
         case 2: /* padder */
            {  int inc = q->blockinc;
               unless (inc < 0)
               {  inc++;
                  if (Z eq MAXINT)
                  {  s_D = MAXINT; s_E = MAXINT; return;
                  }
                  /* try the shortcut: */
                  {  int * sh = q->shortcut;
                     unless (sh eq 0)
                     {  int j = -1;
                        int i; for (i = 1; i <= sh[0]; i++)
                        {  if (sh[i] >= Z) break;
                           j = i;
                        }
                        if (inc < j+1) { inc = j+1; q->E = sh[j]; }
                     }
                  }
                  readda(q->p,q->blocknum+inc,b);
                  q->blockinc = inc; o = 0;
               }
            } continue;
        case 3: /* startoff */
            readda(q->p,q->blocknum,b);
            o = q->o; continue;
      }
      if ((s_wdf & 0X1) ne 0)
      {  o = unpackint(& s_D,b,o); s_D = s_D+q->E;
         o = unpackint(& s_E,b,o); s_E = s_E+s_D;
      } else
      {  o = unpackint(& s_D,b,o); s_D = s_D+q->E;
         s_E = s_D;
      }
      s_wdf = (s_wdf >> 1) - 4;
      q->o = o; return;
   }
}

byte * copybytes(int k, struct DAfile * p, int n, int o)
/* copy k bytes from block n offset o in DA file p */
{  int l = p->blocksize;
   byte * b = (byte *) malloc(k);
   int i = 0;
   byte * r = p->next;
   if (r eq 0) { r = (byte *) malloc(l); p->next = r; p->pblockno = -1; }
   until (k eq 0)
   {  int x = smaller(k,l-o);
      unless (p->pblockno eq n) { readda(p,n,r); p->pblockno = n; }
      mb(x,r,o,b,i); k = k-x; i = i+x; o = 0; n++;
   }
   return b;
}


int * read_shortcut(struct DAfile * p, int n, int o, int shsize, int shcount)
{  n = n+o/p->blocksize; o = o % p->blocksize;
   {  byte * b = copybytes(shsize,p,n,o);
      int * v = (int *) malloc((shcount+1) * sizeof(int));
      int c = unpackint(v+1,b,0);
      int i; for (i = 1; i < shcount; i++)
      {  c = unpackint(v+i+1,b,c); v[i+1] += v[i];  }
      v[0] = shcount;
      free(b);
      return v;
   }
}

extern struct postings * DAopenpostings(struct terminfo * v, struct DAfile * p)
{  struct postings * q = (struct postings *) malloc(sizeof(struct postings));
    q->p = p; q->D = 1; q->E = 0; q->wdf = 0; q->shortcut = 0;
    if (v->freq eq 0)
    {  byte * b = (byte *) malloc(1);
       q->b = b; q->o = 0;
       b[0] = 0;  /* terminator */
       return q;
    }
    {  int l = p->blocksize;
       int size = v->psize;
       int blocknum = v->pn;
       if (p->next eq 0) { p->next = (byte *) malloc(l); p->pblockno = -1; }
       if (l > size)
       {  q->b = copybytes(size,p,blocknum,v->po);
          q->blockinc = -1; q->o = 0;
       } else
       {  q->b = p->next;
          p->next = 0;
          q->blockinc = 0;
          q->blocknum = blocknum; q->o = v->po;

          if (v->shsize > 0)
          {  q->shortcut = read_shortcut(p,blocknum,q->o+size,
                                         v->shsize,v->shcount);
             packint(3, q->b, q->o); /* startoff */
          } else

          {  unless (p->pblockno eq blocknum)
             {  readda(p,blocknum,q->b);
                p->pblockno = blocknum;
             }
          }
       }
    }
    return q;
}

extern void DAreadpostings(struct postings * q, int style, int Z0)
{  Z = Z0;
   if (style > 0)
   {
      do { f(q); q->E = s_E; } while (s_E < Z);
      q->Doc = s_D; q->wdf = s_wdf; return;
   }
   /* interpret ranges if style eq 0 */
   s_D = q->D; s_E = q->E;
   while (s_E < Z or s_E < s_D) { f(q); q->E = s_E; q->wdf = s_wdf; }
   if (s_D < Z) s_D = Z;
   q->D = s_D+1; q->Doc = s_D; return;
}

extern void DAclosepostings(struct postings * q)
{  free(q->b);
   free(q->shortcut);
   free(q);
}

extern int DAnextterm(struct terminfo * v, struct DAfile * p)
{  byte * b = v->p;
   int i = v->o + 2;
   int blockno = v->n;
   if (i eq L2(b,0))
   {  do
      {  blockno++;
         if (blockno > p->lasttermblock) return false;
         readda(p,blockno,b);
      } until (b[p->blocksize-1] eq 0);
      i = 2;
      (p->buffuse)[p->levels] = blockno;
   }
   putin(v,b,i,L2(b,i),blockno,p);
   return true;
}

extern int DAprevterm(struct terminfo * v, struct DAfile * p)
{  byte * b = v->p;
   int i = v->o - 2;
   int blockno = v->n;
   if (i eq 0)
   {  do
      {  blockno--;
         if (blockno < p->firsttermblock) return false;
         readda(p,blockno,b);
      } until (b[p->blocksize-1] eq 0);
      i = L2(b,0)-2;
      (p->buffuse)[p->levels] = blockno;
   }
   putin(v,b,i,L2(b,i),blockno,p);
   return true;
}


void DAreadbytes(struct DAfile * p, int l, struct record * r, int notskipping)
{  int lev = p->levels;
   byte * b = p->buffers[lev];
   int blockno = p->buffuse[lev];
   int bsize = p->blocksize-1;
   int o = p->o;
   int d = 0;
   if (notskipping and (r->p eq 0 or l > r->size))
   {  free(r->p);
      r->p = (byte *) malloc(l+100);
      r->size = l+100;
   }
   while (l > bsize-o)
   {  if (notskipping) mb(bsize-o,b,o,r->p,d);
      d += bsize-o; l -= bsize-o;
      do
      {  blockno++;
         readda(p,blockno,b);
      } until (b[p->blocksize-1] eq BYTERANGE-1);
      o = 2;
   }
   if (notskipping) mb(l,b,o,r->p,d);
   p->buffuse[lev] = blockno; p->o = o+l;
}

void DAnextunit(struct DAfile * p, int m, int n, struct record * r)
{
#define RECHEADSIZE  (LWIDTH+2*ILEN)
#define ROFFSET      (LWIDTH+ILEN)
#define SHIFTUP      (LWIDTH*BITSPERBYTE)
   byte * b;
   int l, number;
   r->number = -1; /* error condition */
   repeat
   {  DAreadbytes(p,RECHEADSIZE,r,true); b = r->p;
      l = LOF(b,0) | b[ROFFSET] << SHIFTUP; /* old bug 9 */
      if (l eq 0) return;
      l -= RECHEADSIZE;
      number = I(b,LWIDTH);  /* the record number */
      if (n < number) return;
      if (m <= number) { DAreadbytes(p,l,r,true); break; }
      DAreadbytes(p,l,r,false);
   }
   r->number = number/2; return;
}

/* DAreadunit(p,key,range,r) reads one unit from the DA unit file
   p into r. The unit is specified by 'key,range', and the
   unit read will in fact be the first unit which has some overlap
   with this range. If there is no such unit r->number is set to -1,
   otherwise the number of the unit read.
   Following a DAreadunit(...) sequential reading can be done with
   DAnextunit(...).
*/

void DAreadunit(struct DAfile * p, int m, int n, struct record * r)
{
#define KBLEN (2*ILEN)
#define KBLEN2 (2*KBLEN)
   byte * * bvec = p->buffers;
   int * buse = p->buffuse;
   int blockno = p->blockcount;  /* root block of index */
   byte * b;
   int i,j;
   int lev = 0;
   if (p->itemcount eq 0) return; /* empty DA file */
   repeat
   {  b = bvec[lev];
      if (blockno ne buse[lev]) { readda(p,blockno,b); buse[lev] = blockno; }
      if (lev eq p->levels) break;
      i = 0; j = L2(b,0)-2;
      until (j-i <= KBLEN)
      {  int h = (i+j)/KBLEN2 * KBLEN;
         if (m < I(b,h+2)) j = h; else i = h;
      }
      i += 2+ILEN;
      blockno = I(b,i);
      lev++;
   }
   p->o = L2(b,0);
   if (p->o eq 0) { printf("STRUCTURE ERROR\n"); exit(1); }
   DAnextunit(p,m,n,r);
}

extern struct record * makerecord()
{  struct record * r = (struct record *) malloc(sizeof(struct record));
   r->size = 0; r->p = malloc(0); r->number = -1; return r;
}

extern void loserecord(struct record * r) { free(r->p); free(r); }

extern int DAgetrecord(struct DAfile * p, int n, struct record * r)
{  int u = 2*n; DAreadunit(p,u,u,r);
   return (r->number eq n);
}

extern struct terms * openterms(struct termvec * tv)
{  byte * p = tv->p;
   tv->nextterm = p+TVSTART;
   tv->l = p+TVSIZE(p,0);
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
   lwidth bytes; each wi is 1 byte.
*/

extern void readterms(struct termvec * tv)
{  byte * t = tv->nextterm;
   if (t >= tv->l) { tv->term = 0; return; }
   {  int flags = t[-1];
      tv->term = t;
      t = t+t[0]; /* t points after the k-form */
      tv->rel = flags & 1;
      tv->freq = -1;
      if (flags & 4) { tv->freq = I(t,0); t += ILEN; }
      tv->wdf = 0;
      if (flags & 2) { tv->wdf = t[0]; t++; }
      tv->termp = 0;
      if (flags & 8) { tv -> termp = t; t += L2(t,0); }
      tv->nextterm = t+1;
   }
}

extern struct termvec * maketermvec()
{  struct termvec * tv = (struct termvec *) malloc(sizeof(struct termvec));
   tv->size = 0; tv->p = malloc(0); tv->number = -1; return tv;
}

extern void losetermvec(struct termvec * tv) { free(tv->p); free(tv); }

extern int DAgettermvec(struct DAfile * p, int n, struct termvec * tv)
{  int u = 2*n+1; DAreadunit(p,u,u, (struct record *) tv);
   return (tv->number eq n);
}

/*----notes

DA term access:
---------------
The procedures defined as extern are:

struct DAfile * DAopen(byte * s, int type)
void DAclose(struct DAfile * p)
int DAterm(byte * k, struct terminfo * t, struct DAfile * p)
struct postings * DAopenpostings(struct terminfo * t, struct DAfile * p)
void DAreadpostings(struct postings * q, int style, int Z0)
void DAclosepostings(struct postings * q)

int DAnextterm(struct terminfo * v, struct DAfile * p)
int DAprevterm(struct terminfo * v, struct DAfile * p)

struct record * makerecord()
void loserecord(struct record * r)
int DAgetrecord(struct DAfile * p, int n, struct record * r)

struct termvec * maketermvec()
void losetermvec(struct termvec * tv)
int DAgettermvec(struct DAfile * p, int n, struct termvec * tv)

struct terms * openterms(struct termvec * tv)
void readterms(struct termvec * tv)

To open DA record and/or DA term files for example:

        struct DAfile * DA_r;
        struct DAfile * DA_t;
        DA_r = DAopen("/home/richard/test/d/R",DARECS);
        DA_t = DAopen("/home/richard/test/d/T",DATERMS);

and to close:

        DAclose(DA_r); DAclose(DA_t);

The total number of documents (or terms) in the DA file is given
by

        DA_r->itemcount       (DA_t->itemcount)

A term is expected to be a 'k-string', with length L in k[0], and
characters in k[1] ... k[L-1]. This is how they come in the packed term
vectors. Obviously it's easy to cast a string into this form:

       {  int len = strlen(s);
          memmove(k+1,s,len); k[0] = len+1;  // k-string in [k,0]

Then you look up a term k in DA_t using a terminfo structure:

       struct terminfo * t;
       found = DAterm(k, &t, DA_t);

'found' is 1 if found, 0 if not found, and if not found 'terminfo' is filled
in with info about the 'nearest' term (I guess you don't need a more exact
spec for 'nearest'). t->freq is the term frequency. Following a DAterm call, t
can be reused to open a posting list:

       string postings * q;
       q = DAopenpostings(&t, DA_t);

(thereafter t can be reused), and read from with

       DAreadpostings(q, style, Z);

If style eq 0 each call delivers a doc number in q->Doc and a wdf number in
q->wdf. Termination occurs when q->Doc eq MAXINT, after which MAXINT is
repeatedly delivered. Z eq 0 usually, but skipping forward can be done by
setting Z > 0. Then the next q->Doc to be delivered will be the first one >=
Z.

Is style eq 1, doc numbers are delivered back in ranges:

      q->Doc to q->E  (with a common q->wdf)

Z is correctly interpreted, so if the next range was 100 to 200 and the call
was DAreadpostings(q,1,137), q->Doc would be 137 and q->E 200.

styles 0 and 1 can't be mixed in successive calls (at least I don't think they
can!). BCPL Muscat uses style 1 reading (with wdf's ignored) for processing
purely Boolean queries.

Finally close with

    DAclosepostings(q)

(you can close before hitting MAXINT.)

Once a term has been found:

       struct terminfo * t;
       found = DAterm(k, d, &t, DA_t);

The next/previous term can be put into t with the calls

       found = DAnextterm(&t, DA_t);
       found = DAprevterm(&t, DA_t);

found is 0 when we hit the end/beginning of the term list. t->term gives the
term (in k-form) and t->freq gives its frequency.

DA record access:
-----------------

The procedures defines as extern are

struct record * makerecord()
void loserecord(struct record * r)
int DAgetrecord(struct DAfile * p, int n, struct record * r)

struct termvec * maketermvec()
void losetermvec(struct termvec * tv)
int DAgettermvec(struct DAfile * p, int n, struct termvec * tv)

struct terms * openterms(struct termvec * tv)
void readterms(struct termvec * tv)

To get records 148, 241 in turn, do this:

    record * r = makerecord();
    found = DAgetrecord(DA_r, 148, r); // record is at r->p
    ...
    found = DAgetrecord(DA_r, 241, r); // record is at r->p
    ...
    loserecord(r);

found is true/false according as found/not found. r keeps a single buffer for
the record read, so the second DAgetrecord overwrites the first.

To get the records together:

    record * r1 = makerecord();
    record * r2 = makerecord();
    found = DAgetrecord(DA_r, 148, r1); // record is at r1->p
    ...
    found = DAgetrecord(DA_r, 241, r2); // record is at r2->p
    ...
    loserecord(r1); loserecord(r2);

Exactly the same principle applies to termvecs:

    termvec * tv = maketermvec();
    found = DAgettermvec(DA_r, 148, tv);
    ...
    found = DAgettermvec(DA_r, 241, tv);
    ...
    losetermvec(tv);

or

    termvec * tv1 = maketermvec();
    termvec * tv2 = maketermvec();
    found = DAgettermvec(DA_r, 148, tv1);
    ...
    found = DAgettermvec(DA_r, 241, tv2);
    ...
    losetermvec(tv1); losetermvec(tv2);

A termvec can be read sequentially with

    openterms(tv);
    readterms(tv); readterms(tv);

Each call of readterms(tv) gives

    tv->term  - the term (k-form), or 0 when the list runs out
    tv->rel   - true/false according as term is/is not marked for
                relevance feedback
    tv->freq  - the term frequency, or -1 if this info is absent
    tv->wdf   - the wdf, or 0 id this info is absent
    tv->termp - the term's positional information, or 0 if absent.
                This can be unpicked by,

    if (tv->termp)
    {  byte * p = tv->termp;
       int l = L2(p,0);
       int i;
       for (i=2; i<l; i=i+LWIDTH+1)
       printf(" offset=%d; width=%d",LOF(p,i),p[i+LWIDTH]);
    }

    byte, L2, LWIDTH and LOF are as defined in daread.c. The term
    occurs at the given offsets in the record, with the given widths.

See test1.c, test2.c etc for examples of daread.c being used.

------*/
