/* backendmanager.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004 Olly Betts
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

#ifndef OM_HGUARD_BACKENDMANAGER_H
#define OM_HGUARD_BACKENDMANAGER_H

#include <xapian.h>
#include <vector>

#ifdef __SUNPRO_CC
class Xapian::WritableDatabase; // Sun's CC appears to need this to compile this file
#endif

class BackendManager {
    private:
	/// The type of a get_database member function
	typedef Xapian::Database (BackendManager::*getdb_func)
				   (const std::vector<std::string> &dbnames);

	/// The type of a get_writable_database member function
	typedef Xapian::WritableDatabase (BackendManager::*getwritedb_func)
				   (const std::vector<std::string> &dbnames);

	/// The current get_database member function
	getdb_func do_getdb;

	/// The current get_writable_database member function
	getwritedb_func do_getwritedb;

	/// The current data directory
	std::string datadir;

	/// The current backend type
	std::string current_type;

	/// Change names of databases into paths to them, within the datadir
	std::vector<std::string>
		change_names_to_paths(const std::vector<std::string> &dbnames);

	/// Throw an exception.
	Xapian::Database getdb_void(const std::vector<std::string> &dbnames);

	/// Throw an exception.
	Xapian::WritableDatabase getwritedb_void(const std::vector<std::string> &dbnames);

#ifdef MUS_BUILD_BACKEND_INMEMORY
	/// Get an inmemory database instance.
	Xapian::Database getdb_inmemory(const std::vector<std::string> &dbnames);

	/// Get a writable inmemory database instance.
	Xapian::WritableDatabase getwritedb_inmemory(const std::vector<std::string> &dbnames);

	/** Get an inmemory database instance, which will throw an error when
	 *  next is called.
	 */
	Xapian::Database getdb_inmemoryerr(const std::vector<std::string> &dbnames);
	Xapian::Database getdb_inmemoryerr2(const std::vector<std::string> &dbnames);
	Xapian::Database getdb_inmemoryerr3(const std::vector<std::string> &dbnames);

	/** Get a writable inmemory database instance, which will throw an
	 *  error when next is called.
	 */
	Xapian::WritableDatabase getwritedb_inmemoryerr(const std::vector<std::string> &dbnames);
	Xapian::WritableDatabase getwritedb_inmemoryerr2(const std::vector<std::string> &dbnames);
	Xapian::WritableDatabase getwritedb_inmemoryerr3(const std::vector<std::string> &dbnames);
#endif

#ifdef MUS_BUILD_BACKEND_REMOTE
	/// Get a remote database instance
	Xapian::Database getdb_remote(const std::vector<std::string> &dbnames);

	/// Get a writable remote database instance
	Xapian::WritableDatabase getwritedb_remote(const std::vector<std::string> &dbnames);
#endif

#ifdef MUS_BUILD_BACKEND_QUARTZ
	/// Get a quartz database instance.
	Xapian::Database getdb_quartz(const std::vector<std::string> &dbnames);

	/// Get a writable quartz database instance.
	Xapian::WritableDatabase getwritedb_quartz(const std::vector<std::string> &dbnames);
#endif

#ifdef MUS_BUILD_BACKEND_MUSCAT36
	/// Get a da database instance.
	Xapian::Database getdb_da(const std::vector<std::string> &dbnames);

	/// Get a writable da database instance.
	Xapian::WritableDatabase getwritedb_da(const std::vector<std::string> &dbnames);

	/// Get a daflimsy database instance.
	Xapian::Database getdb_daflimsy(const std::vector<std::string> &dbnames);

	/// Get a writable daflimsy database instance.
	Xapian::WritableDatabase getwritedb_daflimsy(const std::vector<std::string> &dbnames);

	/// Get a db database instance.
	Xapian::Database getdb_db(const std::vector<std::string> &dbnames);

	/// Get a writable db database instance.
	Xapian::WritableDatabase getwritedb_db(const std::vector<std::string> &dbnames);

	/// Get a dbflimsy database instance.
	Xapian::Database getdb_dbflimsy(const std::vector<std::string> &dbnames);

	/// Get a writable dbflimsy database instance.
	Xapian::WritableDatabase getwritedb_dbflimsy(const std::vector<std::string> &dbnames);
#endif


    public:
	/// Constructor - set up default state.
	BackendManager() :
		do_getdb(&BackendManager::getdb_void),
		do_getwritedb(&BackendManager::getwritedb_void) {}

	/** Set the database type to use.
	 *
	 *  Valid values for dbtype are "inmemory", "quartz",
	 *  "void", "da", "daflimsy", "db", "dbflimsy", and "remote".
	 */
	void set_dbtype(const std::string &type);

	/** Set the directory to store data in.
	 */
	void set_datadir(const std::string &datadir_);

	/** Get the directory to store data in.
	 */
	std::string get_datadir();

	/// Get a database instance of the current type.
	Xapian::Database get_database(const std::vector<std::string> &dbnames);

	/// Get a database instance of the current type, single file case.
	Xapian::Database get_database(const std::string &dbname);

	/// Get a writable database instance.
	Xapian::WritableDatabase get_writable_database(const std::string & dbname);
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
