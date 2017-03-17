# Get XAPIAN_CXXFLAGS, XAPIAN_LIBS, and XAPIAN_VERSION from xapian-config and
# AC_SUBST() them.

# serial 17

# AC_PROVIDE_IFELSE(MACRO-NAME, IF-PROVIDED, IF-NOT-PROVIDED)
# -----------------------------------------------------------
# If this macro is not defined by Autoconf, define it here.
m4_ifdef([AC_PROVIDE_IFELSE],
	 [],
	 [m4_define([AC_PROVIDE_IFELSE],
		 [m4_ifdef([AC_PROVIDE_$1],
			   [$2], [$3])])])

# XO_LIB_XAPIAN([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND[ ,XAPIAN-CONFIG]]])
# --------------------------------------------------------
# AC_SUBST-s XAPIAN_CXXFLAGS, XAPIAN_LIBS, and XAPIAN_VERSION for use in
# Makefile.am
#
# If ACTION-IF-FOUND and ACTION-IF-NOT-FOUND are both unset, then an
# appropriate AC_MSG_ERROR is used as a default ACTION-IF-NOT-FOUND.
# This allows XO_LIB_XAPIAN to be used without any arguments in the
# common case where Xapian is a requirement (rather than optional).
#
# XAPIAN-CONFIG provides the default name for the xapian-config script
# (which the user can override with "./configure XAPIAN_CONFIG=/path/to/it").
# If unset, the default is xapian-config.  Support for this third parameter
# was added in Xapian 1.1.3.
AC_DEFUN([XO_LIB_XAPIAN],
[
  AC_ARG_VAR(XAPIAN_CONFIG, [Location of xapian-config (default:] ifelse([$3], [], xapian-config, [$3]) [on PATH)])
  dnl AC_PATH_PROG ignores an existing user setting of XAPIAN_CONFIG unless it
  dnl has a full path, so add special handling for such cases.
  xapian_config_to_check_for="ifelse([$3], [], xapian-config, [$3])"
  case $XAPIAN_CONFIG in
  "") ;;
  */configure)
    AC_MSG_ERROR([XAPIAN_CONFIG should point to a xapian-config script, not a configure script.])
    ;;
  [[\\/]* | ?:[\\/]*)]
    # XAPIAN_CONFIG has an absolute path, so AC_PATH_PROG can handle it.
    ;;
  [*[\\/]?*)]
    # Convert a relative path to an absolute one.
    XAPIAN_CONFIG=`pwd`/$XAPIAN_CONFIG
    ;;
  *)
    # If there's no path on XAPIAN_CONFIG, use it as the name of the tool to
    # search PATH for, so that things like this work:
    #   ./configure XAPIAN_CONFIG=xapian-config-1.3
    xapian_config_to_check_for=$XAPIAN_CONFIG
    XAPIAN_CONFIG=
    ;;
  esac
  AC_PATH_PROG(XAPIAN_CONFIG, "$xapian_config_to_check_for")
  if test -z "$XAPIAN_CONFIG"; then
    ifelse([$2], ,
      [ifelse([$1], , [
	dnl Simple check to see if the problem is likely to
	dnl be that we're using a "packaged" xapian-core but
	dnl only have the runtime package installed.
	for sfx in '' 32 64 ; do
	  set /usr/lib$sfx/libxapian.so.*
	  if test "/usr/lib$sfx/libxapian.so.*" != "$1" ; then
	    if test -r /etc/debian_version ; then
	      pkg="libxapian-dev"
	    else
	      pkg="xapian-core-devel"
	    fi
	    AC_MSG_ERROR([Can't find xapian-config, although the xapian-core runtime library seems to be installed.  If you've installed xapian-core from a package, you probably need to install an extra package called something like $pkg in order to be able to build code using the Xapian library.])
	  fi
	done
	AC_MSG_ERROR([Can't find xapian-config.  If you have installed the Xapian library, you need to add XAPIAN_CONFIG=/path/to/xapian-config to your configure command.])],
	:)],
      [$2])
  else
    AC_MSG_CHECKING([$XAPIAN_CONFIG works])
    dnl check for --ltlibs but not --libs as "xapian-config --libs" will
    dnl fail if xapian isn't installed...

    dnl run with exec to avoid leaking output on "real" bourne shells
    if (exec >&5 2>&5 ; $XAPIAN_CONFIG --ltlibs --cxxflags; exit $?) then
      AC_MSG_RESULT(yes)
    else
      case $? in
      127)
	AC_MSG_ERROR(['$XAPIAN_CONFIG' not found, aborting])
	;;
      126)
	if test -d "$XAPIAN_CONFIG" ; then
	  AC_MSG_ERROR(['$XAPIAN_CONFIG' is a directory; it should be the filename of the xapian-config script])
	fi
	AC_MSG_ERROR(['$XAPIAN_CONFIG' not executable, aborting])
	;;
      esac
      AC_MSG_ERROR(['$XAPIAN_CONFIG --ltlibs --cxxflags' doesn't work, aborting])
    fi

dnl If LT_INIT, AC_PROG_LIBTOOL or the deprecated older version
dnl AM_PROG_LIBTOOL has already been expanded, enable libtool support now.
dnl Otherwise add hooks to the end of LT_INIT, AC_PROG_LIBTOOL and
dnl AM_PROG_LIBTOOL to enable it if one of these is expanded later.
    XAPIAN_VERSION=`$XAPIAN_CONFIG --version|sed 's/.* //;s/_.*$//'`
    XAPIAN_CXXFLAGS=`$XAPIAN_CONFIG --cxxflags`
    AC_PROVIDE_IFELSE([LT_INIT],
      [XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`],
      [AC_PROVIDE_IFELSE([AC_PROG_LIBTOOL],
	[XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`],
	[AC_PROVIDE_IFELSE([AM_PROG_LIBTOOL],
	  [XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`],
	  dnl Pass magic option so xapian-config knows we called it (so it
	  dnl can choose a more appropriate error message if asked to link
	  dnl with an uninstalled libxapian).  Also pass ac_top_srcdir
	  dnl so the error message can correctly say "configure.ac" or
	  dnl "configure.in" according to which is in use.
	  [XAPIAN_LIBS=`ac_top_srcdir="$ac_top_srcdir" $XAPIAN_CONFIG --from-xo-lib-xapian --libs`
	  m4_ifdef([LT_INIT],
	    [m4_define([LT_INIT], m4_defn([LT_INIT])
		   [XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`])])
	  m4_ifdef([AC_PROG_LIBTOOL],
	    [m4_define([AC_PROG_LIBTOOL], m4_defn([AC_PROG_LIBTOOL])
		 [XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`])])
	  m4_ifdef([AM_PROG_LIBTOOL],
	    [m4_define([AM_PROG_LIBTOOL], m4_defn([AM_PROG_LIBTOOL])
		 [XAPIAN_LIBS=`$XAPIAN_CONFIG --ltlibs`])])])])])
    ifelse([$1], , :, [$1])
  fi
  AC_SUBST(XAPIAN_CXXFLAGS)
  AC_SUBST(XAPIAN_LIBS)
  AC_SUBST(XAPIAN_VERSION)
  m4_define([XO_LIB_XAPIAN_EXPANDED_], [])
])

# XO_REQUIRE(VERSION[, ACTION-IF-LESS-THAN[, ACTION-IF-GREATHER-THAN-OR-EQUAL]])
# --------------------------------------------------------
# Check if $XAPIAN_VERSION is at least VERSION.  This macro should
# be used after XO_LIB_XAPIAN.
#
# If ACTION-IF-LESS-THAN is unset, it defaults to an
# appropriate AC_MSG_ERROR saying that Xapian >= VERSION is needed.
#
# If ACTION-IF-GREATHER-THAN-OR-EQUAL is unset, the default is no
# addtional action.
AC_DEFUN([XO_REQUIRE],
[
  m4_ifndef([XO_LIB_XAPIAN_EXPANDED_],
      [m4_fatal([XO_REQUIRE can only be used after XO_LIB_XAPIAN])])
dnl [Version component '$v' is not a number]
  AC_MSG_CHECKING([if $XAPIAN_CONFIG version >= $1])
  old_IFS=$IFS
  IFS=.
  set x `echo "$XAPIAN_VERSION"|sed 's/_.*//'`
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
    AC_MSG_RESULT([no ($XAPIAN_VERSION)])
    m4_default([$2], [AC_ERROR([XAPIAN_VERSION is $XAPIAN_VERSION, but >= $1 required])])
  else
    AC_MSG_RESULT([yes ($XAPIAN_VERSION)])
    $3
  fi
])
