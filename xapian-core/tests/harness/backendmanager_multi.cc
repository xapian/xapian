/** @file backendmanager_multi.cc
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "backendmanager_multi.h"

#include "index_utils.h"
#include "utils.h"

#include <cstdio>
#include <cstring>
#include "safeerrno.h"

using namespace std;

BackendManagerMulti::~BackendManagerMulti() { }

const char *
BackendManagerMulti::get_dbtype() const
{
    return "multi";
}

#define NUMBER_OF_SUB_DBS 2

string
BackendManagerMulti::createdb_multi(const vector<string> & files)
{
    create_dir_if_needed(".multi");

    string dbpath = ".multi/db";
    vector<string>::const_iterator i;
    for (i = files.begin(); i != files.end(); ++i) {
	dbpath += "__";
	dbpath += *i;
    }

    if (file_exists(dbpath)) return dbpath;

    string tmpfile = dbpath;
    tmpfile += ".tmp";
    ofstream out(tmpfile.c_str());
    if (!out.is_open()) {
	string msg = "Couldn't create file '";
	msg += tmpfile;
	msg += "' (";
	msg += strerror(errno);
	msg += ')';
	throw msg;
    }

    // Open NUMBER_OF_SUB_DBS databases and index files to the alternately so a
    // multi-db combining them contains the documents in the expected order.
    Xapian::WritableDatabase dbs[NUMBER_OF_SUB_DBS];
    for (size_t n = 0; n < NUMBER_OF_SUB_DBS; ++n) {
	string subdbdir = dbpath;
	subdbdir += "___";
	subdbdir += om_tostring(n);
#ifdef XAPIAN_HAS_FLINT_BACKEND
	dbs[n] = Xapian::Flint::open(subdbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	out << "flint " << subdbdir << '\n';
#else
	dbs[n] = Xapian::Quartz::open(subdbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	out << "quartz " << subdbdir << '\n';
#endif
    }
    out.close();

    size_t c = 0;
    FileIndexer f(get_datadir(), files);
    while (f) {
	dbs[c].add_document(f.next());
	c = (c + 1) % NUMBER_OF_SUB_DBS;
    }

    rename(tmpfile.c_str(), dbpath.c_str());

    return dbpath;
}

Xapian::Database
BackendManagerMulti::get_database(const vector<string> & files)
{
    return Xapian::Auto::open_stub(createdb_multi(files));
}

Xapian::Database
BackendManagerMulti::get_database(const string & file)
{
    return BackendManagerMulti::get_database(vector<string>(1, file));
}

Xapian::WritableDatabase
BackendManagerMulti::get_writable_database(const string &, const string &)
{
    throw Xapian::UnimplementedError("Multi-databases don't support writing");
}
