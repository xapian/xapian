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

#include "om/om.h"
#include <vector>

class BackendManager {
    private:
	/// The type of a get_database member function
	typedef OmDatabase (BackendManager::*getdb_func)
				   (const vector<string> &dbnames);

	/// The type of a get_writable_database member function
	typedef OmWritableDatabase (BackendManager::*getwritedb_func)
				   (const vector<string> &dbnames);

	/// The current get_database member function
	getdb_func do_getdb;

	/// The current get_writable_database member function
	getwritedb_func do_getwritedb;

	/// The current data directory
	string datadir;

	/// Change names of databases into paths to them, within the datadir
	vector<string>
		change_names_to_paths(const vector<string> &dbnames);

	/// Throw an exception.
	OmDatabase getdb_void(const vector<string> &dbnames);

	/// Throw an exception.
	OmWritableDatabase getwritedb_void(const vector<string> &dbnames);

	/// Get an inmemory database instance.
	OmDatabase getdb_inmemory(const vector<string> &dbnames);

	/// Get an writable inmemory database instance.
	OmWritableDatabase getwritedb_inmemory(const vector<string> &dbnames);

	/// Get a net database instance
	OmDatabase getdb_net(const vector<string> &dbnames);

	/// Get a writable net database instance
	OmWritableDatabase getwritedb_net(const vector<string> &dbnames);

	/// Get a sleepy database instance.
	OmDatabase getdb_sleepy(const vector<string> &dbnames);

	/// Get a writable sleepy database instance.
	OmWritableDatabase getwritedb_sleepy(const vector<string> &dbnames);
    public:
	/// Constructor - set up default state.
	BackendManager() :
		do_getdb(&BackendManager::getdb_void),
		do_getwritedb(&BackendManager::getwritedb_void) {};

	/** Set the database type to use.
	 *
	 *  Valid values for dbtype are "inmemory", "sleepycat",
	 *  "void", and "net".
	 */
	void set_dbtype(const string &type);

	/** Set the directory to store data in.
	 */
	void set_datadir(const string &datadir_);

	/// Get a database instance of the current type
	OmDatabase get_database(const vector<string> &dbnames);

	/// Get a database instance from individually named databases
	OmDatabase get_database(const string &dbname1,
				const string &dbname2 = "");

	/// Get a writable database instance
	OmWritableDatabase get_writable_database(const string & dbname);
};
