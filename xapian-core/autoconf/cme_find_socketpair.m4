dnl CME_FIND_SOCKETPAIR
dnl Add libraries to LDFLAGS so that socketpair is available.
AC_DEFUN(CME_FIND_SOCKETPAIR,
[dnl
dnl Find the library which provides socketpair()
dnl

AC_MSG_CHECKING(for libraries needed for socketpair)
AC_TRY_LINK(,[socketpair()],
	    AC_MSG_RESULT(none),
	    AC_CHECK_LIB(socket, socketpair,
			 AC_MSG_RESULT(-lsocket)
			 [LDFLAGS="$LDFLAGS -lsocket "],
			 AC_MSG_ERROR(Library containing socketpair() not found)
			)
	   )
])
