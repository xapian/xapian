if MAINTAINER_MODE
# Export these so that we run the locally installed autotools when building
# from a bootstrapped SVN tree.
export ACLOCAL AUTOCONF AUTOHEADER AUTOM4TE AUTOMAKE
endif

# parallel runs of javac can have race conditions where one invocation
# is compiling a dependency .java -> .class while another is trying to
# open the .class to figure out if it has to recompile it; a partially-built
# .class may cause this to fail where a serial build process will not
#
# .NOTPARALLEL is a GNU make special target which disables parallel build
# for this make invocation; since we use recursive make, this should solve
# the problem
.NOTPARALLEL:

EXTRA_DIST = SimpleIndex.java SimpleSearch.java
CLEANFILES = SimpleIndex.class SimpleSearch.class
