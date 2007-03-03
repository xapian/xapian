/* backendmanager.cc: manage backends for testsuite
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#ifdef HAVE_VALGRIND
# include <valgrind/memcheck.h>
#endif

#include "safeerrno.h"

#include <fstream>
#include <string>
#include <vector>

#include <stdio.h>

#include <sys/types.h>
#include "safesysstat.h"

#ifdef HAVE_FORK
# include <signal.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/wait.h>
# include <unistd.h>
// Some older systems had SIGCLD rather than SIGCHLD.
# if !defined SIGCHLD && defined SIGCLD
#  define SIGCHLD SIGCLD
# endif
#endif

#include <xapian.h>
#include "index_utils.h"
#include "backendmanager.h"
#include "omdebug.h"
#include "utils.h"

using namespace std;

void
BackendManager::index_files_to_database(Xapian::WritableDatabase & database,
					const vector<string> & dbnames)
{
    vector<string>::const_iterator p;
    for (p = dbnames.begin(); p != dbnames.end(); ++p) {
	if (p->empty()) continue;
	string filename;
	if (datadir.empty()) {
	    filename = *p;
	} else {
	    filename = datadir;
	    filename += '/';
	    filename += *p;
	    filename += ".txt";
	}

	ifstream from(filename.c_str());
	if (!from)
	    throw Xapian::DatabaseOpeningError("Cannot open file " + filename +
		    " for indexing");

	while (from) {
	    Xapian::Document doc(document_from_stream(from));
	    if (doc.termlist_count() == 0)
		break;
	    database.add_document(doc);
	}
    }
}

BackendManager::BackendManager() :
    do_getdb(&BackendManager::getdb_void),
    do_getwritedb(&BackendManager::getwritedb_void)
{
}

void
BackendManager::set_dbtype(const string &type)
{
    if (type == current_type) {
	// leave it as it is.
    } else if (type == "inmemory") {
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	do_getdb = &BackendManager::getdb_inmemory;
	do_getwritedb = &BackendManager::getwritedb_inmemory;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#if 0
#ifdef XAPIAN_HAS_INMEMORY_BACKEND
    } else if (type == "inmemoryerr") {
	do_getdb = &BackendManager::getdb_inmemoryerr;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr;
    } else if (type == "inmemoryerr2") {
	do_getdb = &BackendManager::getdb_inmemoryerr2;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr2;
    } else if (type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_inmemoryerr3;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr3;
#else
    } else if (type == "inmemoryerr" || type == "inmemoryerr2" ||
	       type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#endif
    } else if (type == "flint") {
#ifdef XAPIAN_HAS_FLINT_BACKEND
	do_getdb = &BackendManager::getdb_flint;
	do_getwritedb = &BackendManager::getwritedb_flint;
	rmdir(".flint");
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "quartz") {
#ifdef XAPIAN_HAS_QUARTZ_BACKEND
	do_getdb = &BackendManager::getdb_quartz;
	do_getwritedb = &BackendManager::getwritedb_quartz;
	rmdir(".quartz");
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "remote") {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	do_getdb = &BackendManager::getdb_remote;
	do_getwritedb = &BackendManager::getwritedb_remote;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "remotetcp") {
#ifdef XAPIAN_HAS_REMOTE_BACKEND
	do_getdb = &BackendManager::getdb_remotetcp;
	do_getwritedb = &BackendManager::getwritedb_remotetcp;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "void") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
    } else {
	throw Xapian::InvalidArgumentError(
	    "Expected inmemory, flint, quartz, remote, remotetcp, or void");
    }
    current_type = type;
}

Xapian::Database
BackendManager::getdb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
Xapian::Database
BackendManager::getdb_inmemory(const vector<string> &dbnames)
{
    return getwritedb_inmemory(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemory(const vector<string> &dbnames)
{
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, dbnames);
    return db;
}

#if 0
Xapian::Database
BackendManager::getdb_inmemoryerr(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_errornext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, dbnames);

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr2(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr2(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr2(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_abortnext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, dbnames);

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr3(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr3(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr3(const vector<string> &dbnames)
{
    // params.set("inmemory_abortnext", 2);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, dbnames);

    return db;
}
#endif
#endif

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const string &dirname)
{
    // create a directory if not present
    struct stat sbuf;
    int result = stat(dirname, &sbuf);
    if (result < 0) {
	if (errno != ENOENT)
	    throw Xapian::DatabaseOpeningError("Can't stat directory");
	if (mkdir(dirname, 0700) < 0)
	    throw Xapian::DatabaseOpeningError("Can't create directory");
	return true; // Successfully created a directory.
    }
    if (!S_ISDIR(sbuf.st_mode))
	throw Xapian::DatabaseOpeningError("Is not a directory.");
    return false; // Already a directory.
}

#ifdef XAPIAN_HAS_FLINT_BACKEND
string
BackendManager::createdb_flint(const vector<string> &dbnames)
{
    string parent_dir = ".flint";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); i++) {
	dbdir += "=" + *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
	// Directory was created, so do the indexing.
	Xapian::WritableDatabase db(Xapian::Flint::open(dbdir, Xapian::DB_CREATE, 2048));
	index_files_to_database(db, dbnames);
    }
    return dbdir;
}

Xapian::Database
BackendManager::getdb_flint(const vector<string> &dbnames)
{
    return Xapian::Flint::open(createdb_flint(dbnames));
}

Xapian::WritableDatabase
BackendManager::getwritedb_flint(const vector<string> &dbnames)
{
    string parent_dir = ".flint";
    create_dir_if_needed(parent_dir);

    // Add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused).
    string dbdir = parent_dir + "/dbw";
    // For a writable database we need to start afresh each time.
    rmdir(dbdir);
    (void)create_dir_if_needed(dbdir);
    touch(dbdir + "/log");
    // directory was created, so do the indexing.
    Xapian::WritableDatabase db(Xapian::Flint::open(dbdir, Xapian::DB_CREATE, 2048));
    index_files_to_database(db, dbnames);
    return db;
}
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
string
BackendManager::createdb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); i++) {
	dbdir += "=" + *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
	// Directory was created, so do the indexing.
	Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE, 2048));
	index_files_to_database(db, dbnames);
    }
    return dbdir;
}

Xapian::Database
BackendManager::getdb_quartz(const vector<string> &dbnames)
{
    return Xapian::Quartz::open(createdb_quartz(dbnames));
}

Xapian::WritableDatabase
BackendManager::getwritedb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    // Add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused).
    string dbdir = parent_dir + "/dbw";
    // For a writable database we need to start afresh each time.
    rmdir(dbdir);
    (void)create_dir_if_needed(dbdir);
    touch(dbdir + "/log");
    // directory was created, so do the indexing.
    Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE, 2048));
    index_files_to_database(db, dbnames);
    return db;
}
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
Xapian::Database
BackendManager::getdb_remote(const vector<string> &dbnames)
{
    // Uses xapian-progsrv as the server.

    vector<string> paths;
    string args = "-t";
    if (!dbnames.empty() && dbnames[0] == "#TIMEOUT#") {
	if (dbnames.size() < 2) {
	    throw Xapian::InvalidArgumentError("Missing timeout parameter");
	}
	args += dbnames[1];
	paths.assign(dbnames.begin() + 2, dbnames.end());
    } else {
	// Default to a long (5 minute) timeout so that tests won't fail just
	// because the host is slow or busy.
	args += "300000";
	paths = dbnames;
    }

    args += ' ';
#ifdef XAPIAN_HAS_FLINT_BACKEND
    args += createdb_flint(paths);
#else
    args += createdb_quartz(paths);
#endif
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, "../bin/xapian-progsrv ");
	return Xapian::Remote::open("./runtest", args);
    }
#endif
    return Xapian::Remote::open("../bin/xapian-progsrv", args);
}

Xapian::WritableDatabase
BackendManager::getwritedb_remote(const vector<string> &dbnames)
{
    // Uses xapian-progsrv as the server.

    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host if slow or busy.
    string args = "-t300000 --writable ";

#ifdef XAPIAN_HAS_FLINT_BACKEND
    (void)getwritedb_flint(dbnames);
    args += ".flint/dbw";
#else
    (void)getwritedb_quartz(dbnames);
    args += ".quartz/dbw";
#endif
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) {
	args.insert(0, "../bin/xapian-progsrv ");
	return Xapian::Remote::open_writable("./runtest", args);
    }
#endif
    return Xapian::Remote::open_writable("../bin/xapian-progsrv", args);
}

#ifdef HAVE_FORK
extern "C" void
on_SIGCHLD(int /*sig*/)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

