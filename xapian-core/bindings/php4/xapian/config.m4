dnl ***********************************************************************
dnl ** THIS config.m4 is provided for PHPIZE and PHP's consumption NOT   **
dnl ** for any part of the rest of the Xapian build system               **
dnl ***********************************************************************

PHP_ARG_WITH(xapian, for Xapian support,
[  --with-xapian[=DIR]             Include Xapian support.])

if test "$PHP_XAPIAN" != "no"; then
  PHP_EXTENSION(xapian, $ext_shared)
  for i in $PHP_XAPIAN /usr/local /usr; do
    if test -f $i/include/om.h; then
      XAPIAN_INCDIR=$i/include
      XAPIAN_DIR=$i
    fi
    if test -f $i/include/om/om.h; then
      XAPIAN_INCDIR=$i/include/om
      XAPIAN_DIR=$i
    fi

    if test -f $i/lib/libxapian.a; then
      XAPIAN_LIBDIR=$i/lib
      XAPIAN_DIR=$i
    fi
  done

  if test -z "$XAPIAN_INCDIR"; then
    AC_MSG_ERROR(Cannot find Xapian include dir)
  fi

  if test -z "$XAPIAN_LIBDIR"; then
    AC_MSG_ERROR(Cannot find XAPIAN lib dir)
  fi

  PHP_REQUIRE_CXX
  AC_CHECK_LIB(stdc++, cin)
  AC_DEFINE(HAVE_XAPIAN, 1, [ ])
  PHP_SUBST(XAPIAN_SHARED_LIBADD)
  AC_DEFINE_UNQUOTED(PHP_XAPIAN_DIR, "$XAPIAN_DIR", [ ])
  AC_ADD_LIBRARY_WITH_PATH(xapian, $XAPIAN_LIBDIR, XAPIAN_SHARED_LIBADD)
  AC_ADD_INCLUDE($XAPIAN_INCDIR)
fi
