/* backendmanager.cc: manage backends for testsuite
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
#include <fstream>
#include <string>
using std::string;
#include <vector>
using std::vector;
#include "autoptr.h"
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include "om/om.h"
#include "textfile_indexer.h"
#include "index_utils.h"
#include "backendmanager.h"
#include "omdebug.h"
#include "utils.h"

OmDocument
string_to_document(string paragraph)
{
    OmStem stemmer("english");

    OmDocument document;
    document.set_data(paragraph);
    om_termcount position = 1;

    for (om_valueno i = 1; i <= 10; ++i) {
	if (i >= paragraph.length()) {
	    break;
	} else {
	    string value = paragraph.substr(i, 1);
	    document.add_value(i, value);
	}
    }
    {
	string value;

	/* We need a value which will be useful for collapsing with DA
	 * databases, where only the first 8 bytes of value 0 count.
	 */
	value = paragraph[2];

	value += string("\0\0\0 \1\t", 6);

	for (int k = 0; k < 256; k++) {
	    value += (char)(k);
	}
	value += paragraph;
	document.add_value(0, value);
    }

    string::size_type spacepos;
    om_termname word;
    while((spacepos = paragraph.find_first_not_of(" \t\n")) != string::npos) {
	if(spacepos) paragraph = paragraph.erase(0, spacepos);
	spacepos = paragraph.find_first_of(" \t\n");
	word = paragraph.substr(0, spacepos);
	select_characters(word, "");
	lowercase_term(word);
	word = stemmer.stem_word(word);
	if (word.size() != 0) {
	    document.add_posting(word, position++);
	}
	paragraph = paragraph.erase(0, spacepos);
    }

    return document;
}