static int
launch_xapian_tcpsrv(const string & args)
{
    int port = 1239;
    // We want to be able to get the exit status of the child process we fork
    // in xapian-tcpsrv doesn't start listening successfully.
    signal(SIGCHLD, SIG_DFL);
try_next_port:
    string cmd = "../bin/xapian-tcpsrv --one-shot --port " + om_tostring(port) + " " + args;
#ifdef HAVE_VALGRIND
    if (RUNNING_ON_VALGRIND) cmd = "./runtest " + cmd;
#endif
    int fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, PF_UNSPEC, fds) < 0) {
	string msg("Couldn't create socketpair: ");
	msg += strerror(errno);
	throw msg;
    }

    pid_t child = fork();
    if (child == 0) {
	// Child process.
	close(fds[0]);
	// Connect stdout and stderr to the socket.
	dup2(fds[1], 1);
	dup2(fds[1], 2);
	execl("/bin/sh", "/bin/sh", "-c", cmd.c_str(), (void*)NULL);
	_exit(-1);
    }

    close(fds[1]);
    if (child == -1) {
	// Couldn't fork.
	int fork_errno = errno;
	close(fds[0]);
	string msg("Couldn't fork: ");
	msg += strerror(fork_errno);
	throw msg;
    }
   
    // Parent process.

    // Wrap the file descriptor in a FILE * so we can read lines using fgets().
    FILE * fh = fdopen(fds[0], "r");
    if (fh == NULL) {
	string msg("Failed to run command '");
	msg += cmd;
	msg += "': ";
	msg += strerror(errno);
	throw msg;
    }
    string output;
    while (true) {
	char buf[256];
	if (fgets(buf, sizeof(buf), fh) == NULL) {
	    fclose(fh);
	    int status;
	    if (waitpid(child, &status, 0) == -1) {
		string msg("waitpid failed: ");
		msg += strerror(errno);
		throw msg;
	    }
	    if (++port < 65536 && status != 0) {
		if (WIFEXITED(status) && WEXITSTATUS(status) == 69) {  
		    // 69 is EX_UNAVAILABLE which xapian-tcpsrv exits
		    // with if (and only if) the port specified was
		    // in use.
		    goto try_next_port;
		}
	    }
	    string msg("Failed to get 'Listening...' from command '");
	    msg += cmd;
	    msg += "' (output: ";
	    msg += output;
	    msg += ")";
	    throw msg;
	}
	if (strcmp(buf, "Listening...\n") == 0) break;
	output += buf;
    }

    // Set a signal handler to clean up the xapian-tcpsrv child process when it
    // finally exits.
    signal(SIGCHLD, on_SIGCHLD);

    return port;
}
#endif

