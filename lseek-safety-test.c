#define _XOPEN_SOURCE 500
#define _FILE_OFFSET_BITS 64
#define _LARGE_FILES 1 /* For AIX */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/resource.h>

static sigjmp_buf jb;

static void sighandler(int signum) {
    siglongjmp(jb, signum);
}

int main(int argc, char ** argv) {
    if (argc != 2) {
	fprintf(stderr, "Usage: %s TESTFILE\n", argv[0]);
	return 1;
    }

    const char * filename = argv[1];

    int fd = open(filename, O_CREAT|O_RDWR|O_TRUNC, 0666);
    if (fd < 0) {
        perror("open failed");
	return 1;
    }

    struct rlimit rlim;
    if (getrlimit(RLIMIT_FSIZE, &rlim) < 0) {
	perror("rlimit");
    } else {
	if (rlim.rlim_cur != RLIM_INFINITY)
	    printf("RLIMIT_FSIZE cur = 0x%llx\n", (long long)rlim.rlim_cur);
	if (rlim.rlim_max != RLIM_INFINITY)
	    printf("RLIMIT_FSIZE max = 0x%llx\n", (long long)rlim.rlim_max);
    }

#define SHIFT (sizeof(off_t) * 8 - 2)
    volatile off_t off_t_max = (((off_t)1 << SHIFT)-1)|((off_t)1 << SHIFT);
    off_t pos = lseek(fd, off_t_max, SEEK_SET);
    if (pos != (off_t)-1) {
	printf("lseek to max off_t value 0x%llx\n worked (diff 0x%llx should be 0)\n", (long long)off_t_max, (long long)(pos - off_t_max));
    } else {
	perror("lseek failed");
	printf("attempted to set pos to 0x%llx\n", (long long)off_t_max);
	printf("probing to find actual max:\n");
	int slo = 0;
        int shi = SHIFT + 1;
	int sm = 43;
	goto in;
	while (shi - slo > 1) {
	    sm = slo + (shi - slo) / 2;
in: ;
	    off_t m = (((off_t)1 << sm) - 1) &~ 4095;
	    pos = lseek(fd, m, SEEK_SET);
	    if (pos == (off_t)-1) {
		printf("lseek to 0x%llx fails, errno=%d (%s)\n", (long long)m, errno, strerror(errno));
		shi = sm;
	    } else {
		printf("lseek to 0x%llx works\n", (long long)m);
		slo = sm;
	    }
	}

	off_t lo = (((off_t)1 << slo) - 1) &~ 4095;
	off_t hi = (((off_t)1 << shi) - 1) &~ 4095;
	off_t m = lo + 1;
	goto in2;
	while (hi - lo > 1) {
	    m = lo + (hi - lo) / 2;
in2:
	    pos = lseek(fd, m, SEEK_SET);
	    if (pos == (off_t)-1) {
		printf("lseek to 0x%llx fails, errno=%d (%s)\n", (long long)m, errno, strerror(errno));
		hi = m;
	    } else {
		printf("lseek to 0x%llx works\n", (long long)m);
		lo = m;
	    }
	}
	off_t_max = lo;
	printf("Max lseekable off_t = 0x%llx\n", (long long)off_t_max);
	pos = lseek(fd, off_t_max + 1, SEEK_SET);
	if (pos != -1)
	    printf("Odd, +1 now works\n");
	pos = lseek(fd, off_t_max, SEEK_SET);
	if (pos == -1)
	    printf("Odd, found limit now fails\n");
    }

    if (pos != off_t_max) {
	fprintf(stderr, "warning: lseek to 0x%llx actually set pos to 0x%llx\n", (long long)off_t_max, (long long)pos);
    }

    if (pwrite(fd, "hello\n", 6, 2) < 0) {
	perror("pwrite failed");
	return 1;
    }

    pos = lseek(fd, 0, SEEK_CUR);
    if (pos != off_t_max) {
	fprintf(stderr, "warning: pwrite changed file pos to 0x%llx\n", (long long)pos);
    }

    char buf[10];
    ssize_t r = pread(fd, buf, 10, 0);
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

    if (sigsetjmp(jb, 1) == 0) {
	signal(SIGXFSZ, sighandler);
	r = write(fd, "", 1);
	if (r >= 0) {
	    fprintf(stderr, "warning: write() succeeded, writing %lld bytes\n", (long long)r);
	    pos = lseek(fd, 0, SEEK_END);
	    fprintf(stderr, "lseek SEEK_END now returns 0x%llx (off_t_max was 0x%llx)\n", (long long)pos, (long long)off_t_max);
	} else {
	    fprintf(stderr, "info: write() failed with errno=%d (%s)\n", errno, strerror(errno));
	}
    } else {
	fprintf(stderr, "info: write() cause SIGXFSZ to be raised\n");
    }

    pos = lseek(fd, 0, SEEK_CUR);
    if (pos != off_t_max) {
	fprintf(stderr, "warning: write changed file pos to 0x%llx\n", (long long)pos);
    }

    fprintf(stderr, "Cool, that basically worked - please report the info and/or warnings given above\n");
    return 0;
}
