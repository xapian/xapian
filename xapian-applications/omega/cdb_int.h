/* $Id: cdb_int.h,v 1.11 2005/04/18 00:19:12 mjt Exp $
 * internal cdb library declarations
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "cdb.h"
#include "safeerrno.h"
#include <cstring>

#ifndef EPROTO
# define EPROTO EINVAL
#endif

struct cdb_rec {
  unsigned hval;
  unsigned rpos;
};
  
struct cdb_rl {
  struct cdb_rl *next;
  unsigned cnt;
  struct cdb_rec rec[254];
};

int _cdb_make_write(struct cdb_make *cdbmp,
		    const char *ptr, unsigned len);
int _cdb_make_fullwrite(int fd, const char *buf, unsigned len);
int _cdb_make_flush(struct cdb_make *cdbmp);
int _cdb_make_add(struct cdb_make *cdbmp, unsigned hval,
                  const void *key, unsigned klen,
                  const void *val, unsigned vlen);
