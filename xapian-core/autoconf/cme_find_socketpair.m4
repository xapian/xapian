dnl CME_FIND_SOCKETPAIR
dnl Add libraries to LDFLAGS so that socketpair is available.
AC_DEFUN(CME_FIND_SOCKETPAIR,
[dnl
dnl Find the library which provides socketpair()
dnl

AC_TRY_LINK(,[socketpair()],
	    [],
	    AC_CHECK_LIB(socket, socketpair,
			 [LDFLAGS="$LDFLAGS -lsocket "],
			 AC_MSG_ERROR(Library containing socketpair() not found)
			)
	   )
)

