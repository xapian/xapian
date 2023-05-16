/** @file
 * @brief C file implementing IO logging
 */
/* Compile the profiler by running `make`.
 *
 * Run using the xapian-io-profile script, which expects profiler.so to be in the same directory as the script:
 *
 * ./xapian-io-profile --log=log_file_name ./executable
 *
 * The resulting log can be analysed with strace-analyse:
 *
 * ./strace-analyse log_file_name
 */

#include <config.h>

#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* Work out the appropriate printf specifier for off_t. */
#if SIZEOF_OFF_T == SIZEOF_INT
# define FORMAT_OFF_T "d"
#elif SIZEOF_OFF_T == SIZEOF_LONG
# define FORMAT_OFF_T "ld"
#elif SIZEOF_OFF_T == SIZEOF_LONG_LONG
# define FORMAT_OFF_T "lld"
#endif

/* Pick a suitable `off64_t` type to use for pread64(), etc. */
#if SIZEOF_OFF_T == 8
# define OFF64_T off_t
# define FORMAT_OFF64_T FORMAT_OFF_T
#elif SIZEOF_LONG == 8
# define OFF64_T long
# define FORMAT_OFF64_T "ld"
#elif SIZEOF_LONG_LONG == 8
# define OFF64_T long long
# define FORMAT_OFF64_T "lld"
#endif

// function for logging calls

void logcall(const char *format, ...)
{
    static FILE *file_ptr = NULL;
    int saved_errno = errno;
    if (file_ptr == NULL) {
	char *fd_str = getenv("XAPIAN_IO_PROFILE_LOG_FD");
	if (fd_str) {
	    int fd = atoi(fd_str);
	    file_ptr = fdopen(fd, "w");
	} else {
	    file_ptr = stderr;
	}
    }
    va_list args_ptr;
    va_start(args_ptr, format);
    vfprintf(file_ptr, format, args_ptr);
    va_end(args_ptr);
    errno = saved_errno;
}

// wrapper for open()

typedef int (*real_open_t)(const char *, int, mode_t);

int open(const char *pathname, int flags, mode_t mode)
{
    static real_open_t real_open = NULL;
    if (!real_open) {
	real_open = (real_open_t)dlsym(RTLD_NEXT, "open");
    }
    int fd = real_open(pathname, flags, mode);
    // realpath can set errno
    int saved_errno = errno;
    char *abspath = realpath(pathname, NULL);
    if (abspath) {
	logcall("open(\"%s\",,) = %d\n", abspath, fd);
	free(abspath);
    } else {
	// display pathname in case realpath fails
	logcall("open(\"%s\",,) = %d\n", pathname, fd);
    }
    errno = saved_errno;
    return fd;
}

#ifdef HAVE_OPEN64
// wrapper for open64()

typedef int (*real_open64_t)(const char *, int, mode_t);

int open64(const char *pathname, int flags, mode_t mode)
{
    static real_open64_t real_open64 = NULL;
    if (!real_open64) {
	real_open64 = (real_open64_t)dlsym(RTLD_NEXT, "open64");
    }
    int fd = real_open64(pathname, flags, mode);
    // realpath can set errno
    int saved_errno = errno;
    char *abspath = realpath(pathname, NULL);
    if (abspath) {
	logcall("open(\"%s\",,) = %d\n", abspath, fd);
	free(abspath);
    } else {
	// display pathname in case realpath fails
	logcall("open(\"%s\",,) = %d\n", pathname, fd);
    }
    errno = saved_errno;
    return fd;
}
#endif

// wrapper for close()

typedef int (*real_close_t)(int);

int close(int fd)
{
    static real_close_t real_close = NULL;
    if (!real_close) {
	real_close = (real_close_t)dlsym(RTLD_NEXT, "close");
    }
    int return_val = real_close(fd);
    logcall("close(%d) = %d\n", fd, return_val);
    return return_val;
}

#ifdef HAVE_FDATASYNC
// wrapper for fdatasync()

typedef ssize_t (*real_fdatasync_t)(int);

ssize_t fdatasync(int fd)
{
    static real_fdatasync_t real_fdatasync = NULL;
    if (!real_fdatasync) {
	real_fdatasync = (real_fdatasync_t)dlsym(RTLD_NEXT, "fdatasync");
    }
    ssize_t return_val = real_fdatasync(fd);
    logcall("fdatasync(%d) = %ld\n", fd, return_val);
    return return_val;
}
#endif

#ifdef HAVE_FSYNC
// wrapper for fsync()

typedef ssize_t (*real_fsync_t)(int);

ssize_t fsync(int fd)
{
    static real_fsync_t real_fsync = NULL;
    if (!real_fsync) {
	real_fsync = (real_fsync_t)dlsym(RTLD_NEXT, "fsync");
    }
    ssize_t return_val = real_fsync(fd);
    logcall("fsync(%d) = %ld\n", fd, return_val);
    return return_val;
}
#endif

// wrapper for pread()

typedef ssize_t (*real_pread_t)(int, void *, size_t, off_t);

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    static real_pread_t real_pread = NULL;
    if (!real_pread) {
	real_pread = (real_pread_t)dlsym(RTLD_NEXT, "pread");
    }
    ssize_t return_val = real_pread(fd, buf, count, offset);
    logcall("pread(%d, \"\", %zu, %" FORMAT_OFF_T ") = %zd\n",
	    fd, count, offset, return_val);
    return return_val;
}

#ifdef HAVE_PREAD64
// wrapper for pread64()

typedef ssize_t (*real_pread64_t)(int, void *, size_t, OFF64_T);

ssize_t pread64(int fd, void *buf, size_t count, OFF64_T offset)
{
    static real_pread64_t real_pread64 = NULL;
    if (!real_pread64) {
	real_pread64 = (real_pread64_t)dlsym(RTLD_NEXT, "pread64");
    }
    ssize_t return_val = real_pread64(fd, buf, count, offset);
    logcall("pread(%d, \"\", %zu, %" FORMAT_OFF64_T ") = %zd\n",
	    fd, count, offset, return_val);
    return return_val;
}
#endif

// wrapper for pwrite()

typedef ssize_t (*real_pwrite_t)(int, const void *, size_t, off_t);

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    static real_pwrite_t real_pwrite = NULL;
    if (!real_pwrite) {
	real_pwrite = (real_pwrite_t)dlsym(RTLD_NEXT, "pwrite");
    }
    ssize_t return_val = real_pwrite(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %zu, %" FORMAT_OFF_T ") = %zd\n",
	    fd, count, offset, return_val);
    return return_val;
}

#ifdef HAVE_PWRITE64
// wrapper for pwrite64()

typedef ssize_t (*real_pwrite64_t)(int, const void *, size_t, OFF64_T);

ssize_t pwrite64(int fd, const void *buf, size_t count, OFF64_T offset)
{
    static real_pwrite64_t real_pwrite64 = NULL;
    if (!real_pwrite64) {
	real_pwrite64 = (real_pwrite64_t)dlsym(RTLD_NEXT, "pwrite64");
    }
    ssize_t return_val = real_pwrite64(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %zu, %" FORMAT_OFF64_T ") = %zd\n",
	    fd, count, offset, return_val);
    return return_val;
}
#endif
