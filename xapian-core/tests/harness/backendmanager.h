/* backendmanager.h
 *
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_BACKENDMANAGER_H
#define OM_HGUARD_BACKENDMANAGER_H

#include <xapian.h>
#include <vector>

#ifdef __SUNPRO_CC
class Xapian::WritableDatabase; // Sun's CC appears to need this to compile this file
#endif

class BackendManager {
    private:
	/// Index data from zero or more text files into a database.
	void index_files_to_database(Xapian::WritableDatabase & database,
				     const std::vector<std::string> & dbnames);

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

	/// Throw an exception.
	Xapian::Database getdb_none(const std::vector<std::string> &dbnames);

	/// Throw an exception.
	Xapian::WritableDatabase getwritedb_none(const std::vector<std::string> &dbnames);

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
	/// Get an inmemory database instance.
	Xapian::Database getdb_inmemory(const std::vector<std::string> &dbnames);

	/// Get a writable inmemory database instance.
	Xapian::WritableDatabase getwritedb_inmemory(const std::vector<std::string> &dbnames);

#if 0
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
#endif

#ifdef XAPIAN_HAS_REMOTE_BACKEND
	/// Get a remote database instance using xapian-progsrv.
	Xapian::Database getdb_remoteprog(const std::vector<std::string> &dbnames);

	/// Get a writable remote database instance using xapian-progsrv.
	Xapian::WritableDatabase getwritedb_remoteprog(const std::vector<std::string> &dbnames);

	/// Get a remote database instance using xapian-tcpsrv.
	Xapian::Database getdb_remotetcp(const std::vector<std::string> &dbnames);

	/// Get a writable remote database instance using xapian-tcpsrv.
	Xapian::WritableDatabase getwritedb_remotetcp(const std::vector<std::string> &dbnames);
#endif

#ifdef XAPIAN_HAS_FLINT_BACKEND
    private:
	std::string createdb_flint(const std::vector<std::string> &dbnames);

    public:
	/// Get a flint database instance.
	Xapian::Database getdb_flint(const std::vector<std::string> &dbnames);

	/// Get a writable flint database instance.
	Xapian::WritableDatabase getwritedb_flint(const std::vector<std::string> &dbnames);
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
    private:
	std::string createdb_quartz(const std::vector<std::string> &dbnames);

    public:
	/// Get a quartz database instance.
	Xapian::Database getdb_quartz(const std::vector<std::string> &dbnames);

	/// Get a writable quartz database instance.
	Xapian::WritableDatabase getwritedb_quartz(const std::vector<std::string> &dbnames);
#endif

    public:
	/// Constructor - set up default state.
	BackendManager();

	/** Set the database type to use.
	 *
	 *  Valid values for dbtype are "inmemory", "flint", "quartz",
	 *  "none", "da", "daflimsy", "db", "dbflimsy", "remoteprog", and
	 *  "remotetcp".
	 */
	void set_dbtype(const std::string &type);

	/** Get the database type currently in use. */
	const std::string & get_dbtype() const { return current_type; }

	/** Set the directory to store data in.
	 */
	void set_datadir(const std::string &datadir_) { datadir = datadir_; }

	/** Get the directory to store data in.
	 */
	const std::string & get_datadir() const { return datadir; }

	/// Get a database instance of the current type.
	Xapian::Database get_database(const std::vector<std::string> &dbnames);

	/// Get a database instance of the current type, single file case.
	Xapian::Database get_database(const std::string &dbname);

	/// Get a writable database instance.
	Xapian::WritableDatabase get_writable_database(const std::string & dbname);

	/// Get the command line required to run xapian-progsrv.
	static const char * get_xapian_progsrv_command();
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
