# a macro to get the libs/cflags for xapian library
# serial 3

dnl OM_PATH_XAPIAN[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Xapian library
dnl Defines XAPIAN_CFLAGS and XAPIAN_LIBS
dnl
AC_DEFUN(OM_PATH_XAPIAN,
[dnl
dnl Get the cflags and libraries from the xapian-config script
dnl
AC_ARG_WITH(xapian-config,
[  --with-xapian-config   Location of xapian-config],
XAPIAN_CONFIG="$withval")

if test "x$XAPIAN_CONFIG" = "x"; then
  XAPIAN_CONFIG=no
fi

if test "x$XAPIAN_CONFIG" = "xno"; then
  AC_PATH_PROG(XAPIAN_CONFIG, xapian-config, no)
fi

AC_MSG_CHECKING(for xapian)
if test "x$XAPIAN_CONFIG" = "xno"; then
  AC_MSG_RESULT(not found)
  ifelse([$2], , :, [$2])
else
  if $XAPIAN_CONFIG --check; then
    AC_MSG_RESULT(yes)
    XAPIAN_CFLAGS=`$XAPIAN_CONFIG --cflags`
    XAPIAN_LIBS=`$XAPIAN_CONFIG --libs`
    ifelse([$1], , :, [$1])
  else
    AC_MSG_RESULT(bad installation)
  fi
fi

AC_SUBST(XAPIAN_CFLAGS)
AC_SUBST(XAPIAN_LIBS)
])
