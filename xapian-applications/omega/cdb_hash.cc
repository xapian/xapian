/* $Id: cdb_hash.c,v 1.5 2003/11/03 16:42:41 mjt Exp $
 * cdb hashing routine
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include <config.h>

#include "cdb.h"

unsigned
cdb_hash(const void *buf, unsigned len)
{
  const unsigned char *p = (const unsigned char *)buf;
  const unsigned char *end = p + len;
  unsigned hash = 5381;	/* start value */
  while (p < end)
    hash = (hash + (hash << 5)) ^ *p++;
  return hash;
}
