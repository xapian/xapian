dnl CME_FIND_HSTRERROR
dnl Add libraries to LDFLAGS so that hstrerror is available.
AC_DEFUN(CME_FIND_HSTRERROR,
[dnl
dnl Find the library which provides hstrerror()
dnl

AC_MSG_CHECKING(for libraries needed for hstrerror)
AC_TRY_LINK(
  [#include <netdb.h>],
  [hstrerror (h_errno);],
  AC_MSG_RESULT(none),
  AC_CHECK_LIB(resolv,
    hstrerror,
    AC_MSG_RESULT(-lresolv)
    [LDFLAGS="$LDFLAGS -lresolv "],
    AC_MSG_ERROR(Library containing hstrerror() not found)
    )
  )
])
