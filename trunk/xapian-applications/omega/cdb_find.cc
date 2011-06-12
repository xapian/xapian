/* $Id: cdb_find.c,v 1.8 2003/11/03 16:42:41 mjt Exp $
 * cdb_find routine
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include <config.h>

#include "cdb_int.h"

#include <cstring>

using namespace std;

int
cdb_find(struct cdb *cdbp, const void *key, unsigned klen)
{
  const unsigned char *htp;	/* hash table pointer */
  const unsigned char *htab;	/* hash table */
  const unsigned char *htend;	/* end of hash table */
  unsigned httodo;		/* ht bytes left to look */
  unsigned pos, n;

  unsigned hval;

  if (klen >= cdbp->cdb_dend)	/* if key size is too large */
    return 0;

  hval = cdb_hash(key, klen);

  /* find (pos,n) hash table to use */
  /* first 2048 bytes (toc) are always available */
  /* (hval % 256) * 8 */
  htp = cdbp->cdb_mem + ((hval << 3) & 2047); /* index in toc (256x8) */
  n = cdb_unpack(htp + 4);	/* table size */
  if (!n)			/* empty table */
    return 0;			/* not found */
  httodo = n << 3;		/* bytes of htab to lookup */
  pos = cdb_unpack(htp);	/* htab position */
  if (n > (cdbp->cdb_fsize >> 3) /* overflow of httodo ? */
      || pos < cdbp->cdb_dend /* is htab inside data section ? */
      || pos > cdbp->cdb_fsize /* htab start within file ? */
      || httodo > cdbp->cdb_fsize - pos) /* entrie htab within file ? */
    return errno = EPROTO, -1;

  htab = cdbp->cdb_mem + pos;	/* htab pointer */
  htend = htab + httodo;	/* after end of htab */
  /* htab starting position: rest of hval modulo htsize, 8bytes per elt */
  htp = htab + (((hval >> 8) % n) << 3);

  for(;;) {
    pos = cdb_unpack(htp + 4);	/* record position */
    if (!pos)
      return 0;
    if (cdb_unpack(htp) == hval) {
      if (pos > cdbp->cdb_dend - 8) /* key+val lengths */
	return errno = EPROTO, -1;
      if (cdb_unpack(cdbp->cdb_mem + pos) == klen) {
	if (cdbp->cdb_dend - klen < pos + 8)
	  return errno = EPROTO, -1;
	if (memcmp(key, cdbp->cdb_mem + pos + 8, klen) == 0) {
	  n = cdb_unpack(cdbp->cdb_mem + pos + 4);
	  pos += 8;
	  if (cdbp->cdb_dend < n || cdbp->cdb_dend - n < pos + klen)
	    return errno = EPROTO, -1;
	  cdbp->cdb_kpos = pos;
	  cdbp->cdb_klen = klen;
	  cdbp->cdb_vpos = pos + klen;
	  cdbp->cdb_vlen = n;
	  return 1;
	}
      }
    }
    httodo -= 8;
    if (!httodo)
      return 0;
    if ((htp += 8) >= htend)
      htp = htab;
  }

}
