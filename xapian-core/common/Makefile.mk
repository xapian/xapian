noinst_HEADERS +=\
	common/alignment_cast.h\
	common/append_filename_arg.h\
	common/bitstream.h\
	common/closefrom.h\
	common/compression_stream.h\
	common/debuglog.h\
	common/errno_to_string.h\
	common/exp10.h\
	common/fd.h\
	common/filetests.h\
	common/fileutils.h\
	common/gnu_getopt.h\
	common/heap.h\
	common/internaltypes.h\
	common/io_utils.h\
	common/keyword.h\
	common/log2.h\
	common/min_non_zero.h\
	common/msvc_dirent.h\
	common/msvcignoreinvalidparam.h\
	common/omassert.h\
	common/output.h\
	common/overflow.h\
	common/pack.h\
	common/parseint.h\
	common/popcount.h\
	common/posixy_wrapper.h\
	common/pretty.h\
	common/realtime.h\
	common/replicate_utils.h\
	common/replicationprotocol.h\
	common/safedirent.h\
	common/safefcntl.h\
	common/safenetdb.h\
	common/safesysselect.h\
	common/safesyssocket.h\
	common/safesysstat.h\
	common/safesyswait.h\
	common/safeunistd.h\
	common/safewindows.h\
	common/safewinsock2.h\
	common/serialise-double.h\
	common/setenv.h\
	common/socket_utils.h\
	common/stdclamp.h\
	common/str.h\
	common/stringutils.h\
	common/wordaccess.h

EXTRA_DIST +=\
	common/Makefile\
	common/Tokeniseise.pm

lib_src +=\
	common/bitstream.cc\
	common/closefrom.cc\
	common/debuglog.cc\
	common/errno_to_string.cc\
	common/fileutils.cc\
	common/io_utils.cc\
	common/keyword.cc\
	common/msvc_dirent.cc\
	common/omassert.cc\
	common/pack.cc\
	common/posixy_wrapper.cc\
	common/replicate_utils.cc\
	common/safe.cc\
	common/serialise-double.cc\
	common/socket_utils.cc\
	common/str.cc

if BUILD_BACKEND_GLASS
lib_src +=\
	common/compression_stream.cc
else
if BUILD_BACKEND_HONEY
lib_src +=\
	common/compression_stream.cc
endif
endif

noinst_LTLIBRARIES += libgetopt.la

libgetopt_la_SOURCES =\
	common/getopt.cc
