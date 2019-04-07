/** @file profiler.c
 * @brief C file implementing IO logging
 */
/* Compile the profiler with:
 * gcc -shared -fPIC -o profiler.so profiler.c -ldl
 *
 * To use it with a process, use:
 * LD_PRELOAD=/path/to/profiler.so /path/to/executable
 *
 * Running this library will produce an strace.log file,
 * which can be fed to strace-analyse to produce a log, by running:
 * ./strace-analyse strace.log
 */

#define _GNU_SOURCE

#include <dlfcn.h>
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
    if (!is_reset) {
	file_ptr = fopen("./strace.log", "w");
	is_reset = 1;
    }
    va_list args_ptr;
    va_start(args_ptr, format);
    vfprintf(file_ptr, format, args_ptr);
    va_end(args_ptr);
}

// wrapper for open()

typedef int (*real_open_t)(const char *, int, ...);

int open(const char *pathname, int flags, ...)
{
    int fd;
    if (flags & (O_CREAT | O_TMPFILE)) {
	va_list args_ptr;
	va_start(args_ptr, flags);
	fd =
	    ((real_open_t)dlsym(RTLD_NEXT, "open"))
	    (pathname, flags, va_arg(args_ptr, mode_t));
	va_end(args_ptr);
    } else {
	fd = ((real_open_t)dlsym(RTLD_NEXT, "open"))(pathname, flags);
    }
    char *abspath = realpath(pathname, NULL);
    logcall("open(\"%s\",,) = %d\n", abspath, fd);
    free(abspath);
    return fd;
}

// wrapper for open64()

typedef int (*real_open64_t)(const char *, int, ...);

int open64(const char *pathname, int flags, ...)
{
    int fd;
    if (flags & (O_CREAT | O_TMPFILE)) {
	va_list args_ptr;
	va_start(args_ptr, flags);
	fd =
	    ((real_open_t)dlsym(RTLD_NEXT, "open64"))
	    (pathname, flags, va_arg(args_ptr, mode_t));
	va_end(args_ptr);
    } else {
	fd = ((real_open_t)dlsym(RTLD_NEXT, "open64"))(pathname, flags);
    }
    char *abspath = realpath(pathname, NULL);
    logcall("open(\"%s\",,) = %d\n", abspath, fd);
    free(abspath);
    return fd;
}

// wrapper for close()

typedef int (*real_close_t)(int);

int close(int fd)
{
    int return_val = ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
    logcall("close(%d) = %d\n", fd, return_val);
    return return_val;
}

// wrapper for fdatasync()

typedef ssize_t (*real_fdatasync_t)(int);

ssize_t fdatasync(int fd)
{
    ssize_t return_val = ((real_fdatasync_t)dlsym(RTLD_NEXT, "fdatasync"))(fd);
    logcall("fdatasync(%d) = %ld\n", fd, return_val);
    return return_val;
}

// wrapper for fsync()

typedef ssize_t (*real_fsync_t)(int);

ssize_t fsync(int fd)
{
    ssize_t return_val = ((real_fsync_t)dlsym(RTLD_NEXT, "fsync"))(fd);
    logcall("fsync(%d) = %ld\n", fd, return_val);
    return return_val;
}

// wrapper for pread()

typedef ssize_t (*real_pread_t)(int, void *, size_t, off_t);

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pread_t)dlsym(RTLD_NEXT, "pread"))(fd, buf, count, offset);
    logcall("pread(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pread64()

typedef ssize_t (*real_pread64_t)(int, void *, size_t, off64_t);

ssize_t pread64(int fd, void *buf, size_t count, off64_t offset)
{
    ssize_t return_val =
	((real_pread64_t)dlsym(RTLD_NEXT, "pread64"))(fd, buf, count, offset);
    logcall("pread(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pwrite()

typedef ssize_t (*real_pwrite_t)(int, const void *, size_t, off_t);

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pwrite_t)dlsym(RTLD_NEXT, "pwrite"))(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}

// wrapper for pwrite64()

typedef ssize_t (*real_pwrite64_t)(int, const void *, size_t, off64_t);

ssize_t pwrite64(int fd, const void *buf, size_t count, off64_t offset)
{
    ssize_t return_val =
	((real_pwrite64_t)dlsym(RTLD_NEXT, "pwrite64"))(fd, buf, count, offset);
    logcall("pwrite(%d, \"\", %lu, %ld) = %ld\n",
	    fd, count, offset, return_val);
    return return_val;
}
