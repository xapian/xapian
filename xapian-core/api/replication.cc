/** @file replication.cc
 * @brief Replication support for Xapian databases.
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>
#include "xapian/replication.h"

#include "xapian/base.h"
#include "xapian/dbfactory.h"
#include "xapian/error.h"

#include "database.h"
#ifdef __WIN32__
# include "msvc_posix_wrapper.h"
#endif
#include "omassert.h"
#include "omdebug.h"
#include "safeerrno.h"
#include "utils.h"

#include <fstream>
#include <map>
#include <string>

using namespace std;
using namespace Xapian;

void
DatabaseMaster::write_changesets_to_fd(int fd,
				       const string & start_revision) const
{
    DEBUGAPICALL(void, "Xapian::DatabaseMaster::write_changesets_to_fd",
		 fd << ", " << start_revision);
    Database db(path);
    if (db.internal.size() != 1) {
	throw Xapian::InvalidOperationError("DatabaseMaster needs to be pointed at exactly one subdatabase");
    }
    db.internal[0]->write_changesets_to_fd(fd, start_revision);
}

string
DatabaseMaster::get_description() const
{
    DEBUGCALL(INTRO, string, "DatabaseMaster::get_description", "");
    RETURN("DatabaseMaster(" + path + ")");
}

/// Internal implementation of DatabaseReplica
class DatabaseReplica::Internal : public Xapian::Internal::RefCntBase {
    /// Don't allow assignment.
    void operator=(const Internal &);

    /// Don't allow copying.
    Internal(const Internal &);

    /// The path to the replica (will point to a stub database file).
    string path;

    /// The path to the actual database in the replica.
    string real_path;

    /// The database being replicated.
    WritableDatabase db;

    /// The parameters stored for this replica.
    map<string, string> parameters;

    /// Read the parameters from a file in the replica.
    void read_parameters();

    /// Write the parameters to a file in the replica.
    void write_parameters() const;

    /** Update the stub database which points to a single flint database.
     *
     *  The stub database file is created at a separate path, and then atomically
     *  moved into place to replace the old stub database.  This should allow
     *  searches to continue uninterrupted.
     *
     *  @param flint_path The path to the flint database.
     */
    void update_stub_database(const string & flint_path) const;

  public:
    /// Open a new DatabaseReplica::Internal for the specified path.
    Internal(const string & path_);

    /// Set a parameter for the replica.
    void set_parameter(const string & name, const string & value);

    /// Get a parameter from the replica.
    string get_parameter(const string & name) const;

    /// Get a string describing the current revision of the replica.
    string get_revision_info() const;

    /// Read and apply the next changeset.
    bool apply_next_changeset_from_fd(int fd);

    /// Return a string describing this object.
    string get_description() const { return path; }
};

// Methods of DatabaseReplica

DatabaseReplica::DatabaseReplica(const DatabaseReplica & other)
	: internal(other.internal)
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::DatabaseReplica", other);
}

void
DatabaseReplica::operator=(const DatabaseReplica & other)
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::operator=", other);
    internal = other.internal;
}

DatabaseReplica::DatabaseReplica()
	: internal(0)
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::DatabaseReplica", "");
}

DatabaseReplica::DatabaseReplica(const string & path)
	: internal(new DatabaseReplica::Internal(path))
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::DatabaseReplica", path);
}

DatabaseReplica::~DatabaseReplica()
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::~DatabaseReplica", "");
}

void
DatabaseReplica::set_parameter(const string & name, const string & value)
{
    DEBUGAPICALL(void, "Xapian::DatabaseReplica::set_parameter",
		 name << ", " << value);
    internal->set_parameter(name, value);
}

string
DatabaseReplica::get_parameter(const string & name) const
{
    DEBUGAPICALL(string, "Xapian::DatabaseReplica::get_parameter", name);
    RETURN(internal->get_parameter(name));
}

string
DatabaseReplica::get_revision_info() const
{
    DEBUGAPICALL(string, "Xapian::DatabaseReplica::get_revision_info", "");
    if (internal.get() == NULL)
	throw Xapian::InvalidOperationError("Attempt to call DatabaseReplica::get_revision_info on a closed replica.");
    RETURN(internal->get_revision_info());
}

bool 
DatabaseReplica::apply_next_changeset_from_fd(int fd)
{
    DEBUGAPICALL(bool, "Xapian::DatabaseReplica::apply_next_changeset_from_fd", fd);
    if (internal.get() == NULL)
	throw Xapian::InvalidOperationError("Attempt to call DatabaseReplica::apply_next_changeset_from_fd on a closed replica.");
    RETURN(internal->apply_next_changeset_from_fd(fd));
}

