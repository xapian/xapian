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

#define MAX_CALL_LEN 5000

// function for logging calls

int reset = 0;

void logcall(const char *call)
{
    FILE *file_ptr;
    if (!reset) {
	file_ptr = fopen("./strace.log", "w");
	reset = 1;
    } else {
	file_ptr = fopen("./strace.log", "a");
    }
    fprintf(file_ptr, "%s", call);
    fclose(file_ptr);
}

// wrapper for open()

typedef int(*real_open_t)(const char *, int, ...);

int open(const char *pathname, int flags, ...)
{
    int fd;
    if (flags & O_CREAT || flags & O_TMPFILE) {
	va_list args_ptr;
	va_start(args_ptr, flags);
	fd =
	    ((real_open_t)dlsym(RTLD_NEXT, "open"))
	    (pathname, flags, va_arg(args_ptr, mode_t));
    } else {
	fd = ((real_open_t)dlsym(RTLD_NEXT, "open"))(pathname, flags);
    }
    char call[MAX_CALL_LEN];
    char *abspath = realpath(pathname, NULL);
    snprintf(call, MAX_CALL_LEN, "open(\"%s\",,) = %d\n", abspath, fd);
    free(abspath);
    logcall(call);
    return fd;
}

// wrapper for close()

typedef int(*real_close_t)(int);

int close(int fd)
{
    int return_val = ((real_close_t)dlsym(RTLD_NEXT, "close"))(fd);
    char call[MAX_CALL_LEN];
    snprintf(call, MAX_CALL_LEN, "close(%d) = %d\n", fd, return_val);
    logcall(call);
    return return_val;
}

// wrapper for fdatasync()

typedef ssize_t(*real_fdatasync_t)(int);

ssize_t fdatasync(int fd)
{
    ssize_t return_val = ((real_fdatasync_t)dlsym(RTLD_NEXT, "fdatasync"))(fd);
    char call[MAX_CALL_LEN];
    snprintf(call, MAX_CALL_LEN, "fdatasync(%d) = %ld\n", fd, return_val);
    logcall(call);
    return return_val;
}

// wrapper for fsync()

typedef ssize_t(*real_fsync_t)(int);

ssize_t fsync(int fd)
{
    ssize_t return_val = ((real_fsync_t)dlsym(RTLD_NEXT, "fsync"))(fd);
    char call[MAX_CALL_LEN];
    snprintf(call, MAX_CALL_LEN, "fsync(%d) = %ld\n", fd, return_val);
    logcall(call);
    return return_val;
}

// wrapper for pread()

typedef ssize_t(*real_pread_t)(int, void *, size_t, off_t);

ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pread_t)dlsym(RTLD_NEXT, "pread"))(fd, buf, count, offset);
    char call[MAX_CALL_LEN];
    char *new_buf = (char *)buf;
    for (size_t i = 0; i < count; ++i) {
	if (new_buf[i] == '\n') {
	    new_buf[i] = '\0';
	}
    }
    snprintf(call, MAX_CALL_LEN, "pread(%d, \"%s\", %lu, %ld) = %ld\n",
	     fd, new_buf, count, offset, return_val);
    logcall(call);
    return return_val;
}

// wrapper for pread64()

typedef ssize_t(*real_pread64_t)(int, void *, size_t, off_t);

ssize_t pread64(int fd, void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pread64_t)dlsym(RTLD_NEXT, "pread64"))(fd, buf, count, offset);
    char call[MAX_CALL_LEN];
    char *new_buf = (char *)buf;
    for (size_t i = 0; i < count; ++i) {
	if (new_buf[i] == '\n') {
	    new_buf[i] = '\0';
	}
    }
    snprintf(call, MAX_CALL_LEN, "pread64(%d, \"%s\", %lu, %ld) = %ld\n",
	     fd, new_buf, count, offset, return_val);
    logcall(call);
    return return_val;
}

// wrapper for pwrite()

typedef ssize_t(*real_pwrite_t)(int, const void *, size_t, off_t);

ssize_t pwrite(int fd, const void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pwrite_t)dlsym(RTLD_NEXT, "pwrite"))(fd, buf, count, offset);
    char call[MAX_CALL_LEN];
    char *new_buf = (char *)buf;
    for (size_t i = 0; i < count; ++i) {
	if (new_buf[i] == '\n') {
	    new_buf[i] = '\0';
	}
    }
    snprintf(call, MAX_CALL_LEN, "pwrite(%d, \"%s\", %lu, %ld) = %ld\n",
	     fd, new_buf, count, offset, return_val);
    logcall(call);
    return return_val;
}

// wrapper for pwrite64()

typedef ssize_t(*real_pwrite64_t)(int, const void *, size_t, off_t);

ssize_t pwrite64(int fd, const void *buf, size_t count, off_t offset)
{
    ssize_t return_val =
	((real_pwrite64_t)dlsym(RTLD_NEXT, "pwrite64"))(fd, buf, count, offset);
    char call[MAX_CALL_LEN];
    char *new_buf = (char *)buf;
    for (size_t i = 0; i < count; ++i) {
	if (new_buf[i] == '\n') {
	    new_buf[i] = '\0';
	}
    }
    snprintf(call, MAX_CALL_LEN, "pwrite64(%d, \"%s\", %lu, %ld) = %ld\n",
	     fd, new_buf, count, offset, return_val);
    logcall(call);
    return return_val;
}
