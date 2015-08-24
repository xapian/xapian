noinst_HEADERS +=\
	common/alltermslist.h\
	common/append_filename_arg.h\
	common/autoptr.h\
	common/bitstream.h\
	common/closefrom.h\
	common/const_database_wrapper.h\
	common/contiguousalldocspostlist.h\
	common/database.h\
	common/databasereplicator.h\
	common/debuglog.h\
	common/document.h\
	common/documentterm.h\
	common/emptypostlist.h\
	common/errno_to_string.h\
	common/esetinternal.h\
	common/expandweight.h\
	common/fileutils.h\
	common/gnu_getopt.h\
	common/inmemory_positionlist.h\
	common/internaltypes.h\
	common/io_utils.h\
	common/leafpostlist.h\
	common/msvc_dirent.h\
	common/msvc_posix_wrapper.h\
	common/multialltermslist.h\
	common/multimatch.h\
	common/multivaluelist.h\
	common/noreturn.h\
	common/omassert.h\
	common/omenquireinternal.h\
	common/omqueryinternal.h\
	common/ortermlist.h\
	common/output.h\
	common/positionlist.h\
	common/pack.h\
	common/postlist.h\
	common/pretty.h\
	common/progclient.h\
	common/realtime.h\
	common/registryinternal.h\
	common/remoteconnection.h\
	common/remote-database.h\
	common/remoteprotocol.h\
	common/remoteserver.h\
	common/remotetcpclient.h\
	common/remotetcpserver.h\
	common/replicate_utils.h\
	common/replicatetcpclient.h\
	common/replicatetcpserver.h\
	common/replication.h\
	common/replicationprotocol.h\
	common/safedirent.h\
	common/safeerrno.h\
	common/safefcntl.h\
	common/safenetdb.h\
	common/safesysselect.h\
	common/safesysstat.h\
	common/safesyswait.h\
	common/safeunistd.h\
	common/safeuuid.h\
	common/safewindows.h\
	common/safewinsock2.h\
	common/serialise-double.h\
	common/serialise.h\
	common/socket_utils.h\
	common/str.h\
	common/stringutils.h\
	common/submatch.h\
	common/tcpclient.h\
	common/tcpserver.h\
	common/termlist.h\
	common/unaligned.h\
	common/utils.h\
	common/valuelist.h\
	common/valuestats.h\
	common/vectortermlist.h\
	common/weightinternal.h

EXTRA_DIST +=\
	common/dir_contents\
	common/win32_uuid.cc\
	common/win32_uuid.h\
	common/Makefile

lib_src +=\
	common/bitstream.cc\
	common/closefrom.cc\
	common/const_database_wrapper.cc\
	common/debuglog.cc\
	common/errno_to_string.cc\
	common/fileutils.cc\
	common/io_utils.cc\
	common/msvc_dirent.cc\
	common/msvc_posix_wrapper.cc\
	common/replicate_utils.cc\
	common/safe.cc\
	common/serialise-double.cc\
	common/socket_utils.cc\
	common/str.cc\
	common/stringutils.cc\
	common/utils.cc

if USE_WIN32_UUID_API
lib_src +=\
	common/win32_uuid.cc
libxapian_la_LDFLAGS += -lrpcrt4
endif

noinst_LTLIBRARIES += libgetopt.la

libgetopt_la_SOURCES =\
	common/getopt.cc
