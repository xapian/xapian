# a macro to get the libs/cflags for omseek library
# serial 3

dnl OM_PATH_OMSEEK[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Omseek library
dnl Defines OMSEEK_CFLAGS and OMSEEK_LIBS
dnl
AC_DEFUN(OM_PATH_OMSEEK,
[dnl
dnl Get the cflags and libraries from the omseek-config script
dnl
AC_ARG_WITH(omseek-config,
[  --with-omseek-config   Location of omseek-config],
OMSEEK_CONFIG="$withval")

AC_ARG_WITH(omseek-uninst,
[  --with-omseek-uninst   Location of top dir of uninstalled omseek],
OMSEEK_UNINST="$withval")

AC_ARG_WITH(omseek-uninst_exec,
[  --with-omseek-uninst-exec  Location of built omseek, if srcdir not builddir],
OMSEEK_UNINST_EXEC="$withval")


if test "x$OMSEEK_CONFIG" = "x"; then
  OMSEEK_CONFIG=no
fi

if test "x$OMSEEK_UNINST" = "xyes"; then
  AC_MSG_ERROR(--with-omseek-uninst needs path of top dir of omseek)
fi
if test "x$OMSEEK_UNINST" = "x"; then
  OMSEEK_UNINST=no
fi

if test "x$OMSEEK_UNINST_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-omseek-uninst-exec needs path of top dir of omseek build)
fi
if test "x$OMSEEK_UNINST_EXEC" = "x"; then
  OMSEEK_UNINST_EXEC=no
fi


dnl If paths aren't absolute, complain
case x$OMSEEK_UNINST in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for omseek-uninst must be absolute (was $OMSEEK_UNINST)]) ;;
esac
case x$OMSEEK_UNINST_EXEC in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for omseek-uninst-exec must be absolute (was $OMSEEK_UNINST_EXEC)]) ;;
esac

dnl OMSEEK_UNINST and OMSEEK_UNINST_EXEC are either an absolute path or no

if test "x$OMSEEK_UNINST" = "xno"; then
  if test "x$OMSEEK_UNINST_EXEC" != "xno"; then
    AC_MSG_ERROR(must specify --with-omseek-uninst if using --with-omseek-uninst-exec)
  fi
  if test "x$OMSEEK_CONFIG" = "xno"; then
    AC_PATH_PROG(OMSEEK_CONFIG, omseek-config, no)
  fi

  AC_MSG_CHECKING(for omseek)
  if test "x$OMSEEK_CONFIG" = "xno"; then
    AC_MSG_RESULT(not found)
    ifelse([$2], , :, [$2])
  else
    if $OMSEEK_CONFIG --check; then
      AC_MSG_RESULT(yes)
      OMSEEK_CFLAGS=`$OMSEEK_CONFIG --cflags`
      OMSEEK_LIBS=`$OMSEEK_CONFIG --libs`
      ifelse([$1], , :, [$1])
    else
      AC_MSG_RESULT(bad installation)
    fi
  fi
else
  AC_MSG_CHECKING(for omseek)

  AC_MSG_RESULT(using uninstalled version)
  if test "x$OMSEEK_UNINST_EXEC" = "xno"; then
    OMSEEK_UNINST_EXEC=$OMSEEK_UNINST
  fi
  OMSEEK_CONFIG_TMP="$OMSEEK_UNINST_EXEC/omseek-config"
  if test -x "$OMSEEK_CONFIG_TMP"; then
    : ;
  else
    cp $OMSEEK_CONFIG_TMP .omseek-config_tmp
    OMSEEK_CONFIG_TMP=./.omseek-config_tmp
    chmod +x .omseek-config_tmp
  fi
  OMSEEK_CFLAGS=`$OMSEEK_CONFIG_TMP --uninst --prefix=$OMSEEK_UNINST --exec-prefix=$OMSEEK_UNINST_EXEC --cflags`
  OMSEEK_LIBS=`$OMSEEK_CONFIG_TMP --uninst --prefix=$OMSEEK_UNINST --exec-prefix=$OMSEEK_UNINST_EXEC --libs`
  if test -x ".omseek-config_tmp"; then
    rm -f .omseek-config_tmp;
  fi
fi

AC_SUBST(OMSEEK_CFLAGS)
AC_SUBST(OMSEEK_LIBS)
])
