/* backendmanager.cc: manage backends for testsuite
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>
#include <fstream>
#include <string>
#include <vector>

#include "autoptr.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include <xapian.h>
#include "textfile_indexer.h"
#include "index_utils.h"
#include "backendmanager.h"
#include "omdebug.h"
#include "utils.h"

using namespace std;

Xapian::Document
string_to_document(string paragraph)
{
    Xapian::Stem stemmer("english");

    Xapian::Document document;
    document.set_data(paragraph);
    Xapian::termcount position = 1;

    for (Xapian::valueno i = 1; i <= 10; ++i) {
	if (i >= paragraph.length()) break;
	string value = paragraph.substr(i, 1);
	document.add_value(i, value);
    }

    {
	/* We need a value which will be useful for collapsing with DA
	 * databases, where only the first 8 bytes of value 0 count.
	 */
	string value;

	value = paragraph[2];

	value += string("\0\0\0 \1\t", 6);

	for (int k = 0; k < 256; k++) {
	    value += (char)(k);
	}
	value += paragraph;
	document.add_value(0, value);
    }

    string::size_type spacepos;
    string word;
    while ((spacepos = paragraph.find_first_not_of(" \t\n")) != string::npos) {
	if (spacepos) paragraph.erase(0, spacepos);
	spacepos = paragraph.find_first_of(" \t\n");
	word = paragraph.substr(0, spacepos);
	select_characters(word, "");
	lowercase_term(word);
	word = stemmer.stem_word(word);
	if (!word.empty()) {
	    document.add_posting(word, position++);
	}
	paragraph.erase(0, spacepos);
    }

    return document;
}

void
index_files_to_database(Xapian::WritableDatabase & database,
                        vector<string> paths)
{
    vector<string>::const_iterator p;
    for (p = paths.begin(); p != paths.end(); ++p) {
	TextfileIndexerSource source(*p);
	AutoPtr<istream> from(source.get_stream());

	while (*from) {
	    string para;
	    get_paragraph(*from, para);
	    database.add_document(string_to_document(para));
	}
    }
}

void
index_files_to_m36(const string &prog, const string &dbdir,
		   vector<string> paths)
{
    string dump = dbdir + "/DATA";
    ofstream out(dump.c_str());
    string valuefile = dbdir + "/keyfile";
    ofstream values(valuefile.c_str());
    values << "omrocks!"; // magic word
    for (vector<string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	AutoPtr<istream> from(source.get_stream());

	while (*from) {
	    string para;
	    get_paragraph(*from, para);
	    Xapian::Document doc = string_to_document(para);
	    out << "#RSTART#\n" << doc.get_data() << "\n#REND#\n#TSTART#\n";
	    {
		Xapian::TermIterator i = doc.termlist_begin();
		Xapian::TermIterator i_end = doc.termlist_end();
		for ( ; i != i_end; ++i) {
		    out << *i << endl;
		}
	    }
	    out << "#TEND#\n";
	    Xapian::ValueIterator value_i = doc.values_begin();
	    string value = string("\0\0\0\0\0\0\0", 8);
	    if (value_i != doc.values_end()) value = (*value_i) + value;
	    value = value.substr(0, 8);
	    values << value;
	}
    }
    out.close();
    string cmd = "../../makeda/" + prog + " -source " + dump +
	" -da " + dbdir + "/ -work " + dbdir + "/tmp- > /dev/null";
    system(cmd);
    unlink(dump);
}

vector<string>
make_strvec(string s1 = "", string s2 = "", string s3 = "", string s4 = "")
{
    vector<string> result;

    if (!s1.empty()) result.push_back(s1);
    if (!s2.empty()) result.push_back(s2);
    if (!s3.empty()) result.push_back(s3);
    if (!s4.empty()) result.push_back(s4);

    return result;
}

void
index_file_to_database(Xapian::WritableDatabase & database, string path)
{
    index_files_to_database(database, make_strvec(path));
}

