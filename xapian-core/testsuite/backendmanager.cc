/* backendmanager.cc: manage backends for testsuite
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

#include "config.h"
#include <fstream>
#include <string>
#include "autoptr.h"
#include <map>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>

#include "om/om.h"
#include "textfile_indexer.h"
#include "../indexer/index_utils.h"
#include "backendmanager.h"
#include "omdebug.h"

OmDocumentContents
string_to_document(std::string paragraph)
{
    OmStem stemmer("english");

    OmDocumentContents document;
    document.data = OmData(paragraph);
    om_termcount position = 1;

    for (om_keyno i=1; i<=10; ++i) {
	if (i >= paragraph.length()) {
	    break;
	} else {
	    OmKey key;
	    key.value = paragraph.substr(i, 1);
	    document.keys[i] = key;
	}
    }

    std::string::size_type spacepos;
    om_termname word;
    while((spacepos = paragraph.find_first_not_of(" \t\n")) != std::string::npos) {
	if(spacepos) paragraph = paragraph.erase(0, spacepos);
	spacepos = paragraph.find_first_of(" \t\n");
	word = paragraph.substr(0, spacepos);
	select_characters(word, "");
	lowercase_term(word);
	word = stemmer.stem_word(word);
	if(word.size() != 0) {
	    document.add_posting(word, position++);
	}
	paragraph = paragraph.erase(0, spacepos);
    }

    return document;
}

void
index_files_to_database(OmWritableDatabase & database,
                        std::vector<std::string> paths)
{
    for (std::vector<std::string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	AutoPtr<std::istream> from(source.get_stream());

	while(*from) {
	    std::string para;
	    get_paragraph(*from, para);
	    database.add_document(string_to_document(para));
	}
    }
}

void
index_files_to_m36(const string &prog, const string &dbdir,
		   std::vector<std::string> paths)
{
    string dump = dbdir + "/DATA";
    ofstream out(dump.c_str());
    string keyfile = dbdir + "/keyfile";
    ofstream keys(keyfile.c_str());
    keys << "omrocks!"; // magic word
    for (std::vector<std::string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	AutoPtr<std::istream> from(source.get_stream());

	while (*from) {
	    std::string para;
	    get_paragraph(*from, para);
	    OmDocumentContents doc = string_to_document(para);
	    out << "#RSTART#\n" << doc.data.value << "\n#REND#\n#TSTART#\n";
	    OmDocumentContents::document_terms::const_iterator i;
	    for (i = doc.terms.begin(); i != doc.terms.end(); i++) {
		out << i->second.tname << endl;
	    }
	    out << "#TEND#\n";
	    OmDocumentContents::document_keys::const_iterator key_i;
	    key_i = doc.keys.begin();
	    string key = string("\0\0\0\0\0\0\0", 8);
	    if (key_i != doc.keys.end()) key = key_i->second.value + key;
	    key = key.substr(0, 8);
	    keys << key;
	}
    }
    out.close();
    string cmd = "../../makeda/" + prog + " -source " + dump +
	" -da " + dbdir + "/ -work " + dbdir + "/tmp- > /dev/null";
    system(cmd.c_str());
    unlink(dump.c_str());
}

std::vector<std::string>
make_strvec(std::string s1 = "",
	    std::string s2 = "",
	    std::string s3 = "",
	    std::string s4 = "")
{
    std::vector<std::string> result;

    if(s1 != "") result.push_back(s1);
    if(s2 != "") result.push_back(s2);
    if(s3 != "") result.push_back(s3);
    if(s4 != "") result.push_back(s4);

    return result;
}

void
index_file_to_database(OmWritableDatabase & database, std::string path)
{
    index_files_to_database(database, make_strvec(path));
}

void
BackendManager::set_dbtype(const std::string &type)
{
    if (type == "inmemory") {
	do_getdb = &BackendManager::getdb_inmemory;
	do_getwritedb = &BackendManager::getwritedb_inmemory;
    } else if (type == "sleepycat") {
	do_getdb = &BackendManager::getdb_sleepycat;
	do_getwritedb = &BackendManager::getwritedb_sleepycat;
	system("rm -fr .sleepycat");
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
		"Expected inmemory, sleepycat, quartz, remote, da, db, "
		"daflimsy, dbflimsy, or void");
    }
}

void
BackendManager::set_datadir(const std::string &datadir_)
{
    datadir = datadir_;
}

OmDatabase
BackendManager::getdb_void(const std::vector<std::string> &)
{
    throw OmInvalidArgumentError("Attempted to open a disabled database");
}

OmWritableDatabase
BackendManager::getwritedb_void(const std::vector<std::string> &)
{
    throw OmInvalidArgumentError("Attempted to open a disabled database");
}

std::vector<std::string>
BackendManager::change_names_to_paths(const std::vector<std::string> &dbnames)
{
    std::vector<std::string> paths;
    for(std::vector<std::string>::const_iterator i = dbnames.begin();
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
BackendManager::getdb_inmemory(const std::vector<std::string> &dbnames)
{
    return getwritedb_inmemory(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_inmemory(const std::vector<std::string> &dbnames)
{
    OmSettings params;
    params.set("backend", "inmemory");
    OmWritableDatabase db(params);
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const std::string &dirname)
{
    // create a directory for sleepycat indexes if not present
    struct stat sbuf;
    int result = stat(dirname.c_str(), &sbuf);
    if (result < 0) {
	if (errno == ENOENT) {
	    if (mkdir(dirname.c_str(), 0700) < 0) {
		throw OmOpeningError("Can't create directory");
	    }
	} else {
	    throw OmOpeningError("Can't stat directory");
	}
	return true; // either threw an exception, or created a directory.
    } else {
	if (!S_ISDIR(sbuf.st_mode)) {
	    throw OmOpeningError("Is not a directory.");
	}
	return false; // Already a directory.
    }
}

OmWritableDatabase
BackendManager::do_getwritedb_sleepycat(const std::vector<std::string> &dbnames,
					bool writable)
{
    std::string parent_dir = ".sleepycat";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    // add 'w' to distinguish readonly dbs (which can be reused) from
    // writable ones (which need to be recreated on each use)
    if (writable) dbdir += 'w';
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (writable) {
	// if the database is opened readonly, we can reuse it, but if it's
	// writable we need to start afresh each time
	std::string cmd = "rm -fr " + dbdir;
	system(cmd.c_str());
    }
    OmSettings params;
    params.set("backend", "sleepycat");
    params.set("sleepycat_dir", dbdir);
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    OmWritableDatabase db(params);
	    index_files_to_database(db, change_names_to_paths(dbnames));
	    // sleepycat needs to be closed and reopened after a write so
	    // let db go out of scope (and close) and reopen below
	}
    }
    return OmWritableDatabase(params);
}

OmDatabase
BackendManager::getdb_sleepycat(const std::vector<std::string> &dbnames)
{
    return do_getwritedb_sleepycat(dbnames, false);
}

OmWritableDatabase
BackendManager::getwritedb_sleepycat(const std::vector<std::string> &dbnames)
{
    return do_getwritedb_sleepycat(dbnames, true);
}

OmWritableDatabase
BackendManager::do_getwritedb_quartz(const std::vector<std::string> &dbnames,
				     bool writable)
{
    std::string parent_dir = ".quartz";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    // add 'w' to distinguish readonly dbs (which can be reused) from
    // writable ones (which need to be recreated on each use)
    if (writable) dbdir += 'w';
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if (writable) {
	// if the database is opened readonly, we can reuse it, but if it's
	// writable we need to start afresh each time
	std::string cmd = "rm -fr " + dbdir;
	system(cmd.c_str());
    }
    OmSettings params;
    params.set("backend", "quartz");
    params.set("quartz_dir", dbdir);
    if (files_exist(change_names_to_paths(dbnames))) {
	if (create_dir_if_needed(dbdir)) {
	    // directory was created, so do the indexing.
	    OmWritableDatabase db(params);
	    index_files_to_database(db, change_names_to_paths(dbnames));
	    return db;
	}
    }
    return OmWritableDatabase(params);
}

OmDatabase
BackendManager::getdb_quartz(const std::vector<std::string> &dbnames)
{
    return do_getwritedb_quartz(dbnames, false);
}

OmWritableDatabase
BackendManager::getwritedb_quartz(const std::vector<std::string> &dbnames)
{
    return do_getwritedb_quartz(dbnames, true);
}

OmDatabase
BackendManager::getdb_network(const std::vector<std::string> &dbnames)
{
    // run an omprogsrv for now.  Later we should also use omtcpsrv
    std::vector<std::string> args;
    args.push_back(datadir);
    args.insert(args.end(), dbnames.begin(), dbnames.end());

    OmSettings params;
    params.set("backend", "remote");
    params.set("remote_type", "prog");
    params.set("remote_program", "../netprogs/omprogsrv");
    params.set("remote_args", args.begin(), args.end());
    OmDatabase db(params);

    return db;
}

OmWritableDatabase
BackendManager::getwritedb_network(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable network database");
}

OmDatabase
BackendManager::getdb_da(const std::vector<std::string> &dbnames)
{
    std::string parent_dir = ".da";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
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
BackendManager::getdb_daflimsy(const std::vector<std::string> &dbnames)
{
    std::string parent_dir = ".daflimsy";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
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
BackendManager::getwritedb_da(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable da database");
}

OmWritableDatabase
BackendManager::getwritedb_daflimsy(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable daflimsy database");
}

OmDatabase
BackendManager::getdb_db(const std::vector<std::string> &dbnames)
{
    std::string parent_dir = ".db";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
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
BackendManager::getdb_dbflimsy(const std::vector<std::string> &dbnames)
{
    std::string parent_dir = ".dbflimsy";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
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
BackendManager::getwritedb_db(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable db database");
}

OmWritableDatabase
BackendManager::getwritedb_dbflimsy(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable dbflimsy database");
}

OmDatabase
BackendManager::get_database(const std::vector<std::string> &dbnames)
{
    return (this->*do_getdb)(dbnames);
}

OmDatabase
BackendManager::get_database(const std::string &dbname1,
			     const std::string &dbname2)
{
    std::vector<std::string> dbnames;
    dbnames.push_back(dbname1);
    dbnames.push_back(dbname2);
    return (this->*do_getdb)(dbnames);
}

OmWritableDatabase
BackendManager::get_writable_database(const std::string &dbname)
{
    std::vector<std::string> dbnames;
    dbnames.push_back(dbname);
    return (this->*do_getwritedb)(dbnames);
}
