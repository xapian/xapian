/* Hand written and maintained config.h for MSVC nmake file build. */

/* Define if ftime returns void */
#define FTIME_RETURNS_VOID 1

/* Define to 1 if you have the <arpa/inet.h> header file. */
/* #undef HAVE_ARPA_INET_H */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define to 1 if you have the `gettimeofday' function. */
/* #undef HAVE_GETTIMEOFDAY */

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the `iconv' library (-liconv). */
/* #undef HAVE_LIBICONV */

/* Define to 1 if you have the `lstat' function. */
/* #undef HAVE_LSTAT */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkdtemp' function. */
/* #undef HAVE_MKDTEMP */

/* Define to 1 if you have the `mmap' function. */
/* #undef HAVE_MMAP */

/* Define to 1 if you have the <netinet/in.h> header file. */
/* #undef HAVE_NETINET_IN_H */

/* Define to 1 if you have the `posix_fadvise' function. */
/* #undef HAVE_POSIX_FADVISE */

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
/* #undef HAVE_SYS_WAIT_H */

/* Define to 1 if you have the <unistd.h> header file. */
/* #undef HAVE_UNISTD_H */

/* Define to 1 if you have the <stdint.h> header file and it can be used in
   C++ code. */
/* #undef HAVE_WORKING_STDINT_H */

/* type of input pointer for iconv */
#define ICONV_INPUT_TYPE char*

/* Name of package */
#define PACKAGE "xapian-omega"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "xapian-omega"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "xapian-omega 1.2.4"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "xapian-omega"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.2.4"

/* Define to the name of a function implementing snprintf but not caring about
   ISO C90 return value semantics (if one exists) */
/* #undef SNPRINTF */

/* Define to the name of a function implementing snprintf with ISO C90
   semantics (if one exists) */
/* #undef SNPRINTF_ISO */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if iconv() should be used for converting character sets. */
/* USE_ICONV */

/* Version number of package */
#define VERSION "1.2.4"

/* Define to 1 if your processor stores words with the most significant byte
   first (like Motorola and SPARC, unlike Intel and VAX). */
/* #undef WORDS_BIGENDIAN */

/* Define to `int' if <sys/types.h> does not define. */
#define mode_t int

/* Define to `int' if <sys/types.h> does not define. */
#define pid_t int

/* Define to `int' if <sys/types.h> does not define. */
#ifdef _WIN64
# define ssize_t __int64
#else
# define ssize_t long
#endif

/* Define rare() as identity, since we don't have this in MSVC (See
 * the section "Branch Prediction Hints" in xapian-core/HACKING) */
#define rare(COND) (COND)

/* Define usual() as identity, since we don't have this in MSVC (See
 * the section "Branch Prediction Hints" in xapian-core/HACKING) */
#define usual(COND) (COND)

/* Must define this here, we can't do it as a build parameter as quotes are
   stripped. */
#define CONFIGFILE_SYSTEM "omega.conf"

/* Disable stupid MSVC "performance" warning for converting int to bool. */
#ifdef _MSC_VER
# pragma warning(disable:4800)
#endif

/* Disable MSVC warning about macros with missing parameters */
#ifdef _MSC_VER
# pragma warning(disable:4003)
#endif

/* no defined helper directory */
#define PKGLIBBINDIR ""