void
BackendManager::set_dbtype(const string &type)
{
    if (type == current_type) {
	// leave it as it is.
    } else if (type == "inmemory") {
#ifdef MUS_BUILD_BACKEND_INMEMORY
	do_getdb = &BackendManager::getdb_inmemory;
	do_getwritedb = &BackendManager::getwritedb_inmemory;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#if 0
#ifdef MUS_BUILD_BACKEND_INMEMORY
    } else if (type == "inmemoryerr") {
	do_getdb = &BackendManager::getdb_inmemoryerr;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr;
    } else if (type == "inmemoryerr2") {
	do_getdb = &BackendManager::getdb_inmemoryerr2;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr2;
    } else if (type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_inmemoryerr3;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr3;
#else
    } else if (type == "inmemoryerr" || type == "inmemoryerr2" ||
	       type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#endif
    } else if (type == "quartz") {
#ifdef MUS_BUILD_BACKEND_QUARTZ
	do_getdb = &BackendManager::getdb_quartz;
	do_getwritedb = &BackendManager::getwritedb_quartz;
	rmdir(".quartz");
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "remote") {
#ifdef MUS_BUILD_BACKEND_REMOTE
	do_getdb = &BackendManager::getdb_remote;
	do_getwritedb = &BackendManager::getwritedb_remote;
#else
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
#ifdef MUS_BUILD_BACKEND_MUSCAT36
    } else if (type == "da") {
	do_getdb = &BackendManager::getdb_da;
	do_getwritedb = &BackendManager::getwritedb_da;
	rmdir(".da");
    } else if (type == "db") {
	do_getdb = &BackendManager::getdb_db;
	do_getwritedb = &BackendManager::getwritedb_db;
	rmdir(".db");
    } else if (type == "daflimsy") {
	do_getdb = &BackendManager::getdb_daflimsy;
	do_getwritedb = &BackendManager::getwritedb_daflimsy;
	rmdir(".daflimsy");
    } else if (type == "dbflimsy") {
	do_getdb = &BackendManager::getdb_dbflimsy;
	do_getwritedb = &BackendManager::getwritedb_dbflimsy;
	rmdir(".dbflimsy");
#else
    } else if (type == "da" || type == "db" || type == "daflimsy" ||
	       type == "dbflimsy") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
#endif
    } else if (type == "void") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
    } else {
	throw Xapian::InvalidArgumentError(
		"Expected inmemory, quartz, remote, da, db, "
		"daflimsy, dbflimsy, or void");
    }
    current_type = type;
}

void
BackendManager::set_datadir(const string &datadir_)
{
    datadir = datadir_;
}

string
BackendManager::get_datadir()
{
    return datadir;
}

Xapian::Database
BackendManager::getdb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_void(const vector<string> &)
{
    throw Xapian::InvalidArgumentError("Attempted to open a disabled database");
}

vector<string>
BackendManager::change_names_to_paths(const vector<string> &dbnames)
{
    vector<string> paths;
    vector<string>::const_iterator i;
    for (i = dbnames.begin(); i != dbnames.end(); ++i) {
	if (!i->empty()) {
	    if (datadir.empty()) {
		paths.push_back(*i);
	    } else {
		paths.push_back(datadir + "/" + *i + ".txt");
	    }
	}
    }
    return paths;
}

#ifdef MUS_BUILD_BACKEND_INMEMORY
Xapian::Database
BackendManager::getdb_inmemory(const vector<string> &dbnames)
{
    return getwritedb_inmemory(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemory(const vector<string> &dbnames)
{
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));
    return db;
}

#if 0
Xapian::Database
BackendManager::getdb_inmemoryerr(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_errornext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr2(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr2(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr2(const vector<string> &dbnames)
{
    // FIXME: params.set("inmemory_abortnext", 1);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

Xapian::Database
BackendManager::getdb_inmemoryerr3(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr3(dbnames);
}

Xapian::WritableDatabase
BackendManager::getwritedb_inmemoryerr3(const vector<string> &dbnames)
{
    // params.set("inmemory_abortnext", 2);
    Xapian::WritableDatabase db(Xapian::InMemory::open());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}
#endif
#endif

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const string &dirname)
{
    // create a directory if not present
    struct stat sbuf;
    int result = stat(dirname, &sbuf);
    if (result < 0) {
	if (errno != ENOENT)
	    throw Xapian::DatabaseOpeningError("Can't stat directory");
        if (mkdir(dirname, 0700) < 0)
	    throw Xapian::DatabaseOpeningError("Can't create directory");
	return true; // Successfully created a directory.
    }
    if (!S_ISDIR(sbuf.st_mode))
	throw Xapian::DatabaseOpeningError("Is not a directory.");
    return false; // Already a directory.
}

#ifdef MUS_BUILD_BACKEND_QUARTZ
Xapian::Database
BackendManager::getdb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); i++) {
	dbdir += "=" + *i;
    }
    // If the database is readonly, we can reuse it if it exists.
    if (create_dir_if_needed(dbdir)) {
	// Directory was created, so do the indexing.
	Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE));
	index_files_to_database(db, change_names_to_paths(dbnames));
    }
    return Xapian::Quartz::open(dbdir);
}

