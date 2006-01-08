/* $Id: cdb_unpack.c,v 1.5 2003/11/03 16:42:41 mjt Exp $
 * unpack 32bit integer
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include <config.h>

#include "cdb.h"

unsigned
cdb_unpack(const unsigned char buf[4])
{
  unsigned n = buf[3];
  n <<= 8; n |= buf[2];
  n <<= 8; n |= buf[1];
  n <<= 8; n |= buf[0];
  return n;
}
