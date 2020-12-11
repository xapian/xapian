/** @file
 * @brief C file implementing IO logging
 */
/* Compile the profiler with:
 * gcc -shared -fPIC -o /path/to/profiler.so profiler.c -ldl
 *
 * To use it with a process, change the path to profiler.so inside the
 * xapian-io-profile script, then use:
 * ./xapian-io-profile --log=log_file_name ./executable
 *
 * Running this library will produce a log file,
 * which can be fed to strace-analyse to produce a log, by running:
 * ./strace-analyse log_file_name
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// function for logging calls

static int is_reset = 0;

void logcall(const char *format, ...)
{
    static FILE *file_ptr;
    int saved_errno = errno;
    if (!is_reset) {
	int fd;
	char *fd_str = getenv("XAPIAN_IO_PROFILE_LOG_FD");
	if (fd_str) {
	    fd = atoi(fd_str);
	} else {
	    fd = 2; // log to stderr
	}
	file_ptr = fdopen(fd, "w");
	is_reset = 1;
    }
    va_list args_ptr;
    va_start(args_ptr, format);
    vfprintf(file_ptr, format, args_ptr);
    va_end(args_ptr);
    errno = saved_errno;
}

// wrapper for open()

typedef int (*real_open_t)(const char *, int, ...);

int open(const char *pathname, int flags, ...)
{
    static real_open_t real_open = NULL;
    if (!real_open) {
	real_open = (real_open_t)dlsym(RTLD_NEXT, "open");
    }
    int fd;
    if (flags & (O_CREAT | O_TMPFILE)) {
	va_list args_ptr;
	va_start(args_ptr, flags);
	fd = real_open(pathname, flags, va_arg(args_ptr, mode_t));
	va_end(args_ptr);
    } else {
	fd = real_open(pathname, flags);
    }
    // realpath can set errno
    int saved_errno = errno;
    char *abspath = realpath(pathname, NULL);
    if (abspath) {
	logcall("open(\"%s\",,) = %d\n", abspath, fd);
	free(abspath);
    } else {
	// display pathname incase realpath fails
	logcall("open(\"%s\",,) = %d\n", pathname, fd);
    }
    errno = saved_errno;
    return fd;
}

// wrapper for open64()

typedef int (*real_open64_t)(const char *, int, ...);

int open64(const char *pathname, int flags, ...)
{
    static real_open64_t real_open64 = NULL;
    if (!real_open64) {
	real_open64 = (real_open64_t)dlsym(RTLD_NEXT, "open64");
    }
    int fd;
    if (flags & (O_CREAT | O_TMPFILE)) {
	va_list args_ptr;
	va_start(args_ptr, flags);
	fd = real_open64(pathname, flags, va_arg(args_ptr, mode_t));
	va_end(args_ptr);
    } else {
	fd = real_open64(pathname, flags);
    }
    // realpath can set errno
    int saved_errno = errno;
    char *abspath = realpath(pathname, NULL);
    if (abspath) {
	logcall("open(\"%s\",,) = %d\n", abspath, fd);
	free(abspath);
    } else {
	// display pathname incase realpath fails
	logcall("open(\"%s\",,) = %d\n", pathname, fd);
    }
    errno = saved_errno;
    return fd;
}

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

// wrapper for pread()

typedef ssize_t (*real_pread_t)(int, void *, size_t, off_t);

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    static real_pread_t real_pread = NULL;
    if (!real_pread) {
	real_pread = (real_pread_t)dlsym(RTLD_NEXT, "pread");
    }
    ssize_t return_val = real_pread(fd, buf, count, offset);
    logcall("pread(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pread64()

typedef ssize_t (*real_pread64_t)(int, void *, size_t, off_t);

ssize_t pread64(int fd, void *buf, size_t count, off_t offset)
{
    static real_pread64_t real_pread64 = NULL;
    if (!real_pread64) {
	real_pread64 = (real_pread64_t)dlsym(RTLD_NEXT, "pread64");
    }
    ssize_t return_val = real_pread64(fd, buf, count, offset);
    logcall("pread(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pwrite()

typedef ssize_t (*real_pwrite_t)(int, const void *, size_t, off_t);

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    static real_pwrite_t real_pwrite = NULL;
    if (!real_pwrite) {
	real_pwrite = (real_pwrite_t)dlsym(RTLD_NEXT, "pwrite");
    }
    ssize_t return_val = real_pwrite(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pwrite64()

typedef ssize_t (*real_pwrite64_t)(int, const void *, size_t, off_t);

ssize_t pwrite64(int fd, const void *buf, size_t count, off_t offset)
{
    static real_pwrite64_t real_pwrite64 = NULL;
    if (!real_pwrite64) {
	real_pwrite64 = (real_pwrite64_t)dlsym(RTLD_NEXT, "pwrite64");
    }
    ssize_t return_val = real_pwrite64(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}
