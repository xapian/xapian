/* database.cc: Database factories and base class
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "database.h"
#include "utils.h" // for om_tostring
#include <xapian/dbfactory.h>
#include <xapian/error.h>

#include <fstream>
#include <string>

using namespace std;

// Include headers for all the enabled database backends
#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
#include "muscat36/da_database.h"
#include "muscat36/db_database.h"
#endif
#ifdef XAPIAN_BUILD_BACKEND_INMEMORY
#include "inmemory/inmemory_database.h"
#endif
#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
#include "quartz/quartz_database.h"
#endif
#ifdef XAPIAN_BUILD_BACKEND_REMOTE
// These headers are all in common
#include "net_database.h"
#include "progclient.h"
#include "tcpclient.h"
#endif

namespace Xapian {

#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
Database
Quartz::open(const string &dir) {
    DEBUGAPICALL_STATIC(Database, "Quartz::open", dir);
    return Database(new QuartzDatabase(dir));
}

WritableDatabase
Quartz::open(const string &dir, int action, int block_size) {
    DEBUGAPICALL_STATIC(WritableDatabase, "Quartz::open", dir << ", " <<
			action << ", " << block_size);
    return WritableDatabase(new QuartzWritableDatabase(dir, action,
						       block_size));
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_INMEMORY
// Note: a read-only inmemory database will always be empty, and so there's
// not much use in allowing one to be created.
WritableDatabase
InMemory::open() {
    DEBUGAPICALL_STATIC(Database, "InMemory::open", "");
    return WritableDatabase(new InMemoryDatabase());
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
Database
Muscat36::open_da(const string &R, const string &T, bool heavy_duty) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_da", R << ", " << T << ", " <<
			heavy_duty);
    return Database(new DADatabase(R, T, "", heavy_duty));
}

Database
Muscat36::open_da(const string &R, const string &T, const string &values,
		  bool heavy_duty) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_da", R << ", " << T << ", " <<
			values << ", " << heavy_duty);
    return Database(new DADatabase(R, T, values, heavy_duty));
}

Database
Muscat36::open_db(const string &DB, size_t cache_size) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_db", DB << ", " <<
			cache_size);
    return Database(new DBDatabase(DB, "", cache_size));
}

Database
Muscat36::open_db(const string &DB, const string &values, size_t cache_size) {
    DEBUGAPICALL_STATIC(Database, "Muscat36::open_db", DB << ", " << values <<
			", " << cache_size);
    return Database(new DBDatabase(DB, values, cache_size));
}
#endif

#ifdef XAPIAN_BUILD_BACKEND_REMOTE
Database
Remote::open(const string &program, const string &args, unsigned int timeout)
{
    DEBUGAPICALL_STATIC(Database, "Remote::open", args << ", " << timeout);
    Xapian::Internal::RefCntPtr<NetClient> link(new ProgClient(program, args, timeout));
    return Database(new NetworkDatabase(link));
}

Database
Remote::open(const string &host, unsigned int port,
	unsigned int timeout, unsigned int connect_timeout)
{
    DEBUGAPICALL_STATIC(Database, "Remote::open", host << ", " << port <<
			", " << timeout << ", " << connect_timeout);
    if (connect_timeout == 0) connect_timeout = timeout;
    Xapian::Internal::RefCntPtr<NetClient> link(new TcpClient(host, port, timeout, connect_timeout));
    return Database(new NetworkDatabase(link));
}
#endif

static void
open_stub(Database *db, const string &file)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    ifstream stub(file.c_str());
    string line;
    int line_no = 1;
    bool ok = false;
    while (getline(stub, line)) {
	string::size_type space = line.find(' ');
	if (space != string::npos) {
	    string type = line.substr(0, space);
	    line.erase(0, space + 1);
	    if (type == "auto") {
		db->add_database(Database(line));
		ok = true;
#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
	    } else if (type == "quartz") {
		db->add_database(Quartz::open(line));
		ok = true;
#endif
#ifdef XAPIAN_BUILD_BACKEND_REMOTE
	    } else if (type == "remote") {
		string::size_type colon = line.find(':');
		if (colon == 0) {
		    // prog
		    // FIXME: timeouts
		    // FIXME: Is prog actually useful beyond testing?
		    // Is it a security risk?
		    space = line.find(' ');
		    string args;
		    if (space != string::npos) {
			args = line.substr(space + 1);
			line = line.substr(1, space - 1);
		    } else {
			line.erase(0, 1);
		    }
		    db->add_database(Remote::open(line, args));
		    ok = true;
		} else if (colon != string::npos) {
		    // tcp
		    // FIXME: timeouts
		    unsigned int port = atoi(line.c_str() + colon + 1);
		    line.erase(colon);
		    db->add_database(Remote::open(line, port));
		    ok = true;
		}
#endif
	    }
#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
	    // FIXME: da and db too, but I'm too slack to do those right now!
#endif
	}
	if (!ok) break;
	++line_no;
    }
    if (!ok) {
	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange it to be read as a stub database via infelicities in
	// an application which uses Xapian.  The line number is enough
	// information to identify the problem line.
	throw DatabaseOpeningError("Bad line " + om_tostring(line_no) + " in stub database file `" + file + "'");
    }
}

Database
Auto::open_stub(const string &file)
{
    DEBUGAPICALL_STATIC(Database, "Auto::open_stub", file);
    Database db;
    open_stub(&db, file);
    return db;
}

namespace Internal {

void
open_database(Database * db, const string &path)
{
    // Check for path actually being a file - if so, assume it to be
    // a stub database.
    if (file_exists(path)) {
	open_stub(db, path);
	return;
    }

#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
    if (file_exists(path + "/record_DB")) {
	db->internal.push_back(new QuartzDatabase(path));
	return;
    }
#endif
#ifdef XAPIAN_BUILD_BACKEND_MUSCAT36
    if (file_exists(path + "/R") && file_exists(path + "/T")) {
	// can't easily tell flimsy from heavyduty so assume hd
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	db->internal.push_back(new DADatabase(path + "/R", path + "/T",
					      keyfile, true));
	return;
    }
    string dbfile = path + "/DB";
    if (!file_exists(dbfile)) {
	dbfile += ".da";
	if (!file_exists(dbfile)) dbfile = "";
    }
    if (!dbfile.empty()) {
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	db->internal.push_back(new DBDatabase(dbfile, keyfile));
	return;
    }
#endif

    throw FeatureUnavailableError("Couldn't detect type of database");
}

void
open_writable_database(Database *db, const string &path, int action)
{
#ifdef XAPIAN_BUILD_BACKEND_QUARTZ
    // Only quartz currently supports disk-based writable databases - if other
    // writable backends are added then this code needs to look at action and
    // perhaps autodetect.
    db->internal.push_back(new QuartzWritableDatabase(path, action, 8192));
#else
    throw FeatureUnavailableError("No disk-based writable backend is enabled");
#endif
}

}

///////////////////////////////////////////////////////////////////////////////

Database::Internal::Internal()
	: transaction_in_progress(false)
{
}

Database::Internal::~Internal()
{
}

void
Database::Internal::keep_alive() const
{
    // For the normal case of local databases, nothing needs to be done.
}

// Discard any exceptions - we're called from the destructors of derived
// classes so we can't safely throw.
void
Database::Internal::dtor_called()
{
    if (transaction_in_progress) {
	try {
	    transaction_in_progress = false;
	    // cancel the current transaction
	    throw Xapian::UnimplementedError("transactions aren't implemented");
	} catch (...) {
	}
    }

    try {
	flush();
    } catch (...) {
    }
}

void
Database::Internal::begin_transaction()
{
    if (transaction_in_progress)
	throw InvalidOperationError("Cannot begin transaction - transaction already in progress");
    // begin transaction
    throw Xapian::UnimplementedError("transactions aren't implemented");
    transaction_in_progress = true;
}

void
Database::Internal::commit_transaction()
{
    if (!transaction_in_progress)
	throw InvalidOperationError("Cannot commit transaction - no transaction currently in progress");
    // commit transaction
    throw Xapian::UnimplementedError("transactions aren't implemented");
}

void
Database::Internal::cancel_transaction()
{
    if (!transaction_in_progress)
	throw InvalidOperationError("Cannot cancel transaction - no transaction currently in progress");
    transaction_in_progress = false;
    // cancel transaction
    throw Xapian::UnimplementedError("transactions aren't implemented");
}

Xapian::docid
Database::Internal::get_lastdocid() const
{
    DEBUGCALL(DB, void, "Database::Internal::get_lastdocid", "");
    throw Xapian::UnimplementedError("Database::Internal::get_lastdocid() not yet implemented");
}

}
