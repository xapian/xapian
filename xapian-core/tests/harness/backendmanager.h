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

// Paths to xapian-tcpsrv and xapian-progsrv.
#ifdef __WIN32__
// Under __WIN32__ we want \ path separators since we pass this path to
// CreateProcess().
# ifdef _MSC_VER
#  ifdef DEBUG
#   define XAPIAN_BIN_PATH "..\\win32\\Debug\\"
#  else
#   define XAPIAN_BIN_PATH "..\\win32\\Release\\"
#  endif
# else
#  define XAPIAN_BIN_PATH "..\\bin\\" // mingw
# endif
#else
# define XAPIAN_BIN_PATH "../bin/"
#endif
#define XAPIAN_TCPSRV XAPIAN_BIN_PATH"xapian-tcpsrv"
#define XAPIAN_PROGSRV XAPIAN_BIN_PATH"xapian-progsrv"

#ifdef __SUNPRO_CC
class Xapian::WritableDatabase; // Sun's CC appears to need this to compile this file
#endif

class BackendManager {
    /// The current data directory
    std::string datadir;

    /// Index data from zero or more text files into a database.
    void index_files_to_database(Xapian::WritableDatabase & database,
				 const std::vector<std::string> & dbnames);

  protected:
    bool create_dir_if_needed(const std::string &dirname);

#ifdef XAPIAN_HAS_INMEMORY_BACKEND
    /// Get a writable inmemory database instance.
    Xapian::WritableDatabase getwritedb_inmemory(const std::vector<std::string> &dbnames);
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
  protected:
    std::string createdb_flint(const std::vector<std::string> &dbnames);

  public:
    /// Get a writable flint database instance.
    Xapian::WritableDatabase getwritedb_flint(const std::string & name,
					      const std::vector<std::string> &files);
#endif

#ifdef XAPIAN_HAS_QUARTZ_BACKEND
  protected:
    std::string createdb_quartz(const std::vector<std::string> &dbnames);

  public:
    /// Get a writable quartz database instance.
    Xapian::WritableDatabase getwritedb_quartz(const std::string & name,
					       const std::vector<std::string> &files);
#endif

  public:
    /// Constructor - set up default state.
    BackendManager() { }

    /// Virtual methods, so virtual destructor.
    virtual ~BackendManager() { } // FIXME: move out of header

    /** Get the database type currently in use.
     *
     *  Current possible return values are "inmemory", "flint", "quartz",
     *  "none", "remoteprog", and "remotetcp".
     */
    virtual const char * get_dbtype() const { return "none"; } // FIXME: move out of header

    /** Set the directory to store data in.
     */
    void set_datadir(const std::string &datadir_) { datadir = datadir_; }

    /** Get the directory to store data in.
     */
    const std::string & get_datadir() const { return datadir; }

    /// Get a database instance of the current type.
    virtual Xapian::Database get_database(const std::vector<std::string> &dbnames);

    /// Get a database instance of the current type, single file case.
    virtual Xapian::Database get_database(const std::string &dbname);

    /// Get a writable database instance.
    virtual Xapian::WritableDatabase get_writable_database(const std::string & name, const std::string & file);

    /// Get a remote database instance with the specified timeout.
    virtual Xapian::Database get_remote_database(const std::vector<std::string> & files, unsigned int timeout);

    /// Create a Database object for the last opened WritableDatabase.
    virtual Xapian::Database get_writable_database_as_database();

    /// Create a WritableDatabase object for the last opened WritableDatabase.
    virtual Xapian::WritableDatabase get_writable_database_again();

    /// Get the command line required to run xapian-progsrv.
    static const char * get_xapian_progsrv_command();
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
