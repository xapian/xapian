#! /bin/sh
# bootstrap -- Helps bootstrapping libtool, when checked out from CVS.
#
#   Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc,
#   Mritten by Gary V. Vaughan, 2003
#
#   This file is part of GNU Libtool.
#
# GNU Libtool is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# GNU Libtool is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Libtool; see the file COPYING.  If not, a copy
# can be downloaded from  http://www.gnu.org/licenses/gpl.html,
# or obtained by writing to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
####

# It is okay for the bootstrap process to require unreleased autoconf
# or automake, as long as any released libtool will work with at least
# the newest stable versions of each.  Generally, newer versions offer
# better features, and configure.ac documents oldest version of each
# required for bootstrap (AC_PREREQ, and AM_INIT_AUTOMAKE).

SHELL=${CONFIG_SHELL-/bin/sh}
export SHELL
: ${AUTORECONF=autoreconf}
: ${AUTOCONF=autoconf}
: ${AUTOMAKE=automake}
: ${AUTOM4TE=autom4te}
: ${MAKE=make}
: ${GREP=grep}
: ${EGREP=egrep}
: ${FGREP=fgrep}
: ${SED=sed}
: ${LN_S='ln -s'}
: ${MAKEINFO=makeinfo}
: ${WORKING_LIBOBJ_SUPPORT=false}

case $1 in
--help|-h*)
  cat <<EOF
`echo $0 | sed 's,^.*/,,g'`: This script is designed to bootstrap a fresh CVS checkout
of Libtool.  Useful environment variable settings:
  reconfdirs='. libltdl'     Do not bootstrap the old test suite.
  WORKING_LIBOBJ_SUPPORT=:   Declare that you have fixed LIBOBJDIR support
                             in autoconf (> 2.59) and automake (> 1.9.6).
EOF
  exit
  ;;
esac

test -f ./configure.ac || {
  echo "bootstrap: can't find ./configure.ac, please rerun from top_srcdir"
  exit 1
}


# Extract auxdir and m4dir from configure.ac:
lt_tab='	'
my_sed_traces='s,#.*$,,; s,^dnl .*$,,; s, dnl .*$,,;
	/AC_CONFIG_AUX_DIR[^_]/  {
	    s,^.*AC_CONFIG_AUX_DIR([[ '"$lt_tab"']*\([^])]*\).*$,auxdir=\1,; p;
	};
	/AC_CONFIG_MACRO_DIR/   {
	    s,^.*AC_CONFIG_MACRO_DIR([[ '"$lt_tab"']*\([^])]*\).*$,m4dir=\1,; p;
	};
	d;'
eval `cat configure.ac 2>/dev/null | $SED "$my_sed_traces"`


