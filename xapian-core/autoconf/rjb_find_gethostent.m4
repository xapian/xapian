dnl RJB_FIND_GETHOSTENT
dnl Add libraries to LDFLAGS so that gethost* is available.
AC_DEFUN(RJB_FIND_GETHOSTENT,
[dnl
dnl Find the library which provides gethost*()
dnl

AC_MSG_CHECKING(for libraries needed for gethostbyname)
AC_TRY_LINK(
  [#include <netdb.h>],
  [gethostbyname ("feefifo");],
  AC_MSG_RESULT(none),
  AC_CHECK_LIB(nsl,
    gethostbyname,
    AC_MSG_RESULT(-lnsl)
    [LDFLAGS="$LDFLAGS -lnsl "],
    AC_MSG_ERROR(Library containing gethostbyname() not found)
    )
  )
])