void
index_files_to_database(OmWritableDatabase & database,
                        vector<string> paths)
{
    for (vector<string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	AutoPtr<std::istream> from(source.get_stream());

	while(*from) {
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
    std::ofstream out(dump.c_str());
    string valuefile = dbdir + "/keyfile";
    std::ofstream values(valuefile.c_str());
    values << "omrocks!"; // magic word
    for (vector<string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	AutoPtr<std::istream> from(source.get_stream());

	while (*from) {
	    string para;
	    get_paragraph(*from, para);
	    OmDocument doc = string_to_document(para);
	    out << "#RSTART#\n" << doc.get_data() << "\n#REND#\n#TSTART#\n";
	    {
		OmTermIterator i = doc.termlist_begin();
		OmTermIterator i_end = doc.termlist_end();
		for ( ; i != i_end; ++i) {
		    out << *i << std::endl;
		}
	    }
	    out << "#TEND#\n";
	    OmValueIterator value_i = doc.values_begin();
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
make_strvec(string s1 = "",
	    string s2 = "",
	    string s3 = "",
	    string s4 = "")
{
    vector<string> result;

    if (!s1.empty()) result.push_back(s1);
    if (!s2.empty()) result.push_back(s2);
    if (!s3.empty()) result.push_back(s3);
    if (!s4.empty()) result.push_back(s4);

    return result;
}

void
index_file_to_database(OmWritableDatabase & database, string path)
{
    index_files_to_database(database, make_strvec(path));
}

void
BackendManager::set_dbtype(const string &type)
{
    if (type == current_type) {
	// leave it as it is.
    } else if (type == "inmemory") {
	do_getdb = &BackendManager::getdb_inmemory;
	do_getwritedb = &BackendManager::getwritedb_inmemory;
    } else if (type == "inmemoryerr") {
	do_getdb = &BackendManager::getdb_inmemoryerr;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr;
    } else if (type == "inmemoryerr2") {
	do_getdb = &BackendManager::getdb_inmemoryerr2;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr2;
    } else if (type == "inmemoryerr3") {
	do_getdb = &BackendManager::getdb_inmemoryerr3;
	do_getwritedb = &BackendManager::getwritedb_inmemoryerr3;
    } else if (type == "quartz") {
	do_getdb = &BackendManager::getdb_quartz;
	do_getwritedb = &BackendManager::getwritedb_quartz;
	system("rm -fr .quartz");
    } else if (type == "remote") {
	do_getdb = &BackendManager::getdb_network;
	do_getwritedb = &BackendManager::getwritedb_network;
    } else if (type == "da") {
	do_getdb = &BackendManager::getdb_da;
	do_getwritedb = &BackendManager::getwritedb_da;
	system("rm -fr .da");
    } else if (type == "db") {
	do_getdb = &BackendManager::getdb_db;
	do_getwritedb = &BackendManager::getwritedb_db;
	system("rm -fr .db");
    } else if (type == "daflimsy") {
	do_getdb = &BackendManager::getdb_daflimsy;
	do_getwritedb = &BackendManager::getwritedb_daflimsy;
	system("rm -fr .daflimsy");
    } else if (type == "dbflimsy") {
	do_getdb = &BackendManager::getdb_dbflimsy;
	do_getwritedb = &BackendManager::getwritedb_dbflimsy;
	system("rm -fr .dbflimsy");
    } else if (type == "void") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
    } else {
	throw OmInvalidArgumentError(
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

const std::string
BackendManager::get_datadir(void)
{
    return datadir;
}

OmDatabase
BackendManager::getdb_void(const vector<string> &)
{
    throw OmInvalidArgumentError("Attempted to open a disabled database");
}

OmWritableDatabase
BackendManager::getwritedb_void(const vector<string> &)
{
    throw OmInvalidArgumentError("Attempted to open a disabled database");
}

vector<string>
BackendManager::change_names_to_paths(const vector<string> &dbnames)
{
    vector<string> paths;
    for(vector<string>::const_iterator i = dbnames.begin();
	i != dbnames.end();
	i++) {
	if (i->length() > 0) {
	    if(datadir.size() == 0) {
		paths.push_back(*i);
	    } else {
		paths.push_back(datadir + "/" + *i + ".txt");
	    }
	}
    }
    return paths;
}

OmDatabase
BackendManager::getdb_inmemory(const vector<string> &dbnames)
{
    return getwritedb_inmemory(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_inmemory(const vector<string> &dbnames)
{
    OmSettings params;
    params.set("backend", "inmemory");
    OmWritableDatabase db(params);
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

OmDatabase
BackendManager::getdb_inmemoryerr(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_inmemoryerr(const vector<string> &dbnames)
{
    OmSettings params;
    params.set("backend", "inmemory");
    params.set("inmemory_errornext", 1);
    OmWritableDatabase db(params);
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

OmDatabase
BackendManager::getdb_inmemoryerr2(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr2(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_inmemoryerr2(const vector<string> &dbnames)
{
    OmSettings params;
    params.set("backend", "inmemory");
    params.set("inmemory_abortnext", 1);
    OmWritableDatabase db(params);
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

OmDatabase
BackendManager::getdb_inmemoryerr3(const vector<string> &dbnames)
{
    return getwritedb_inmemoryerr3(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_inmemoryerr3(const vector<string> &dbnames)
{
    OmSettings params;
    params.set("backend", "inmemory");
    params.set("inmemory_abortnext", 2);
    OmWritableDatabase db(params);
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

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
	if (errno != ENOENT) throw OmOpeningError("Can't stat directory");
        if (mkdir(dirname, 0700) < 0) {
	    throw OmOpeningError("Can't create directory");
	}
	return true; // Successfully created a directory.
    }
    if (!S_ISDIR(sbuf.st_mode)) throw OmOpeningError("Is not a directory.");
    return false; // Already a directory.
}

OmDatabase
BackendManager::do_getdb_quartz(const vector<string> &dbnames, bool writable)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    // add 'w' to distinguish writable dbs (which need to be recreated on each
    // use) from readonly ones (which can be reused)
    if (writable) dbdir += 'w';
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (writable) {
	// if the database is opened readonly, we can reuse it, but if it's
	// writable we need to start afresh each time
	string cmd = "rm -fr " + dbdir;
	system(cmd);
    }
    OmSettings params;
    params.set("backend", "quartz");
    params.set("quartz_dir", dbdir);
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    OmSettings params1 = params;
	    params1.set("database_create", true);
	    OmWritableDatabase db(params1);
	    index_files_to_database(db, change_names_to_paths(dbnames));
	}
    }
    return OmDatabase(params);
}

OmWritableDatabase
BackendManager::do_getwritedb_quartz(const vector<string> &dbnames,
				bool writable)
{
    string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/db";
    // add 'w' to distinguish readonly dbs (which can be reused) from
    // writable ones (which need to be recreated on each use)
    if (writable) dbdir += 'w';
    for (vector<string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (writable) {
	// if the database is opened readonly, we can reuse it, but if it's
	// writable we need to start afresh each time
	string cmd = "rm -fr " + dbdir;
	system(cmd);
    }
    OmSettings params;
    params.set("backend", "quartz");
    params.set("quartz_dir", dbdir);
    params.set("quartz_logfile", dbdir + "/logfile");
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    OmSettings params1 = params;
	    params1.set("database_create", true);
	    OmWritableDatabase db(params1);
	    index_files_to_database(db, change_names_to_paths(dbnames));
	}
    }
    return OmWritableDatabase(params);
}

OmDatabase
BackendManager::getdb_quartz(const vector<string> &dbnames)
{
    return do_getdb_quartz(dbnames, false);
}

OmWritableDatabase
BackendManager::getwritedb_quartz(const vector<string> &dbnames)
{
    return do_getwritedb_quartz(dbnames, true);
}

OmDatabase
BackendManager::getdb_network(const vector<string> &dbnames)
{
    // run an omprogsrv for now.  Later we should also use omtcpsrv
    string args = datadir;
    vector<string>::const_iterator i;
    for (i=dbnames.begin(); i!=dbnames.end(); ++i) {
	if (*i == "#TIMEOUT#") {
	    ++i;
	    if (i == dbnames.end()) {
		throw OmInvalidArgumentError("Missing timeout parameter");
	    }
	    args += " -t" + *i;
	} else {
	    args += ' ';
	    args += *i;
	}
    }

    OmSettings params;
    params.set("backend", "remote");
    params.set("remote_type", "prog");
    params.set("remote_program", "../netprogs/omprogsrv");
    params.set("remote_args", args);
    OmDatabase db(params);

    return db;
}

OmWritableDatabase
BackendManager::getwritedb_network(const vector<string> &/*dbnames*/)
{
    throw OmInvalidArgumentError("Attempted to open writable network database");
}

OmDatabase
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
    OmSettings params;
    params.set("backend", "da");
    params.set("m36_record_file", dbdir + "/R");
    params.set("m36_term_file", dbdir + "/T");
    params.set("m36_key_file", dbdir + "/keyfile");
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    // need to build temporary file and run it through makeda (yum)
	    index_files_to_m36("makeDA", dbdir, change_names_to_paths(dbnames));
	}
    }
    return OmDatabase(params);
}

OmDatabase
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
    OmSettings params;
    params.set("backend", "da");
    params.set("m36_record_file", dbdir + "/R");
    params.set("m36_term_file", dbdir + "/T");
    params.set("m36_key_file", dbdir + "/keyfile");
    params.set("m36_heavyduty", false);
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    // need to build temporary file and run it through makeda (yum)
	    index_files_to_m36("makeDAflimsy", dbdir, change_names_to_paths(dbnames));
	}
    }
    return OmDatabase(params);
}

OmWritableDatabase
BackendManager::getwritedb_da(const vector<string> &/*dbnames*/)
{
    throw OmInvalidArgumentError("Attempted to open writable da database");
}

OmWritableDatabase
BackendManager::getwritedb_daflimsy(const vector<string> &/*dbnames*/)
{
    throw OmInvalidArgumentError("Attempted to open writable daflimsy database");
}

OmDatabase
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
    OmSettings params;
    params.set("backend", "db");
    params.set("m36_db_file", dbdir + "/DB");
    params.set("m36_key_file", dbdir + "/keyfile");
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    // need to build temporary file and run it through makedb (yum)
	    index_files_to_m36("makeDB", dbdir, change_names_to_paths(dbnames));
	}
    }
    return OmDatabase(params);
}

OmDatabase
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
    OmSettings params;
    params.set("backend", "db");
    params.set("m36_db_file", dbdir + "/DB");
    params.set("m36_key_file", dbdir + "/keyfile");
    // should autodetect flimsy - don't specify to test this
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    // need to build temporary file and run it through makedb (yum)
	    index_files_to_m36("makeDBflimsy", dbdir, change_names_to_paths(dbnames));
	}
    }
    return OmDatabase(params);
}

OmWritableDatabase
BackendManager::getwritedb_db(const vector<string> &/*dbnames*/)
{
    throw OmInvalidArgumentError("Attempted to open writable db database");
}

OmWritableDatabase
BackendManager::getwritedb_dbflimsy(const vector<string> &/*dbnames*/)
{
    throw OmInvalidArgumentError("Attempted to open writable dbflimsy database");
}

OmDatabase
BackendManager::get_database(const vector<string> &dbnames)
{
    return (this->*do_getdb)(dbnames);
}

OmDatabase
BackendManager::get_database(const string &dbname1,
			     const string &dbname2)
{
    vector<string> dbnames;
    dbnames.push_back(dbname1);
    dbnames.push_back(dbname2);
    return (this->*do_getdb)(dbnames);
}

OmWritableDatabase
BackendManager::get_writable_database(const string &dbname)
{
    vector<string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getwritedb)(dbnames);
}
