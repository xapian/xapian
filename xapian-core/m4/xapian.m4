# a macro to get the libs/cxxflags for compiling with xapian library
# serial 3

dnl OM_PATH_XAPIAN([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Xapian library
dnl Defines and AC_SUBST-s XAPIAN_CXXFLAGS and XAPIAN_LIBS
dnl
AC_DEFUN(OM_PATH_XAPIAN,
[dnl Get the cxxflags and libraries from the xapian-config script
AC_ARG_WITH(xapian-config,
[  --with-xapian-config    Location of xapian-config],
XAPIAN_CONFIG="$withval")

if test -z "$XAPIAN_CONFIG"; then
  XAPIAN_CONFIG=no
fi

if test no = "$XAPIAN_CONFIG"; then
  AC_PATH_PROG(XAPIAN_CONFIG, xapian-config, no)
fi

AC_MSG_CHECKING(for xapian)
if test no = "$XAPIAN_CONFIG"; then
  AC_MSG_RESULT(not found)
  ifelse([$2], , :, [$2])
else
  # run with exec to avoid leaking output on "real" bourne shells
  if (exec >&5 2>&5 ; $XAPIAN_CONFIG --libs --cxxflags; exit $?) then
    AC_MSG_RESULT(yes)
    XAPIAN_CXXFLAGS="`$XAPIAN_CONFIG --cxxflags`"
    XAPIAN_LIBS="`$XAPIAN_CONFIG --libs`"
    ifelse([$1], , :, [$1])
  else
    AC_MSG_ERROR([\`$XAPIAN_CONFIG --libs --cxxflags' doesn't work, aborting])
  fi
fi

AC_SUBST(XAPIAN_CXXFLAGS)
AC_SUBST(XAPIAN_LIBS)
])
