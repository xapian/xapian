#!/bin/sh

# Run ./configure with a --prefix argument of `pwd`/tmp_install (and any other
# arguments supplied on the command line).
#
# Buildbot commands aren't passed through the shell, and I couldn't think of
# any other way to supply a --prefix relative to the build directory.

./configure --prefix=`pwd`/tmp_install "$@"
