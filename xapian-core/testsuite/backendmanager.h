/* backendmanager.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "om/om.h"
#include <vector>

class BackendManager {
    private:
	/// The type of a get_database member function
	typedef OmDatabase (BackendManager::*getdb_func)
				   (const std::vector<std::string> &dbnames);

	/// The type of a get_writable_database member function
	typedef OmWritableDatabase (BackendManager::*getwritedb_func)
				   (const std::vector<std::string> &dbnames);

	/// The current get_database member function
	getdb_func do_getdb;

	/// The current get_writable_database member function
	getwritedb_func do_getwritedb;

	/// The current data directory
	std::string datadir;

	/// Change names of databases into paths to them, within the datadir
	std::vector<std::string>
		change_names_to_paths(const std::vector<std::string> &dbnames);

	/// Throw an exception.
	OmDatabase getdb_void(const std::vector<std::string> &dbnames);

	/// Throw an exception.
	OmWritableDatabase getwritedb_void(const std::vector<std::string> &dbnames);

	/// Get an inmemory database instance.
	OmDatabase getdb_inmemory(const std::vector<std::string> &dbnames);

	/// Get an writable inmemory database instance.
	OmWritableDatabase getwritedb_inmemory(const std::vector<std::string> &dbnames);

	/// Get a net database instance
	OmDatabase getdb_net(const std::vector<std::string> &dbnames);

	/// Get a writable net database instance
	OmWritableDatabase getwritedb_net(const std::vector<std::string> &dbnames);

	/// Get a sleepycat database instance.
	OmDatabase getdb_sleepycat(const std::vector<std::string> &dbnames);

	/// Get a writable sleepycat database instance.
	OmWritableDatabase getwritedb_sleepycat(const std::vector<std::string> &dbnames);

	/// Do the actual work of creating a sleepycat database instance.
	OmWritableDatabase do_getwritedb_sleepycat(const std::vector<std::string> &dbnames,
						   bool writable);
    
	/// Get a quartz database instance.
	OmDatabase getdb_quartz(const std::vector<std::string> &dbnames);

	/// Get a writable quartz database instance.
	OmWritableDatabase getwritedb_quartz(const std::vector<std::string> &dbnames);

	/// Do the actual work of creating a quartz database instance.
	OmWritableDatabase do_getwritedb_quartz(const std::vector<std::string> &dbnames,
						bool writable);

    public:
	/// Constructor - set up default state.
	BackendManager() :
		do_getdb(&BackendManager::getdb_void),
		do_getwritedb(&BackendManager::getwritedb_void) {};

	/** Set the database type to use.
	 *
	 *  Valid values for dbtype are "inmemory", "sleepycat", "quartz",
	 *  "void", and "net".
	 */
	void set_dbtype(const std::string &type);

	/** Set the directory to store data in.
	 */
	void set_datadir(const std::string &datadir_);

	/// Get a database instance of the current type
	OmDatabase get_database(const std::vector<std::string> &dbnames);

	/// Get a database instance from individually named databases
	OmDatabase get_database(const std::string &dbname1,
				const std::string &dbname2 = "");

	/// Get a writable database instance
	OmWritableDatabase get_writable_database(const std::string & dbname);
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