Xapian::WritableDatabase
BackendManager::getwritedb_quartz(const vector<string> &dbnames)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    // Add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused).
    string dbdir = parent_dir + "/dbw";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end(); ++i) {
	dbdir += "=" + *i;
    }
    // For a writable database we need to start afresh each time.
    rmdir(dbdir);
    (void)create_dir_if_needed(dbdir);
    touch(dbdir + "/log");
    // directory was created, so do the indexing.
    Xapian::WritableDatabase db(Xapian::Quartz::open(dbdir, Xapian::DB_CREATE));
    index_files_to_database(db, change_names_to_paths(dbnames));
    return db;
}
#endif

#ifdef MUS_BUILD_BACKEND_REMOTE
Xapian::Database
BackendManager::getdb_remote(const vector<string> &dbnames)
{
    // run an omprogsrv for now.  Later we should also use omtcpsrv
    string args = datadir;
    bool timeout = false;
    vector<string>::const_iterator i;
    for (i = dbnames.begin(); i != dbnames.end(); ++i) {
	if (*i == "#TIMEOUT#") {
	    ++i;
	    if (i == dbnames.end()) {
		throw Xapian::InvalidArgumentError("Missing timeout parameter");
	    }
	    args += " -t" + *i;
	    timeout = true;
	} else {
	    args += ' ';
	    args += *i;
	}
    }
    // Nice long timeout (5 minutes) so we don't timeout just because
    // the host is slow.
    if (!timeout) args += " -t300000";
    return Xapian::Remote::open("../bin/omprogsrv", args);
}

Xapian::WritableDatabase
BackendManager::getwritedb_remote(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable remote database");
}
#endif

#ifdef MUS_BUILD_BACKEND_MUSCAT36
Xapian::Database
BackendManager::getdb_da(const vector<string> &dbnames)
{
    string parent_dir = ".da";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makeda (yum)
	index_files_to_m36("makeDA", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_da(dbdir + "/R", dbdir + "/T",
				     dbdir + "/keyfile");
}

Xapian::Database
BackendManager::getdb_daflimsy(const vector<string> &dbnames)
{
    string parent_dir = ".daflimsy";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makeda (yum)
	index_files_to_m36("makeDAflimsy", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_da(dbdir + "/R", dbdir + "/T",
				     dbdir + "/keyfile", false);
}

Xapian::WritableDatabase
BackendManager::getwritedb_da(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable da database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_daflimsy(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable daflimsy database");
}

Xapian::Database
BackendManager::getdb_db(const vector<string> &dbnames)
{
    string parent_dir = ".db";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makedb (yum)
	index_files_to_m36("makeDB", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_db(dbdir + "/DB", dbdir + "/keyfile");
}

Xapian::Database
BackendManager::getdb_dbflimsy(const vector<string> &dbnames)
{
    string parent_dir = ".dbflimsy";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    // should autodetect flimsy - don't specify to test this
    if (create_dir_if_needed(dbdir)) {
	// directory was created, so do the indexing.
	// need to build temporary file and run it through makedb (yum)
	index_files_to_m36("makeDBflimsy", dbdir, change_names_to_paths(dbnames));
    }
    return Xapian::Muscat36::open_db(dbdir + "/DB", dbdir + "/keyfile");
}

Xapian::WritableDatabase
BackendManager::getwritedb_db(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable db database");
}

Xapian::WritableDatabase
BackendManager::getwritedb_dbflimsy(const vector<string> &/*dbnames*/)
{
    throw Xapian::InvalidArgumentError("Attempted to open writable dbflimsy database");
}
#endif

Xapian::Database
BackendManager::get_database(const vector<string> &dbnames)
{
    return (this->*do_getdb)(dbnames);
}

Xapian::Database
BackendManager::get_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getdb)(dbnames);
}

Xapian::WritableDatabase
BackendManager::get_writable_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getwritedb)(dbnames);
}
