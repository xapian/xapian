//#define _XOPEN_SOURCE 500
//#define _FILE_OFFSET_BITS 64
//#define _LARGE_FILES 1 /* For AIX */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
//#include <setjmp.h>
//#include <signal.h>
//#include <sys/resource.h>
#include <io.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>

#define OFF_T unsigned __int64
#define SSIZE_T __int64

//static jmp_buf jb;

#if 0
static void sighandler(int signum) {
    longjmp(jb, signum);
}
#endif

static int
set_errno_from_getlasterror()
{
    int e;
    unsigned long winerr = GetLastError();
    switch (winerr) {
	case ERROR_FILENAME_EXCED_RANGE:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_PATH_NOT_FOUND:
	case ERROR_INVALID_DRIVE:
	case ERROR_NO_MORE_FILES:
	case ERROR_BAD_NETPATH:
	case ERROR_BAD_NET_NAME:
	case ERROR_BAD_PATHNAME:
	    e = ENOENT;
	    break;
	case ERROR_ARENA_TRASHED:
	case ERROR_NOT_ENOUGH_MEMORY:
	case ERROR_INVALID_BLOCK:
	case ERROR_NOT_ENOUGH_QUOTA:
	    e = ENOMEM;
	    break;
	case ERROR_LOCK_VIOLATION:
	case ERROR_LOCK_FAILED:
	case ERROR_SEEK_ON_DEVICE:
	case ERROR_NETWORK_ACCESS_DENIED:
	case ERROR_NOT_LOCKED:
	case ERROR_ACCESS_DENIED:
	case ERROR_CANNOT_MAKE:
	case ERROR_FAIL_I24:
	case ERROR_DRIVE_LOCKED:
	case ERROR_CURRENT_DIRECTORY:
	    e = EACCES;
	    break;
	case ERROR_INVALID_FUNCTION:
	case ERROR_INVALID_ACCESS:
	case ERROR_NEGATIVE_SEEK:
	case ERROR_INVALID_DATA:
	case ERROR_INVALID_PARAMETER:
	    e = EINVAL;
	    break;
	case ERROR_NO_PROC_SLOTS:
	case ERROR_NESTING_NOT_ALLOWED:
	case ERROR_MAX_THRDS_REACHED:
	    e = EAGAIN;
	    break;
	case ERROR_INVALID_HANDLE:
	case ERROR_INVALID_TARGET_HANDLE:
	case ERROR_DIRECT_ACCESS_HANDLE:
	    e = EBADF;
	    break;
	case ERROR_ALREADY_EXISTS:
	case ERROR_FILE_EXISTS:
	    e = EEXIST;
	    break;
	case ERROR_BROKEN_PIPE:
	    e = EPIPE;
	    break;
	case ERROR_DISK_FULL:
	    e = ENOSPC;
	    break;
	case ERROR_TOO_MANY_OPEN_FILES:
	    e = EMFILE;
	    break;
	case ERROR_WAIT_NO_CHILDREN:
	case ERROR_CHILD_NOT_COMPLETE:
	    e = ECHILD;
	    break;
	case ERROR_DIR_NOT_EMPTY:
	    e = ENOTEMPTY;
	    break;
	case ERROR_BAD_ENVIRONMENT:
	    e = E2BIG;
	    break;
	case ERROR_BAD_FORMAT:
	    e = ENOEXEC;
	    break;
	case ERROR_NOT_SAME_DEVICE:
	    e = EXDEV;
	    break;
	default:
	    if (winerr >= ERROR_WRITE_PROTECT && winerr <= ERROR_SHARING_BUFFER_EXCEEDED)
		e = EACCES;
	    else if (winerr >= ERROR_INVALID_STARTING_CODESEG && winerr <= ERROR_INFLOOP_IN_RELOC_CHAIN)
		e = ENOEXEC;
	    else
		e = EINVAL;
	    break;
    }
    errno = e;
    return -1;
}

SSIZE_T
pread(int fd, char * p, size_t n, OFF_T o)
{
    HANDLE h = (HANDLE)(intptr_t)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
	// _get_osfhandle() sets errno to EBADF.
	return -1;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = (DWORD)o;
    if (sizeof(OFF_T) > 4) {
	overlapped.OffsetHigh = o >> 32;
    }
    DWORD c;
    if (!ReadFile(h, p, n, &c, &overlapped)) {
	if (GetLastError() != ERROR_IO_PENDING)
	    return set_errno_from_getlasterror();
	if (!GetOverlappedResult(h,
				 &overlapped,
				 &c,
				 TRUE)) {
	    return set_errno_from_getlasterror();
	}
    }
    return c;
}

