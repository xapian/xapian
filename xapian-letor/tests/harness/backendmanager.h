/** @file backendmanager.h
 * @brief Base class for backend handling in test harness
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
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

class BackendManager {
    /// The current data directory
    std::string datadir;

    /// Index data from zero or more text files into a database.
    void index_files_to_database(Xapian::WritableDatabase & database,
				 const std::vector<std::string> & files);

    bool create_dir_if_needed(const std::string &dirname);

    /// Proxy method implementing get_database().
    Xapian::Database do_get_database(const std::vector<std::string> &files);

    /// Proxy method implementing get_database_path().
    std::string do_get_database_path(const std::vector<std::string> &files);

    /// Returns path to an indexed Xapian::WritableDatabase
    std::string createdb(const std::vector<std::string> &files);

  public:
    /// Constructor - set up default state.
    BackendManager() { }

    /// Destructor
    ~BackendManager();

    /** Get the database type currently in use.
     */
    std::string get_dbtype() const;

    /** Set the directory to store data in.
     */
    void set_datadir(const std::string &datadir_) { datadir = datadir_; }

    /** Get the directory to store data in.
     */
    const std::string & get_datadir() const { return datadir; }

    /// Get a database instance of the current type.
    Xapian::Database get_database(const std::vector<std::string> &files);

    /// Get a database instance of the current type, single file case.
    Xapian::Database get_database(const std::string &file);

    /// Get the path of a database instance, if such a thing exists.
    std::string get_database_path(const std::vector<std::string> &files);

    /// Get the path of a database instance, if such a thing exists (single file case).
    std::string get_database_path(const std::string &file);

    /** Called after each test, to perform any necessary cleanup.
     *
     *  May be called more than once for a given test in some cases.
     */
    void clean_up();

    /// Get the command line required to run xapian-progsrv.
    static const char * get_xapian_progsrv_command();
};

#endif /* OM_HGUARD_BACKENDMANAGER_H */
