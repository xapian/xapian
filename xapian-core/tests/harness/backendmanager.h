/** @file
 * @brief Base class for backend handling in test harness
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2017,2018 Olly Betts
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

// Paths to xapian-tcpsrv and xapian-progsrv.
#ifdef __WIN32__
// Under __WIN32__ we want \ path separators since we pass this path to
// CreateProcess().
# define XAPIAN_BIN_PATH "..\\bin\\"
# define EXE_SUFFIX ".exe"
#else
# define XAPIAN_BIN_PATH "../bin/"
# define EXE_SUFFIX
#endif
#define XAPIAN_TCPSRV XAPIAN_BIN_PATH "xapian-tcpsrv" EXE_SUFFIX
#define XAPIAN_PROGSRV XAPIAN_BIN_PATH "xapian-progsrv" EXE_SUFFIX

class BackendManager {
    /// The current data directory
    std::string datadir;

    /// The backend database type (set by subclass ctor).
    std::string dbtype;

  protected:
    /// Index data from zero or more text files into a database.
    void index_files_to_database(Xapian::WritableDatabase & database,
				 const std::vector<std::string> & files);

    bool create_dir_if_needed(const std::string &dirname);

    /** Virtual method implementing get_database().
     *
     *  If we just called this get_database() then each subclass which
     *  defined it would also need to un-hide the non-virtual overloaded method
     *  with "using get_database(const std::string&);" or similar.
     */
    virtual Xapian::Database do_get_database(const std::vector<std::string> &files);

    /** Virtual method implementing get_database_path().
     *
     *  If we just called this get_database_path() then each subclass which
     *  defined it would also need to un-hide the non-virtual overloaded method
     *  with "using get_database_path(const std::string&);" or similar.
     */
    virtual std::string do_get_database_path(const std::vector<std::string> &files);

  public:
    /// Constructor.
    BackendManager(const std::string& datadir_, const std::string& dbtype_)
	: datadir(datadir_), dbtype(dbtype_) {}

    /** We have virtual methods and want to be able to delete derived classes
     *  using a pointer to the base class, so we need a virtual destructor.
     */
    virtual ~BackendManager();

    /** Get the database type currently in use.
     */
    const std::string& get_dbtype() const { return dbtype; }

    /** Get the directory to store data in.
     */
    const std::string & get_datadir() const { return datadir; }

    /// Get a database instance of the current type.
    Xapian::Database get_database(const std::vector<std::string> &files);

    /// Get a database instance of the current type, single file case.
    Xapian::Database get_database(const std::string &file);

    /** Get a database instance of the current type, generated case.
     *
     * @param dbname	The name of the database (base on your testcase name).
     * @param gen	Generator function - should index data to the empty
     *			WritableDatabase provided.
     * @param arg	String argument to pass to @a gen - it's up to you how
     *			to make use of this (or just ignore it if you don't need
     *			it).
     */
    Xapian::Database get_database(const std::string &dbname,
				  void (*gen)(Xapian::WritableDatabase&,
					      const std::string &),
				  const std::string &arg);

    /// Get a database instance by path
    virtual Xapian::Database get_database_by_path(const std::string& path);

    /// Get the path of a database instance, if such a thing exists.
    std::string get_database_path(const std::vector<std::string> &files);

    /// Get the path of a database instance, if such a thing exists (single file case).
    std::string get_database_path(const std::string &file);

    /// Get the path of a generated database instance.
    std::string get_database_path(const std::string &dbname,
				  void (*gen)(Xapian::WritableDatabase&,
					      const std::string &),
				  const std::string &arg);

    /// Get a writable database instance.
    virtual Xapian::WritableDatabase get_writable_database(const std::string & name, const std::string & file);

    /// Get a remote Xapian::WritableDatabase instance with specified args.
    virtual Xapian::WritableDatabase
    get_remote_writable_database(std::string args);

    /// Get the path of a writable database instance, if such a thing exists.
    virtual std::string get_writable_database_path(const std::string & name);

    /// Get a generated writable database instance
    virtual Xapian::WritableDatabase
    get_generated_database(const std::string& name);

    /// Get the path to use for generating a database, if supported.
    virtual std::string get_generated_database_path(const std::string & name);

    /// Finalise the generated database
    virtual void finalise_generated_database(const std::string& name);

    /// Get a remote database instance with the specified timeout.
    virtual Xapian::Database get_remote_database(const std::vector<std::string> & files, unsigned int timeout);

    /** Get the args for opening a writable remote database with the
     *  specified timeout.
     */
    virtual std::string get_writable_database_args(const std::string& path,
						   unsigned int timeout);

    /// Create a Database object for the last opened WritableDatabase.
    virtual Xapian::Database get_writable_database_as_database();

    /// Create a WritableDatabase object for the last opened WritableDatabase.
    virtual Xapian::WritableDatabase get_writable_database_again();

    /// Get the path of the last opened WritableDatabase.
    virtual std::string get_writable_database_path_again();

    /// Get a path to compact a database to.
    virtual std::string get_compaction_output_path(const std::string& name);

    /** Called after each test, to perform any necessary cleanup.
     *
     *  May be called more than once for a given test in some cases.
     */
    virtual void clean_up();

    /// Get the command line required to run xapian-progsrv.
    static const char * get_xapian_progsrv_command();
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
