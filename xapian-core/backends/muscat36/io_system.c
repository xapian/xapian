/* io_system.c: Martin / Ollys IO code for split files.
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

#include <stdio.h>   /* sprintf etc */
#include <stdlib.h>  /* exit etc */
#include <fcntl.h>   /* O_RDONLY etc */
#include <sys/types.h> /* lseek */
#include <unistd.h> /* read, open, lseek */
#include "io_system.h"

#if 0
extern int X_findtoread(const char * s)
{  filehandle h = open(s, O_RDONLY, 0666);
   if (h < 0) { printf("Can't open %s\n",s); exit(1); }
   return h;
}

extern int X_point(filehandle h, int n, int m)
{ return lseek(h, (long)n*m, 0); } /* <0 is error */

extern int X_close(filehandle h)
{ return close(h); } /* non-zero is error */

extern int X_read(filehandle h, byte * b, int n)
{ return read(h, b, n); }
#else
#include <sys/stat.h>
#define DIAG(X)
#define FLAG(X) 0
#define HIGHEST_BFD 255
#define UNIX 1

#if defined(NT) || !defined(EINTR)
# define read_all(FD, BUF, C, POS) read((FD), (BUF), (C))
#else
#ifdef HAVE_PREAD
static ssize_t read_all(int fd, char *buf, size_t count, BIG_OFF_T pos) {
   int res;

  retry:
   res = BIGPREAD(fd, buf, count, pos);
   if (res < 0) {
      if (errno == EINTR) goto retry;
      /* genuine read error */
      return -1;
   }

   return res;
}
#else
# define read_all(FD, BUF, C, POS) read_all_((FD), (BUF), (C))
/* Like read, but will retry if a signal interupts the read */
static ssize_t read_all_(int fd, char *buf, size_t count) {
   int res;

  retry:
   res = read(fd, buf, count);
   if (res < 0) {
      if (errno == EINTR) goto retry;
      /* genuine read error */
      return -1;
   }

   return res;
}
#endif
#endif

/* max length tacked on to base filename to allow for appending "!<number>" */
#define EXTRA_MAX 8

/* lowest numbered BFD to "issue" */
/* e.g. set to 3 if we want to allow 0,1,2 to mean stdin/stdout/stderr */
#define LOWEST_BFD 3

/* max size for each sub file */
/* NB this must be *strictly* less than 2G */
#define MAXPOS (1024*1024*1024)

#define BIG_OFF_T off_t

/* hold the info we need about a given bfd (big file descriptor) */
typedef struct bfd {
   char *fname;
   int flags;
   int fd;
   int fileno;
   BIG_OFF_T pos;
   int /*umode_t*/ mode;
   int bfd_flags;
#define BFD_DONTFAKE 1 /* split files disabled */
} bfd;

#define bfd_dontfake(BFD) ((BFD)->bfd_flags & BFD_DONTFAKE)

/* array of pointers to extra info */
static bfd *bfds[HIGHEST_BFD + 1];

static int bf_inited = 0;

static int init_bf( void ) {
   int i;
   if (bfds[0]) return 1; /* do nothing on repeated calls */
   /* set up stdin, stdout and stderr */
   for ( i=0 ; i <= 2 ; i++ ) {
      bfd *p;
      p = calloc(1, sizeof(struct bfd));
      if (!p) return 0;
      p->bfd_flags = BFD_DONTFAKE;
      p->fd = i;
      bfds[i] = p;
   }
   bf_inited = 1;
   return 1;
}

#define BIG_STRUCT_STAT struct stat
#define OPEN open
#define BIGSTAT stat
#define BIGFSTAT fstat

extern int X_findtoread(const char *pathname) {
   char *fnm;
   bfd *p;
   int fd = -1;
   int bfd;
   int flags = O_CREAT /*| O_TRUNC*/;
   BIG_STRUCT_STAT st;

   if (!bf_inited) init_bf();

   fnm = malloc(strlen(pathname)+EXTRA_MAX+1);
   p = calloc(1, sizeof(struct bfd));
   if (!fnm || !p) goto err;

   strcpy(fnm, pathname);
   p->fileno = 0;
   p->pos = 0;

   fd = OPEN(fnm, O_RDONLY, 0644);
   flags = O_RDONLY;

   if (fd < 0) goto err;

   /* look for unused bfd -- !HACK! start at random elt better? */
   for( bfd = LOWEST_BFD ; ; ++bfd ) {
      if (bfd > HIGHEST_BFD) goto err;
      if (!bfds[bfd]) break;
   }

   p->fname = fnm;
   p->flags = flags;

   DIAG(("open flags = %d (stored as %d)\n", flags, p->flags));

   p->fd = fd;
   bfds[bfd] = p;
   p->bfd_flags = 0;
   p->mode = 0644;
   /* if the file is already over the limit, leave it be */
   if (BIGFSTAT(fd, &st) != -1) {
#ifdef UNIX
      p->mode = st.st_mode;
#endif
      if (st.st_size > MAXPOS) {
         p->bfd_flags |= BFD_DONTFAKE;
      } else if (FLAG('y')) {
         if (st.st_size < MAXPOS) {
            p->bfd_flags |= BFD_DONTFAKE;
         } else {
            /* see if the file has already split - if so, we have
             * to keep on faking
             */
            char *endp = fnm + strlen(fnm);
            strcpy(endp, "!1");
            if (BIGSTAT(fnm, &st) == -1) p->bfd_flags |= BFD_DONTFAKE;
            *endp = '\0';
         }
      }
   } else {
      if (FLAG('y')) p->bfd_flags |= BFD_DONTFAKE;
   }

   DIAG(( "opened bfd = %d\n", bfd ));
   return bfd;

   err:
   if (fnm) free(fnm);
   if (p) free(p);
   if (fd >= 0) close(fd);
   return -1;
}