void 
DatabaseReplica::close()
{
    DEBUGAPICALL(bool, "Xapian::DatabaseReplica::close", "");
    internal = NULL;
}

string
DatabaseReplica::get_description() const
{
    DEBUGCALL(INTRO, string, "DatabaseReplica::get_description", "");
    RETURN("DatabaseReplica(" + internal->get_description() + ")");
}

// Methods of DatabaseReplica::Internal

void
DatabaseReplica::Internal::read_parameters()
{
    parameters.clear();

    string param_path = path + "_p";
    if (!file_exists(param_path)) {
	ifstream p_in(param_path.c_str());
	string line;
	while (getline(p_in, line)) {
	    string::size_type eq = line.find('=');
	    if (eq != string::npos) {
		string key = line.substr(0, eq);
		line.erase(0, eq + 1);
		parameters[key] = line;
	    }
	}
    }
}

void
DatabaseReplica::Internal::write_parameters() const
{
    string param_path = path + "_p";
    ofstream p_out(param_path.c_str());

    map<string, string>::const_iterator i;
    for (i = parameters.begin(); i != parameters.end(); ++i)
    {
	p_out << i->first << "=" << i->second << endl;
    }
}

void
DatabaseReplica::Internal::update_stub_database(const string & flint_path) const
{
    string tmp_path = path + ".tmp";
    {
	ofstream stub(tmp_path.c_str());
	stub << "flint " << flint_path;
    }
    int result;
#ifdef __WIN32__
    result = msvc_posix_rename(tmp_path.c_str(), path.c_str());
#else
    result = rename(tmp_path.c_str(), path.c_str());
#endif
    if (result == -1) {
	string msg("Failed to update stub db file for replica: ");
	msg += path;
	throw Xapian::DatabaseOpeningError(msg);
    }
}

DatabaseReplica::Internal::Internal(const string & path_)
	: path(path_), real_path(), db(), parameters()
{
    DEBUGCALL(API, void, "DatabaseReplica::Internal::Internal", path_);
    if (dir_exists(path)) {
	throw InvalidOperationError("Replica path should not be a directory");
    }
#ifndef XAPIAN_HAS_FLINT_BACKEND
    throw FeatureUnavailableError("Flint backend is not enabled, and needed for database replication");
#endif
    if (!file_exists(path)) {
	// The database doesn't already exist - make a stub database, and point
	// it to a new flint database.
	real_path = path + "_0";
	db.add_database(Flint::open(real_path, Xapian::DB_CREATE));
	update_stub_database(real_path);
    } else {
	// The database already exists as a stub database - open it.  We can't
	// just use the standard opening routines, because we want to open it
	// for writing.  We enforce that the stub database points to a single
	// flint database here.
	ifstream stub(path.c_str());
	string line;
	while (getline(stub, line)) {
	    string::size_type space = line.find(' ');
	    if (space != string::npos) {
		string type = line.substr(0, space);
		line.erase(0, space + 1);
		real_path = line;
		if (type == "flint") {
		    db.add_database(Flint::open(real_path, Xapian::DB_OPEN));
		} else {
		    throw FeatureUnavailableError("Database replication only works with flint databases.");
		}
	    }
	}
	if (db.internal.size() != 1) {
	    throw Xapian::InvalidOperationError("DatabaseReplica needs to be pointed at exactly one subdatabase");
	}
    }

    read_parameters();
}

void
DatabaseReplica::Internal::set_parameter(const string & name,
					 const string & value)
{
    DEBUGCALL(API, void, "DatabaseReplica::Internal::set_parameter",
	      name << ", " << value);
    if (value.empty()) {
	parameters.erase(name);
    } else {
	parameters[name] = value;
    }
    write_parameters();
}

string
DatabaseReplica::Internal::get_parameter(const string & name) const
{
    DEBUGCALL(API, string, "DatabaseReplica::Internal::get_parameter", name);
    map<string, string>::const_iterator i = parameters.find(name);
    if (i == parameters.end()) {
	RETURN("");
    } else {
	RETURN(i->second);
    }
}

string
DatabaseReplica::Internal::get_revision_info() const
{
    DEBUGCALL(API, string, "DatabaseReplica::Internal::get_revision_info", "");
    if (db.internal.size() != 1) {
	throw Xapian::InvalidOperationError("DatabaseReplica needs to be pointed at exactly one subdatabase");
    }
    RETURN((db.internal[0])->get_revision_info());
}

bool 
DatabaseReplica::Internal::apply_next_changeset_from_fd(int fd)
{
    DEBUGCALL(API, bool,
	      "DatabaseReplica::Internal::apply_next_changeset_from_fd", fd);
    // FIXME - implement
    (void) fd;
    RETURN(false);
}
