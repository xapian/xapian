/* omdatabaseinternal.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>
#include "utils.h"
#include "omdatabaseinternal.h"
#include "alltermslist.h"
#include "emptyalltermslist.h"
#include "multialltermslist.h"

#include "progclient.h"
#include "tcpclient.h"

#include "../backends/multi/multi_postlist.h"
#include "../backends/multi/multi_termlist.h"

#include "omdebug.h"
#include <xapian/output.h>
#include <vector>
#include <fstream>

using namespace std;

// Include headers for all the enabled database backends
#ifdef MUS_BUILD_BACKEND_MUSCAT36
#include "../backends/muscat36/da_database.h"
#include "../backends/muscat36/db_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_INMEMORY
#include "../backends/inmemory/inmemory_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_QUARTZ
#include "../backends/quartz/quartz_database.h"
#endif
#ifdef MUS_BUILD_BACKEND_REMOTE
// net_database.h is in common/
#include "net_database.h"
#endif

using std::string;

#ifdef MUS_BUILD_BACKEND_QUARTZ
OmDatabase
OmQuartz__open(const string &dir) {
    return OmDatabase(new OmDatabase::Internal(new QuartzDatabase(dir)));
}

OmWritableDatabase
OmQuartz__open(const string &dir, int action, int block_size) {
    return OmWritableDatabase(new OmDatabase::Internal(
	new QuartzWritableDatabase(dir, action, block_size)));
}
#endif

#ifdef MUS_BUILD_BACKEND_INMEMORY
// Note: a read-only inmemory database will always be empty, and so there's
// not much use in allowing one to be created.
OmWritableDatabase
OmInMemory__open() {
    return OmWritableDatabase(new OmDatabase::Internal(new InMemoryDatabase()));
}
#endif

#ifdef MUS_BUILD_BACKEND_MUSCAT36
OmDatabase
OmMuscat36DA__open(const string &R, const string &T, bool heavy_duty) {
    return OmDatabase(new OmDatabase::Internal(
	new DADatabase(R, T, "", heavy_duty)));
}

OmDatabase
OmMuscat36DA__open(const string &R, const string &T, const string &keys,
		   bool heavy_duty) {
    return OmDatabase(new OmDatabase::Internal(
	new DADatabase(R, T, keys, heavy_duty)));
}

OmDatabase
OmMuscat36DB__open(const string &DB, size_t cache_size) {
    return OmDatabase(new OmDatabase::Internal(
	new DBDatabase(DB, "", cache_size)));
}

OmDatabase
OmMuscat36DB__open(const string &DB, const string &keys, size_t cache_size) {
    return OmDatabase(new OmDatabase::Internal(
	new DBDatabase(DB, keys, cache_size)));
}
#endif

#ifdef MUS_BUILD_BACKEND_REMOTE
OmDatabase
OmRemote__open(const string &program, const string &args, unsigned int timeout)
{
    RefCntPtr<NetClient> link(new ProgClient(program, args, timeout));
    return OmDatabase(new OmDatabase::Internal(new NetworkDatabase(link)));
}

OmDatabase
OmRemote__open(const string &host, unsigned int port,
	unsigned int timeout, unsigned int connect_timeout)
{
    if (connect_timeout == 0) connect_timeout = timeout;
    RefCntPtr<NetClient> link(new TcpClient(host, port, timeout, connect_timeout));
    return OmDatabase(new OmDatabase::Internal(new NetworkDatabase(link)));
}
#endif

OmDatabase
OmStub__open(const string &file)
{
    // A stub database is a text file with one or more lines of this format:
    // <dbtype> <serialised db object>
    ifstream stub(file.c_str());
    OmDatabase db;
    string line;
    int line_no = 1;
    bool ok = false;
    while (getline(stub, line)) {
	string::size_type space = line.find(' ');
	if (space != string::npos) {
	    string type = line.substr(0, space);
	    line.erase(0, space + 1);
	    if (type == "auto") {
		db.add_database(OmAuto__open(line));
		ok = true;
#ifdef MUS_BUILD_BACKEND_QUARTZ
	    } else if (type == "quartz") {
		db.add_database(OmQuartz__open(line));
		ok = true;
#endif
#ifdef MUS_BUILD_BACKEND_REMOTE
	    } else if (type == "remote") {
		string::size_type colon = line.find(':');
		if (colon == 0) {
		    // prog
		    // FIXME: timeouts
		    // FIXME: Is prog actually useful beyond testing?
		    // Is it a security risk?
		    space = line.find(' ');
		    string args;
		    if (space != string::npos) {
			args = line.substr(space + 1);
			line = line.substr(1, space - 1);
		    } else {
			line.erase(0, 1);
		    }
		    db.add_database(OmRemote__open(line, args));
		    ok = true;
		} else if (colon != string::npos) {
		    // tcp
		    // FIXME: timeouts
		    unsigned int port = atoi(line.c_str() + colon);
		    line.erase(colon);
		    db.add_database(OmRemote__open(line, port));
		    ok = true;
		}
#endif
	    }
	    // FIXME: da and db too, but I'm too slack to do those right now!
	}
	if (!ok) break;
	++line_no;
    }
    if (!ok) {
	// Don't include the line itself - that might help an attacker
	// by revealing part of a sensitive file's contents if they can
	// arrange it to be read as a stub database.  The line number is
	// enough information to identify the problem line.
	throw Xapian::OpeningError("Bad line " + om_tostring(line_no) + " in stub database file `" + file + "'");
    }
    return db;
}

OmDatabase
OmAuto__open(const string &path)
{
    // Check for path actually being a file - if so, assume it to be
    // a stub database.
    if (file_exists(path)) {
	return OmStub__open(path);
    }

#ifdef MUS_BUILD_BACKEND_MUSCAT36
    if (file_exists(path + "/R") && file_exists(path + "/T")) {
	// can't easily tell flimsy from heavyduty so assume hd
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return OmMuscat36DA__open(path + "/R", path + "/T", keyfile, true);
    }
    if (file_exists(path + "/DB")) {
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return OmMuscat36DB__open(path + "/DB", keyfile);
    }
    if (file_exists(path + "/DB.da")) {
	string keyfile = path + "/keyfile";
	if (!file_exists(path + "/keyfile")) keyfile = "";
	return OmMuscat36DB__open(path + "/DB.da", keyfile);
    }
#endif
#ifdef MUS_BUILD_BACKEND_QUARTZ
    // FIXME: Quartz has lots of files, and the names may change
    // during development.  Make sure this stays up to date.

    if (file_exists(path + "/record_DB")) {
	return OmQuartz__open(path);
    }
#endif

    throw Xapian::FeatureUnavailableError("Couldn't detect type of database");
}

OmWritableDatabase
OmAuto__open(const std::string &path, int action)
{
    // There's only quartz which is writable at present - if more are added
    // then this code needs to look at action and perhaps autodetect.
    return OmQuartz__open(path, action);
}

/////////////////////////////////////
// Methods of OmDatabase::Internal //
/////////////////////////////////////

OmDatabase::Internal::Internal(Database *db)
{
    add_database(db);
}

void
OmDatabase::Internal::add_database(Database *db)
{
    RefCntPtr<Database> newdb(db);
    databases.push_back(newdb);
}

void
OmDatabase::Internal::add_database(RefCntPtr<Database> newdb)
{
    databases.push_back(newdb);
}

om_doclength
OmDatabase::Internal::get_avlength() const
{
    om_doccount docs = 0;
    om_doclength totlen = 0;

    vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); i++) {
	om_doccount db_doccount = (*i)->get_doccount();
	docs += db_doccount;
	totlen += (*i)->get_avlength() * db_doccount;
    }
    if (docs == 0) return 0.0;

    return totlen / docs;
}

LeafPostList *
OmDatabase::Internal::open_post_list(const string & tname,
				     const OmDatabase &db) const
{
    // Don't bother checking that the term exists first.  If it does, we
    // just end up doing more work, and if it doesn't, we save very little
    // work.
    vector<LeafPostList *> pls;
    try {
	vector<RefCntPtr<Database> >::const_iterator i;
	for (i = databases.begin(); i != databases.end(); i++) {
	    pls.push_back((*i)->open_post_list(tname));
	    pls.back()->next();
	}
	Assert(pls.begin() != pls.end());
    } catch (...) {
	vector<LeafPostList *>::iterator i;
	for (i = pls.begin(); i != pls.end(); i++) {
	    delete *i;
	    *i = 0;
	}
	throw;
    }

    return new MultiPostList(pls, db);
}

LeafTermList *
OmDatabase::Internal::open_term_list(om_docid did, const OmDatabase &db) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return new MultiTermList(databases[dbnumber]->open_term_list(realdid),
			     databases[dbnumber], db);
}

Document *
OmDatabase::Internal::open_document(om_docid did) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;

    return databases[dbnumber]->open_document(realdid);
}

PositionList *
OmDatabase::Internal::open_position_list(om_docid did,
					 const string &tname) const
{
    unsigned int multiplier = databases.size();
    Assert(multiplier != 0);
    om_docid realdid = (did - 1) / multiplier + 1;
    om_doccount dbnumber = (did - 1) % multiplier;
    return databases[dbnumber]->open_position_list(realdid, tname);
}

TermList *
OmDatabase::Internal::open_allterms() const
{
    if (databases.empty()) return new EmptyAllTermsList();
    
    vector<TermList *> lists;

    vector<RefCntPtr<Database> >::const_iterator i;
    for (i = databases.begin(); i != databases.end(); ++i) {
	lists.push_back((*i)->open_allterms());
    }

    if (lists.size() == 1) return lists[0];

    return new MultiAllTermsList(lists);
}