SSIZE_T
pwrite(int fd, const char * p, size_t n, OFF_T o)
{
    HANDLE h = (HANDLE)(intptr_t)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
	// _get_osfhandle() sets errno to EBADF.
	return -1;
    }

    OVERLAPPED overlapped;
    memset(&overlapped, 0, sizeof(overlapped));
    overlapped.Offset = (DWORD)o;
    if (sizeof(OFF_T) > 4) {
	overlapped.OffsetHigh = o >> 32;
    }
    DWORD c;
    if (!WriteFile(h, p, n, &c, &overlapped)) {
	if (GetLastError() != ERROR_IO_PENDING)
	    return set_errno_from_getlasterror();
	if (!GetOverlappedResult(h,
				 &overlapped,
				 &c,
				 TRUE)) {
	    return set_errno_from_getlasterror();
	}
    }
    return c;
}

static int openoverlapped(const char* filename) {
    DWORD dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
    /* Subsequent operations may open this file to read, write or delete it */
    DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    DWORD dwCreationDisposition = CREATE_ALWAYS;

    HANDLE handleWin =
	CreateFile(filename,
		   dwDesiredAccess,
		   dwShareMode,
		   NULL,
		   dwCreationDisposition,
		   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		   NULL);
    if (handleWin == INVALID_HANDLE_VALUE) {
	return set_errno_from_getlasterror();
    }

    /* Return a standard file descriptor. */
    return _open_osfhandle((intptr_t)(handleWin), O_CREAT|O_RDWR|O_TRUNC|O_BINARY);
}

