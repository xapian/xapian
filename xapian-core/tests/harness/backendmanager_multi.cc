/** @file backendmanager_multi.cc
 * @brief BackendManager subclass for multi databases.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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
#include "str.h"
#include "utils.h"

#include <cstdio> // For rename().
#include <cstring>
#include "safeerrno.h"

using namespace std;

BackendManagerMulti::BackendManagerMulti(const std::string & subtype_)
	: subtype(subtype_)
{
    if (!(false
#ifdef XAPIAN_HAS_BRASS_BACKEND
	  || subtype == "brass"
#endif
#ifdef XAPIAN_HAS_CHERT_BACKEND
	  || subtype == "chert"
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
	  || subtype == "flint"
#endif
	 )) {
	throw ("Unknown backend type \"" + subtype + "\" specified for multi database subdatabases");
    }
}

std::string
BackendManagerMulti::get_dbtype() const
{
    return "multi_" + subtype;
}

#define NUMBER_OF_SUB_DBS 2

string
BackendManagerMulti::createdb_multi(const vector<string> & files)
{
    string dbdir = ".multi" + subtype;
    create_dir_if_needed(dbdir);

    string dbname = "db";
    vector<string>::const_iterator i;
    for (i = files.begin(); i != files.end(); ++i) {
	dbname += "__";
	dbname += *i;
    }
    string dbpath = dbdir + "/" + dbname;

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

    // Open NUMBER_OF_SUB_DBS databases and index files to them alternately so
    // a multi-db combining them contains the documents in the expected order.
    Xapian::WritableDatabase dbs[NUMBER_OF_SUB_DBS];
    for (size_t n = 0; n < NUMBER_OF_SUB_DBS; ++n) {
	string subdbdir = dbname;
	subdbdir += "___";
	subdbdir += str(n);
#if defined XAPIAN_HAS_BRASS_BACKEND
	if (subtype == "brass") {
	    dbs[n] = Xapian::Brass::open(dbdir + "/" + subdbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	    out << "brass " << subdbdir << '\n';
	}
#endif
#if defined XAPIAN_HAS_CHERT_BACKEND
	if (subtype == "chert") {
	    dbs[n] = Xapian::Chert::open(dbdir + "/" + subdbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	    out << "chert " << subdbdir << '\n';
	}
#endif
#ifdef XAPIAN_HAS_FLINT_BACKEND
	if (subtype == "flint") {
	    dbs[n] = Xapian::Flint::open(dbdir + "/" + subdbdir, Xapian::DB_CREATE_OR_OVERWRITE);
	    out << "flint " << subdbdir << '\n';
	}
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

string
BackendManagerMulti::do_get_database_path(const vector<string> & files)
{
    return createdb_multi(files);
}

Xapian::WritableDatabase
BackendManagerMulti::get_writable_database(const string &, const string &)
{
    throw Xapian::UnimplementedError("Multi-databases don't support writing");
}
