dnl @synopsis TYPE_SOCKLEN_T
dnl
dnl Check to see what type we should pass where some systems want socklen_t.
dnl Note that some versions of HP-UX define socklen_t yet want int, so just
dnl checking if socklen_t is defined isn't good enough.
dnl
dnl Loosely based on:
dnl http://mail.gnome.org/archives/xml/2001-August/msg00061.html
dnl
dnl Original author: Albert Chin
AC_DEFUN([TYPE_SOCKLEN_T],
[
  AC_MSG_CHECKING([for type to use for 5th parameter to getsockopt])
  AC_CACHE_VAL([xo_cv_socklen_t_equiv],
  [
    for t in socklen_t int size_t unsigned long "unsigned long"; do
      AC_TRY_COMPILE([
	#include <sys/types.h>
	#ifdef __WIN32__
	# include <winsock2.h>
	#else
        # include <sys/socket.h>
	#endif
      ],[
	$t len;
	getsockopt(0, 0, 0, 0, &len);
      ],[
        xo_cv_socklen_t_equiv="$t"
        break
      ])
    done
    if test -z "$xo_cv_socklen_t_equiv"; then
      AC_MSG_RESULT([not found])
      AC_MSG_ERROR([Failed to find type for 5th parameter to getsockopt])
    fi
  ])
  AC_MSG_RESULT([$xo_cv_socklen_t_equiv])
  AC_DEFINE_UNQUOTED(SOCKLEN_T, [$xo_cv_socklen_t_equiv],
		     [type to use for 5th parameter to getsockopt])
])
