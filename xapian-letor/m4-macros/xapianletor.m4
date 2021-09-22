# Get XAPIANLETOR_CXXFLAGS, XAPIANLETOR_LIBS, and XAPIANLETOR_VERSION from xapianletor-config and
# AC_SUBST() them.

# serial 18

# AC_PROVIDE_IFELSE(MACRO-NAME, IF-PROVIDED, IF-NOT-PROVIDED)
# -----------------------------------------------------------
# If this macro is not defined by Autoconf, define it here.
m4_ifdef([AC_PROVIDE_IFELSE],
	 [],
	 [m4_define([AC_PROVIDE_IFELSE],
		 [m4_ifdef([AC_PROVIDE_$1],
			   [$2], [$3])])])

# XO_LIB_XAPIANLETOR([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND[ ,XAPIANLETOR-CONFIG]]])
# --------------------------------------------------------
# AC_SUBST-s XAPIANLETOR_CXXFLAGS, XAPIANLETOR_LIBS, and XAPIANLETOR_VERSION for use in
# Makefile.am
#
# If ACTION-IF-FOUND and ACTION-IF-NOT-FOUND are both unset, then an
# appropriate AC_MSG_ERROR is used as a default ACTION-IF-NOT-FOUND.
# This allows XO_LIB_XAPIANLETOR to be used without any arguments in the
# common case where Xapian is a requirement (rather than optional).
#
# XAPIANLETOR-CONFIG provides the default name for the xapianletor-config script
# (which the user can override with "./configure XAPIANLETOR_CONFIG=/path/to/it").
# If unset, the default is xapianletor-config.
AC_DEFUN([XO_LIB_XAPIANLETOR],
[
  AC_ARG_VAR(XAPIANLETOR_CONFIG, [Location of xapianletor-config (default:] ifelse([$3], [], xapianletor-config, [$3]) [on PATH)])
  dnl AC_PATH_PROG ignores an existing user setting of XAPIANLETOR_CONFIG unless it
  dnl has a full path, so add special handling for such cases.
  config_script_to_check_for="ifelse([$3], [], xapianletor-config, [$3])"
  case $XAPIANLETOR_CONFIG in
  "") ;;
  */configure)
    AC_MSG_ERROR([XAPIANLETOR_CONFIG should point to a xapianletor-config script, not a configure script.])
    ;;
  [[\\/]* | ?:[\\/]*)]
    # XAPIANLETOR_CONFIG has an absolute path, so AC_PATH_PROG can handle it.
    ;;
  [*[\\/]?*)]
    # Convert a relative path to an absolute one.
    XAPIANLETOR_CONFIG=`pwd`/$XAPIANLETOR_CONFIG
    ;;
  *)
    # If there's no path on XAPIANLETOR_CONFIG, use it as the name of the tool to
    # search PATH for, so that things like this work:
    #   ./configure XAPIANLETOR_CONFIG=xapianletor-config-1.3
    config_script_to_check_for=$XAPIANLETOR_CONFIG
    XAPIANLETOR_CONFIG=
    ;;
  esac
  AC_PATH_PROG(XAPIANLETOR_CONFIG, "$config_script_to_check_for")
  AC_MSG_CHECKING([$XAPIANLETOR_CONFIG works])
  dnl check for --ltlibs but not --libs as "xapianletor-config --libs" will
  dnl fail if xapian isn't installed...

  dnl run with exec to avoid leaking output on "real" bourne shells
  if (exec >&5 2>&5 ; $XAPIANLETOR_CONFIG --ltlibs --cxxflags; exit $?) then
    AC_MSG_RESULT(yes)
  else
    case $? in
    127)
    AC_MSG_ERROR(['$XAPIANLETOR_CONFIG' not found, aborting])
    ;;
    126)
    if test -d "$XAPIANLETOR_CONFIG" ; then
      AC_MSG_ERROR(['$XAPIANLETOR_CONFIG' is a directory; it should be the filename of the xapianletor-config script])
    fi
    AC_MSG_ERROR(['$XAPIANLETOR_CONFIG' not executable, aborting])
    ;;
    esac
      AC_MSG_ERROR(['$XAPIANLETOR_CONFIG --ltlibs --cxxflags' doesn't work, aborting])
  fi

dnl If LT_INIT, AC_PROG_LIBTOOL or the deprecated older version
dnl AM_PROG_LIBTOOL has already been expanded, enable libtool support now.
dnl Otherwise add hooks to the end of LT_INIT, AC_PROG_LIBTOOL and
dnl AM_PROG_LIBTOOL to enable it if one of these is expanded later.
  XAPIANLETOR_VERSION=`$XAPIANLETOR_CONFIG --version|sed 's/.* //;s/_.*$//'`
  XAPIANLETOR_CXXFLAGS=`$XAPIANLETOR_CONFIG --cxxflags`
  AC_PROVIDE_IFELSE([LT_INIT],
    [XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`],
    [AC_PROVIDE_IFELSE([AC_PROG_LIBTOOL],
      [XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`],
      [AC_PROVIDE_IFELSE([AM_PROG_LIBTOOL],
	[XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`],
	dnl Pass magic option so xapianletor-config knows we called it (so it
	dnl can choose a more appropriate error message if asked to link
	dnl with an uninstalled libxapian).  Also pass ac_top_srcdir
	dnl so the error message can correctly say "configure.ac" or
	dnl "configure.in" according to which is in use.
	[XAPIANLETOR_LIBS=`ac_top_srcdir="$ac_top_srcdir" $XAPIANLETOR_CONFIG --from-xo-lib-xapianletor --libs`
	m4_ifdef([LT_INIT],
	  [m4_define([LT_INIT], m4_defn([LT_INIT])
	    [XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`])])
	m4_ifdef([AC_PROG_LIBTOOL],
	  [m4_define([AC_PROG_LIBTOOL], m4_defn([AC_PROG_LIBTOOL])
	    [XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`])])
	m4_ifdef([AM_PROG_LIBTOOL],
	  [m4_define([AM_PROG_LIBTOOL], m4_defn([AM_PROG_LIBTOOL])
	    [XAPIANLETOR_LIBS=`$XAPIANLETOR_CONFIG --ltlibs`])])])])])
  ifelse([$1], , :, [$1])
  AC_SUBST(XAPIANLETOR_CXXFLAGS)
  AC_SUBST(XAPIANLETOR_LIBS)
  AC_SUBST(XAPIANLETOR_VERSION)
  m4_define([XO_LIB_XAPIANLETOR_EXPANDED_], [])
])

# XO_LETOR_REQUIRE(VERSION[, ACTION-IF-LESS-THAN[, ACTION-IF-GREATHER-THAN-OR-EQUAL]])
# --------------------------------------------------------
# Check if $XAPIANLETOR_VERSION is at least VERSION.  This macro should
# be used after XO_LIB_XAPIANLETOR.
#
# If ACTION-IF-LESS-THAN is unset, it defaults to an
# appropriate AC_MSG_ERROR saying that Xapian >= VERSION is needed.
#
# If ACTION-IF-GREATHER-THAN-OR-EQUAL is unset, the default is no
# addtional action.
AC_DEFUN([XO_LETOR_REQUIRE],
[
  m4_ifndef([XO_LIB_XAPIANLETOR_EXPANDED_],
      [m4_fatal([XO_LETOR_REQUIRE can only be used after XO_LIB_XAPIANLETOR])])
dnl [Version component '$v' is not a number]
  AC_MSG_CHECKING([if $XAPIANLETOR_CONFIG version >= $1])
  old_IFS=$IFS
  IFS=.
  set x `echo "$XAPIANLETOR_VERSION"|sed 's/_.*//'`
  IFS=$old_IFS
  res=
  m4_foreach([min_component], m4_split([$1], [\.]), [
ifelse(regexp(min_component, [^[0-9][0-9]*$]), [-1], [m4_fatal(Component `min_component' not numeric)])dnl
  if test -z "$res" ; then
    shift
    if test "$[]1" -gt 'min_component' ; then
      res=1
    elif test "$[]1" -lt 'min_component' ; then
      res=0
    fi
  fi])
  if test "$res" = 0 ; then
    AC_MSG_RESULT([no ($XAPIANLETOR_VERSION)])
    m4_default([$2], [AC_MSG_ERROR([XAPIANLETOR_VERSION is $XAPIANLETOR_VERSION, but >= $1 required])])
  else
    AC_MSG_RESULT([yes ($XAPIANLETOR_VERSION)])
    $3
  fi
])
