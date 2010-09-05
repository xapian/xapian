/* $Id: cdb_init.c,v 1.9 2003/11/03 21:42:20 mjt Exp $
 * cdb_init, cdb_free and cdb_read routines
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_MMAP
# include <sys/mman.h>
#else
# include "safeunistd.h"
# include <cstdlib>
#endif
#ifdef __WIN32__
# include "safewindows.h"
#endif
#include "safesysstat.h"
#include "cdb_int.h"
#include <cstring>

using namespace std;

int
cdb_init(struct cdb *cdbp, int fd)
{
  struct stat st;
  unsigned char *mem;
  unsigned fsize, dend;
#ifndef HAVE_MMAP
#ifdef _WIN32
  HANDLE hFile, hMapping;
#else
  size_t size;
  unsigned char *p;
#endif
#endif

  /* get file size */
  if (fstat(fd, &st) < 0)
    return -1;
  /* trivial sanity check: at least toc should be here */
  if (st.st_size < 2048)
    return errno = EPROTO, -1;
  fsize = (unsigned)(st.st_size & 0xffffffffu);
  /* memory-map file */
#ifndef HAVE_MMAP
#ifdef _WIN32
  hFile = (HANDLE) _get_osfhandle(fd);
  if(hFile == (HANDLE) -1) return -1;
  hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (!hMapping) return -1;
  mem = (unsigned char *)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
  if (!mem) return -1;
#else
  // No mmap, so take the very crude approach of malloc and read the whole file in!
  if ((mem = (unsigned char *)malloc(fsize)) == NULL)
    return -1;
  size = fsize;
  p = mem;
  while (size > 0) {
    ssize_t n = read(fd, (void*)p, size);
    if (n == -1)
      return -1;
    p += n;
    size -= n;
  }
#endif
#else
  mem = (unsigned char *)mmap(NULL, fsize, PROT_READ, MAP_SHARED, fd, 0);
  if (mem == (unsigned char *)-1)
    return -1;
#endif /* _WIN32 */

  cdbp->cdb_fd = fd;
  cdbp->cdb_fsize = fsize;
  cdbp->cdb_mem = mem;

#if 0
  /* XXX don't know well about madvise syscall -- is it legal
     to set different options for parts of one mmap() region?
     There is also posix_madvise() exist, with POSIX_MADV_RANDOM etc...
  */
#ifdef MADV_RANDOM
  /* set madvise() parameters. Ignore errors for now if system
     doesn't support it */
  madvise(mem, 2048, MADV_WILLNEED);
  madvise(mem + 2048, cdbp->cdb_fsize - 2048, MADV_RANDOM);
#endif
#endif

  cdbp->cdb_vpos = cdbp->cdb_vlen = 0;
  cdbp->cdb_kpos = cdbp->cdb_klen = 0;
  dend = cdb_unpack(mem);
  if (dend < 2048) dend = 2048;
  else if (dend >= fsize) dend = fsize;
  cdbp->cdb_dend = dend;

  return 0;
}

#ifdef __cplusplus
class VoidStarOrCharStar {
    void *p;
  public:
    VoidStarOrCharStar(const void *p_) : p(const_cast<void*>(p_)) { }
    VoidStarOrCharStar(const char *p_) : p(const_cast<char*>(p_)) { }
    operator void*() { return p; }
    operator char*() { return static_cast<char*>(p); }
};
#endif

void
cdb_free(struct cdb *cdbp)
{
  if (cdbp->cdb_mem) {
#ifdef _WIN32
    HANDLE hFile, hMapping;
#endif

#ifndef HAVE_MMAP
#ifdef __cplusplus
    void * p = const_cast<void*>((const void*)cdbp->cdb_mem);
#else
    void * p = (void*)cdbp->cdb_mem;
#endif
#ifdef _WIN32
    hFile = (HANDLE) _get_osfhandle(cdbp->cdb_fd);
    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    UnmapViewOfFile(p);
    CloseHandle(hMapping);
#else
    free(p);
#endif
#else
#ifdef __cplusplus
    /* Solaris sys/mman.h defines munmap as taking char* unless __STDC__ is
     * defined (which it isn't in C++).
     */
    VoidStarOrCharStar p(cdbp->cdb_mem);
#else
    void * p = (void*)cdbp->cdb_mem;
#endif
    munmap(p, cdbp->cdb_fsize);
#endif /* _WIN32 */
    cdbp->cdb_mem = NULL;
  }
  cdbp->cdb_fsize = 0;
}

const void *
cdb_get(const struct cdb *cdbp, unsigned len, unsigned pos)
{
  if (pos > cdbp->cdb_fsize || cdbp->cdb_fsize - pos < len) {
    errno = EPROTO;
    return NULL;
  }
  return cdbp->cdb_mem + pos;
}

int
cdb_read(const struct cdb *cdbp, void *buf, unsigned len, unsigned pos)
{
  const void *data = cdb_get(cdbp, len, pos);
  if (!data) return -1;
  memcpy(buf, data, len);
  return 0;
}