#define UINT64 unsigned long long
#define BIGLSEEK lseek

extern int X_point(filehandle fd, int n, int m)
{
        if (bfd_dontfake(bfds[fd])) {
           bfds[fd]->pos = n * m;
           DIAG(( " - don't fake\n" ));
        } else {
           UINT64 bigpos = (UINT64)n*(UINT64)m;
           int fileno;
           BIG_OFF_T pos;

           fileno = bigpos >> 30;
           pos = bigpos & (((UINT64)1<<30) - 1);
           DIAG((" -> file %d pos %d", fileno, pos));
           if (fileno != bfds[fd]->fileno) {
              char *endp;

              if (close( bfds[fd]->fd ) == -1) return -1;
              bfds[fd]->fileno = fileno;

              endp = bfds[fd]->fname + strlen(bfds[fd]->fname);
              if (fileno>0) sprintf( endp, "!%d", fileno );
              DIAG(( " (opening file '%s', %d, %d)",bfds[fd]->fname, bfds[fd]->flags, bfds[fd]->mode));
              bfds[fd]->fd = OPEN(bfds[fd]->fname, bfds[fd]->flags, bfds[fd]->mode);
              *endp = '\0';
              if (bfds[fd]->fd==-1) {
                 /* !HACK! probably need to leave bfd in a better state */
                 DIAG((" didn't open!\n"));
                 return -1;
              }
           }
           else if (FLAG('z') && bfds[fd]->pos == pos) {
              /* reading of consecutive blocks is fairly common, so
               * optimise away the unneeded lseeks in this case */
              return 0;
           }
           bfds[fd]->pos = pos;
           DIAG(("\n"));
        }
        return BIGLSEEK(bfds[fd]->fd, bfds[fd]->pos, SEEK_SET);
}

extern int X_close(filehandle a1) {
    if ( a1 >= LOWEST_BFD && a1 <= HIGHEST_BFD && bfds[a1] ) {
        int fd = bfds[a1]->fd;
        free( bfds[a1]->fname );
        free( bfds[a1] );
        bfds[a1] = NULL;
        return close(fd);
    }
    /* don't close *stdout or *stderr */
    if (a1 == 1 || a1 == 2) return 0; /* ? STD(OUT/ERR)_FILENO ? */
    return -1;
}

extern int X_read(filehandle fd, byte * buf, int count) {
        long /*ssize_t*/ res=0, res2;

        DIAG(( "X_read( %d, %p %d ) ->",fd,buf,count));

        if (!bfd_dontfake(bfds[fd]) && (MAXPOS - bfds[fd]->pos < count)) {
           /* This will span 2 files so handle remainder of this file */
           char *endp;
           int c = MAXPOS - bfds[fd]->pos;

           DIAG(( " read( %d, %p %d ) +\n",bfds[fd]->fd,buf,c));
           if (c>0) {
              res = read_all(bfds[fd]->fd, buf, c, bfds[fd]->pos);
              if (res == -1) return -1;
              bfds[fd]->pos += res;
              /* check for partial success */
              DIAG(( "read of size %d returned %d\n",c,res));
              if (res<c) return res;
              /* otherwise, adjust buf and count */
              buf += res;
              count -= res;
           }
           /* and move onto the next file */
           /* close failing on read is probably not a problem for data
            * intergrity, but it does indicate something is amiss */
           if (close( bfds[fd]->fd ) == -1) return -1;

           endp = bfds[fd]->fname + strlen(bfds[fd]->fname);
           sprintf( endp, "!%d", ++bfds[fd]->fileno );

           DIAG(( "opening %s", bfds[fd]->fname ));
           bfds[fd]->fd = OPEN(bfds[fd]->fname, bfds[fd]->flags, bfds[fd]->mode);
           *endp = '\0';
           if (bfds[fd]->fd == -1) {
              /* !HACK! probably need to leave bfd in a better state */
              DIAG((" - didn't open\n"));
              return res;
           }
           bfds[fd]->pos = 0;
           if (count==0) return res; /* avoid potential bugs in 0 length xfer */
        }
        DIAG(( " read( %d, %p %d )",bfds[fd]->fd,buf,count));
        res2 = read_all(bfds[fd]->fd, buf, count, bfds[fd]->pos);
        if (res2 == -1) {
           /* if there was no previous read, return the error value */
           if (res == 0) return -1;
           /* otherwise, we've got a partial read */
           res2 = 0;
        }
        DIAG(( " OK\n"));
        bfds[fd]->pos += res2;
        DIAG(( "X_read size %d+%d=%d\n",res,res2,res+res2));
        return res+res2;
}
#endif
