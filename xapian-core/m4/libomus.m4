# a macro to get the libs/cflags for libomus
# serial 1

dnl AM_PATH_LIBOMUS([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to LIBOMUS
dnl Defines LIBOMUS_CFLAGS and LIBOMUS_LIBS
dnl
AC_DEFUN(AM_PATH_LIBOMUS,
[dnl
dnl Get the cflags and libraries from the libomus-config script
dnl
AC_ARG_WITH(libomus-config,
[  --with-libomus-config=LIBOMUS_CONFIG   Location of libomus-config],
LIBOMUS_CONFIG="$withval")

AC_PATH_PROG(LIBOMUS_CONFIG, libomus-config, no)
AC_MSG_CHECKING(for libomus)
if test "$LIBOMUS_CONFIG" = "no"; then
  AC_MSG_RESULT(no)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(yes)
  LIBOMUS_CFLAGS=`$LIBOMUS_CONFIG --cflags $module_args`
  LIBOMUS_LIBS=`$LIBOMUS_CONFIG --libs $module_args`
  ifelse([$1], , :, [$1])
fi
AC_SUBST(LIBOMUS_CFLAGS)
AC_SUBST(LIBOMUS_LIBS)
])
