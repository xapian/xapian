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
CXXFLAGS_sleepycatsave=$CXXFLAGS
LIBS_sleepycatsave=$LIBS
CXXFLAGS="$CXXFLAGS $SLEEPYCAT_INCLUDES $SLEEPYCAT_LINKFLAGS"
LIBS="$LIBS $SLEEPYCAT_LIBS"
AC_TRY_RUN([#include <db_cxx.h>
	   int main() {
	   int major, minor, patch;
	   DbEnv::version(&major, &minor, &patch);
	   return !($1);
	   }
	   ],
	   [have_sleepycat_cxx=yes],
	   [have_sleepycat_cxx=no],
	   AC_TRY_COMPILE([#include <db_cxx.h>],
			  [int major, minor, patch;
			  DbEnv::version(&major, &minor, &patch);
			  return !($1);
			  ],
			  [have_sleepycat_cxx=yes],
			  [have_sleepycat_cxx=no],
			  [have_sleepycat_cxx=no]))
CXXFLAGS=$CXXFLAGS_sleepycatsave
LIBS=$LIBS_sleepycatsave
AC_LANG_RESTORE

])
