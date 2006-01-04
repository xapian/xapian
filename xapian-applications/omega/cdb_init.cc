/* $Id: cdb_init.c,v 1.9 2003/11/03 21:42:20 mjt Exp $
 * cdb_init, cdb_free and cdb_read routines
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include <sys/types.h>
#ifdef _WIN32
# include <windows.h>
#else
# include <sys/mman.h>
#endif
#include <sys/stat.h>
#include "cdb_int.h"

int
cdb_init(struct cdb *cdbp, int fd)
{
  struct stat st;
  unsigned char *mem;
  unsigned fsize, dend;
#ifdef _WIN32
  HANDLE hFile, hMapping;
#endif

  /* get file size */
  if (fstat(fd, &st) < 0)
    return -1;
  /* trivial sanity check: at least toc should be here */
  if (st.st_size < 2048)
    return errno = EPROTO, -1;
  fsize = (unsigned)(st.st_size & 0xffffffffu);
  /* memory-map file */
#ifdef _WIN32
  hFile = (HANDLE) _get_osfhandle(fd);
  if(hFile == (HANDLE) -1) return -1;
  hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (!hMapping) return -1;
  mem = (unsigned char *)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
  if (!mem) return -1;
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

void
cdb_free(struct cdb *cdbp)
{
  if (cdbp->cdb_mem) {
#ifdef _WIN32
    HANDLE hFile, hMapping;
#endif

#ifdef _WIN32
    hFile = (HANDLE) _get_osfhandle(cdbp->cdb_fd);
    hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
    UnmapViewOfFile((void*) cdbp->cdb_mem);
    CloseHandle(hMapping);
#else
    munmap((void*)cdbp->cdb_mem, cdbp->cdb_fsize);
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
