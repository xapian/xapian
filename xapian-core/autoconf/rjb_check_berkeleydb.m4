dnl RJB_CHECK_BERKELEYDB
dnl Check for a working BerkeleyDB
dnl This macro currently just checks whether some parameters work.  Needs
dnl much improvement.

AC_DEFUN(RJB_CHECK_BERKELEYDB,
[dnl
dnl Check whether the parameters SLEEPYCAT_INCLUDES, SLEEPYCAT_LINKFLAGS,
dnl and SLEEPYCAT_LIBS represent a working BerkeleyDB
dnl

AC_LANG_SAVE
AC_LANG_CPLUSPLUS
CXXFLAGS_sleepysave=$CXXFLAGS
LIBS_sleepysave=$LIBS
CXXFLAGS="$CXXFLAGS $SLEEPYCAT_INCLUDES $SLEEPYCAT_LINKFLAGS"
LIBS="$LIBS $SLEEPYCAT_LIBS"
AC_TRY_COMPILE([#include <db_cxx.h>],
	       [int major, minor, patch;
               DbEnv::version(&major, &minor, &patch);
               return !($1);
               ],
               [have_sleepy_cxx=yes],
               [have_sleepy_cxx=no],
               [have_sleepy_cxx=no])
CXXFLAGS=$CXXFLAGS_sleepysave
LIBS=$LIBS_sleepysave
AC_LANG_RESTORE

])
