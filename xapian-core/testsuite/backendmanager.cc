/* backendmanager.cc - manage backends for testsuite
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
#include <iostream>
#include <string>
#include <memory>
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
	std::auto_ptr<std::istream> from(source.get_stream());

	while(*from) {
	    std::string para;
	    get_paragraph(*from, para);
	    database.add_document(string_to_document(para));
	}
    }
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
	do_getdb = &BackendManager::getdb_sleepy;
	do_getwritedb = &BackendManager::getwritedb_sleepy;
	DebugMsg("Removing .sleepy/..." << endl);
	system("rm -fr .sleepy");
    } else if (type == "net") {
	do_getdb = &BackendManager::getdb_net;
	do_getwritedb = &BackendManager::getwritedb_net;
    } else if (type == "void") {
	do_getdb = &BackendManager::getdb_void;
	do_getwritedb = &BackendManager::getwritedb_void;
    } else {
	throw OmInvalidArgumentError("Expected inmemory, sleepy, net or void");
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
    OmWritableDatabase db("inmemory", make_strvec());
    index_files_to_database(db, change_names_to_paths(dbnames));

    return db;
}

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const std::string &dirname)
{
    // create a directory for sleepy indexes if not present
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

OmDatabase
BackendManager::getdb_sleepy(const std::vector<std::string> &dbnames)
{
    return getwritedb_sleepy(dbnames);
}

OmWritableDatabase
BackendManager::getwritedb_sleepy(const std::vector<std::string> &dbnames)
{
    std::string parent_dir = ".sleepy";
    create_dir_if_needed(parent_dir);

    std::string dbdir = parent_dir + "/db";
    for (std::vector<std::string>::const_iterator i = dbnames.begin();
	 i != dbnames.end();
	 i++) {
	dbdir += "=" + *i;
    }
    if(files_exist(change_names_to_paths(dbnames))) {
	bool created = create_dir_if_needed(dbdir);

	if (created) {
	    // directory was created, so do the indexing.
	    OmWritableDatabase db("sleepycat", make_strvec(dbdir));
	    index_files_to_database(db, change_names_to_paths(dbnames));
	    return db;
	} else {
	    // else just return a read-only db.
	    return OmWritableDatabase("sleepycat", make_strvec(dbdir));
	}
    } else {
	// open a non-existant database
	return OmWritableDatabase("sleepycat", make_strvec(dbdir));
    }
}

OmDatabase
BackendManager::getdb_net(const std::vector<std::string> &dbnames)
{
    // run an omprogsrv for now.  Later we should also use omtcpsrv
    std::vector<std::string> args;
    args.push_back("prog");
    args.push_back("../netprogs/omprogsrv");
    args.push_back(datadir);
    args.insert(args.end(), dbnames.begin(), dbnames.end());
    OmDatabase db("net", args);

    return db;
}

OmWritableDatabase
BackendManager::getwritedb_net(const std::vector<std::string> &dbnames)
{
    throw OmInvalidArgumentError("Attempted to open writable network database");
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