Xapian::Database
BackendManager::getdb_remotetcp(const vector<string> &dbnames)
{
#ifndef HAVE_FORK
    throw string("Can't run remotetcp tests without fork()");
#else
    // Uses xapian-tcpsrv as the server.

    vector<string> paths;
    string args = "-t";
    if (!dbnames.empty() && dbnames[0] == "#TIMEOUT#") {
	if (dbnames.size() < 2) {
	    throw Xapian::InvalidArgumentError("Missing timeout parameter");
	}
	args += dbnames[1];
	paths.assign(dbnames.begin() + 2, dbnames.end());
    } else {
	// Default to a long (5 minute) timeout so that tests won't fail just
	// because the host if slow or busy.
	args += "300000";
	paths = dbnames;
    }

    args += ' ';
#ifdef XAPIAN_HAS_FLINT_BACKEND
    args += createdb_flint(paths);
#else
    args += createdb_quartz(paths);
#endif

    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open("127.0.0.1", port);
#endif
}

Xapian::WritableDatabase
BackendManager::getwritedb_remotetcp(const vector<string> &dbnames)
{
#ifndef HAVE_FORK
    throw string("Can't run remotetcp tests without fork()");
#else
    // Uses xapian-tcpsrv as the server.

    // Default to a long (5 minute) timeout so that tests won't fail just
    // because the host if slow or busy.
    string args = "-t300000 --writable ";

#ifdef XAPIAN_HAS_FLINT_BACKEND
    (void)getwritedb_flint(dbnames);
    args += ".flint/dbw";
#else
    (void)getwritedb_quartz(dbnames);
    args += ".quartz/dbw";
#endif

    int port = launch_xapian_tcpsrv(args);
    return Xapian::Remote::open_writable("127.0.0.1", port);
#endif
}
#endif

Xapian::Database
BackendManager::get_database(const vector<string> &dbnames)
{
    return (this->*do_getdb)(dbnames);
}

Xapian::Database
BackendManager::get_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getdb)(dbnames);
}

Xapian::WritableDatabase
BackendManager::get_writable_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getwritedb)(dbnames);
}