# Upgrade caveat:
cat <<'EOF'
WARNING: If bootstrapping with this script fails, it may be due to an
WARNING: incompatible installed `libtool.m4' being pulled in to
WARNING: `aclocal.m4'.  The best way to work around such a problem is to
WARNING: uninstall your system libtool files, or failing that, overwrite
WARNING: them with all m4 file as shipped with this distribution (except
WARNING: `lt~obsolete.m4').  After that, retry this bootstrap.
EOF

find . -depth \( -name autom4te.cache -o -name libtool \) -print \
  | grep -v '{arch}' \
  | xargs rm -rf

# Delete stale files from previous libtool versions.
rm -f acinclude.m4 libltdl/config.h

# Workaround for missing LIBOBJDIR support in Autoconf 2.59, Automake 1.9.6:
# Have symlinks of the libobj files in top source dir.
# Set WORKING_LIBOBJ_SUPPORT=: when calling bootstrap if you have fixed tools.
case `($AUTOCONF --version) 2>/dev/null` in
  *\ 2.59[c-z]* | *\ 2.[6-9][0-9]* | *\ [3-9].[0-9]*)
  case `($AUTOMAKE --version) 2>/dev/null` in
    *\ 1.9[a-z]* | *\ 1.1[0-9]* | *\ 1.[2-9][0-9]* | *\ [2-9].[0-9]*)
      WORKING_LIBOBJ_SUPPORT=: ;;
  esac ;;
esac
for file in argz.c lt__dirent.c lt__strl.c; do
  rm -f $file
  $WORKING_LIBOBJ_SUPPORT || $LN_S libltdl/$file $file
done

if test -z "$reconfdirs"; then
  reconfdirs=". libltdl `ls -1d tests/*demo tests/*demo[0-9]`"
fi

# Extract the package name and version number from configure.ac:
set dummy `$SED -n '
    /AC_INIT/{
	s/[][,()]/ /g
	p
    }' configure.ac`
shift

# Whip up a dirty Makefile:
makes='Makefile.am libltdl/Makefile.inc'
rm -f Makefile
$SED '/^if /,/^endif$/d;/^else$/,/^endif$/d;/^include /d' $makes > Makefile

# Building distributed files from configure is bad for automake, so we
# generate them here, and have Makefile rules to keep them up to date.
# We don't have all the substitution values to build ltmain.sh from this
# script yet, but we need config/ltmain.sh for the libtool commands in
# configure, and ltversion.m4 to generate configure in the first place:
rm -f $auxdir/ltmain.sh $m4dir/ltversion.m4

$MAKE ./$auxdir/ltmain.sh ./$m4dir/ltversion.m4 ./doc/notes.txt \
    ./libtoolize.in ./tests/defs.in ./tests/package.m4 \
    ./tests/testsuite ./libltdl/Makefile.am \
    srcdir=. top_srcdir=. PACKAGE="$2" VERSION="$3" \
    PACKAGE_BUGREPORT="bug-$2@gnu.org" M4SH="$AUTOM4TE --language=m4sh" \
    AUTOTEST="$AUTOM4TE --language=autotest" SED="$SED" MAKEINFO="$MAKEINFO" \
    GREP="$GREP" FGREP="$FGREP" EGREP="$EGREP" LN_S="$LN_S"

test -f clcommit.m4sh && $MAKE -f Makefile.maint ./commit \
    srcdir=. top_srcdir=. PACKAGE="$2" VERSION="$3" M4SH="$AUTOM4TE -l m4sh" \
    SED="$SED" GREP="$GREP" FGREP="$FGREP" EGREP="$EGREP" LN_S="$LN_S"

rm -f Makefile

# Make a dummy libtoolize script for autoreconf:
cat > $auxdir/libtoolize <<'EOF'
#! /bin/sh
# This is a dummy file for bootstrapping CVS libtool.
echo "$0: Bootstrap detected, no files installed." | sed 's,^.*/,,g'
exit 0
EOF
chmod 755 $auxdir/libtoolize

# Running the installed `libtoolize' will trash the local (newer) libtool.m4
# among others.  Call the dummy script we made earlier.
LIBTOOLIZE=`pwd`/$auxdir/libtoolize
export LIBTOOLIZE

for sub in $reconfdirs; do
  $AUTORECONF --force --verbose --install $sub
done

# Autoheader valiantly tries to prevent needless reconfigurations by
# not changing the timestamp of config-h.in unless the file contents
# are updated.  Unfortunately config-h.in depends on aclocal.m4 which
# *is* updated, so running 'libtoolize --ltdl=. && configure && make'
# causes autoheader to be called... undesireable for users that do not
# have it!  Fudge the timestamp to prevent that:
sleep 2 && touch libltdl/config-h.in

# Remove our dummy libtoolize
rm -f $auxdir/libtoolize

# These files can cause an infinite configure loop if left behind.
rm -f Makefile libltdl/Makefile libtool vcl.tmp

# This file is misgenerated earlier in bootstrap to satisfy automake 1.9.1
# and earlier, but has a new enough timestamp to not be updated.  Force it
# to be regenerated at make-time with proper substitutions in place:
touch $auxdir/ltmain.m4sh

# Commit script caveat:
cat <<EOF
WARNING: You might want to regenerate \`commit' and \`$auxdir/mailnotify'
WARNING: after you have run \`configure' to discover the real whereabouts
WARNING: of \`sed', \`grep' etc. like this:
WARNING:
WARNING:      rm -f commit $auxdir/mailnotify
WARNING:      make -f Makefile.maint ./commit ./$auxdir/mailnotify
EOF

exit 0