int main(int argc, char ** argv) {
    if (argc != 2) {
	fprintf(stderr, "Usage: %s TESTFILE\n", argv[0]);
	return 1;
    }

    const char * filename = argv[1];

    int fd = openoverlapped(filename);
    if (fd < 0) {
        perror("open failed");
	return 1;
    }

    {
	// See if write() works at all.
	SSIZE_T r = write(fd, "", 1);
	if (r >= 0) {
	    fprintf(stderr, "info: initial write(fd, \"\", 1) succeeded, writing %lld bytes\n", (long long)r);
	    SSIZE_T pos = _lseeki64(fd, 0, SEEK_END);
	    fprintf(stderr, "lseek SEEK_END now returns 0x%llx\n", (long long)pos);
	    if (pos == -1)
		fprintf(stderr, "error: %s\n", strerror(errno));
	} else {
	    fprintf(stderr, "info: initial write() failed with errno=%d (%s)\n", errno, strerror(errno));
	}
    }

    /*
    struct rlimit rlim;
    if (getrlimit(RLIMIT_FSIZE, &rlim) < 0) {
	perror("rlimit");
    } else {
	if (rlim.rlim_cur != RLIM_INFINITY)
	    fprintf(stderr, "RLIMIT_FSIZE cur = 0x%llx\n", (long long)rlim.rlim_cur);
	if (rlim.rlim_max != RLIM_INFINITY)
	    fprintf(stderr, "RLIMIT_FSIZE max = 0x%llx\n", (long long)rlim.rlim_max);
    }
    */

#define SHIFT (sizeof(OFF_T) * 8 - 2)
    volatile SSIZE_T off_t_max = (((OFF_T)1 << SHIFT)-1)|((OFF_T)1 << SHIFT);
    errno = 0;
    SSIZE_T pos = _lseeki64(fd, off_t_max, SEEK_SET);
    if (pos != -1) {
	fprintf(stderr, "lseek to max OFF_T value 0x%llx\n worked (diff 0x%llx should be 0)\n", (long long)off_t_max, (long long)(pos - off_t_max));
    } else {
	perror("lseek failed");
	fprintf(stderr, "attempted to set pos to 0x%llx\n", (long long)off_t_max);
	fprintf(stderr, "probing to find actual max:\n");
	int slo = 0;
        int shi = SHIFT + 1;
	int sm = 43;
	goto in;
	while (shi - slo > 1) {
	    sm = slo + (shi - slo) / 2;
in: ;
	    OFF_T m = (((OFF_T)1 << sm) - 1) &~ 4095;
	    errno = 0;
	    pos = _lseeki64(fd, m, SEEK_SET);
	    if (pos == -1) {
		fprintf(stderr, "lseek to 0x%llx fails, errno=%d (%s)\n", (long long)m, errno, strerror(errno));
		shi = sm;
	    } else {
		fprintf(stderr, "lseek to 0x%llx works\n", (long long)m);
		slo = sm;
	    }
	}

	OFF_T lo = (((OFF_T)1 << slo) - 1) &~ 4095;
	OFF_T hi = (((OFF_T)1 << shi) - 1);
	OFF_T m = lo + 1;
	goto in2;
	while (hi - lo > 1) {
	    m = lo + (hi - lo) / 2;
in2:
	    errno = 0;
	    pos = _lseeki64(fd, m, SEEK_SET);
	    if (pos == -1) {
		fprintf(stderr, "lseek to 0x%llx fails, errno=%d (%s)\n", (long long)m, errno, strerror(errno));
		hi = m;
	    } else {
		fprintf(stderr, "lseek to 0x%llx works\n", (long long)m);
		lo = m;
	    }
	}
	off_t_max = lo;
	fprintf(stderr, "Max lseekable OFF_T = 0x%llx\n", (long long)off_t_max);
	errno = 0;
	pos = _lseeki64(fd, off_t_max + 1, SEEK_SET);
	if (pos != -1)
	    fprintf(stderr, "Odd, +1 now works, taking us to 0x%llx\n", (long long)pos);
	else
	    fprintf(stderr, "Good, seeking +1 fails, error: %s\n", strerror(errno));
	errno = 0;
	pos = _lseeki64(fd, off_t_max, SEEK_SET);
	if (pos == -1)
	    fprintf(stderr, "Odd, found limit now fails, error: %s\n", strerror(errno));

    }

    errno = 0;
    pos = _lseeki64(fd, off_t_max, SEEK_SET);
    if (pos != off_t_max) {
	fprintf(stderr, "warning: lseek to 0x%llx actually set pos to 0x%llx\n", (long long)off_t_max, (long long)pos);
    }
    if (pos == -1)
	fprintf(stderr, "error: %s\n", strerror(errno));

    if (pwrite(fd, "hello\n", 6, 2) < 0) {
	perror("pwrite failed");
	return 1;
    }

    pos = _lseeki64(fd, 0, SEEK_CUR);
    if (pos != off_t_max) {
	fprintf(stderr, "warning: pwrite changed file pos to 0x%llx\n", (long long)pos);
    }
    if (pos == -1)
	fprintf(stderr, "error: %s\n", strerror(errno));

    char buf[10];
    SSIZE_T r = pread(fd, buf, 10, 0);
    if (r < 0) {
	fprintf(stderr, "fd=%d\n", fd);
	perror("pread failed");
	return 1;
    }
    
    if (r != 8) {
	int i;
	fprintf(stderr, "pread() read %lld bytes, expected 8\ngot:", (long long)r);
	for (i = 0; i != (int)r; ++i) {
	    fprintf(stderr, " 0x%02x", (unsigned char)buf[i]);
	}
	fprintf(stderr, "\n");
	return 1;
    }

    if (memcmp(buf, "\0\0hello\n", 8) != 0) {
	int i;
	fprintf(stderr, "pread() didn't read the expected data back\ngot:");
	for (i = 0; i != 8; ++i) {
	    fprintf(stderr, " 0x%02x", (unsigned char)buf[i]);
	}
	fprintf(stderr, "\n");
	return 1;
    }

//    if (setjmp(jb) == 0) {
//	signal(SIGXFSZ, sighandler);
    {
	SSIZE_T r = write(fd, "", 1);
	if (r >= 0) {
	    fprintf(stderr, "warning: write() succeeded, writing %lld bytes\n", (long long)r);
	    pos = _lseeki64(fd, 0, SEEK_END);
	    fprintf(stderr, "lseek SEEK_END now returns 0x%llx (off_t_max was 0x%llx)\n", (long long)pos, (long long)off_t_max);
	    if (pos == -1)
		fprintf(stderr, "error: %s\n", strerror(errno));
	} else {
	    fprintf(stderr, "info: write() failed with errno=%d (%s)\n", errno, strerror(errno));
	}
    }
//    } else {
//	fprintf(stderr, "info: write() cause SIGXFSZ to be raised\n");

    pos = _lseeki64(fd, 0, SEEK_CUR);
    if (pos != off_t_max) {
	fprintf(stderr, "warning: write changed file pos to 0x%llx\n", (long long)pos);
    }
    if (pos == -1)
	fprintf(stderr, "error: %s\n", strerror(errno));

    fprintf(stderr, "Cool, that basically worked - please report the info and/or warnings given above\n");
    return 0;
}
