/* backendmanager.cc - manage backends for testsuite
 *
 * ----START-LICENCE----
 * Copyright 2000 Dialog Corporation
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

OmDocumentContents
string_to_document(string paragraph)
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

    string::size_type spacepos;
    om_termname word;
    while((spacepos = paragraph.find_first_not_of(" \t\n")) != string::npos) {
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
index_files_to_database(OmWritableDatabase & database, vector<string> paths)
{
    for (vector<string>::const_iterator p = paths.begin();
	 p != paths.end();
	 p++) {
	TextfileIndexerSource source(*p);
	auto_ptr<istream> from(source.get_stream());

	while(*from) {
	    string para;
	    get_paragraph(*from, para);
	    database.add_document(string_to_document(para));
	}
    }
}

vector<string>
make_strvec(string s1 = "",
	    string s2 = "",
	    string s3 = "",
	    string s4 = "")
{
    vector<string> result;

    if(s1 != "") result.push_back(s1);
    if(s2 != "") result.push_back(s2);
    if(s3 != "") result.push_back(s3);
    if(s4 != "") result.push_back(s4);

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
    if (type == "inmemory") {
	do_getdb = &getdb_inmemory;
    } else if (type == "sleepycat") {
	do_getdb = &getdb_sleepy;
	cout << "Removing .sleepy/..." << endl;
	system("rm -fr .sleepy");
    } else if (type == "net") {
	do_getdb = &getdb_net;
    } else if (type == "void") {
	do_getdb = &getdb_void;
    } else {
	throw OmInvalidArgumentError("Expected inmemory or sleepy");
    }
}

void
BackendManager::set_datadir(const string &datadir_)
{
    datadir = datadir_;
}

OmDatabase
BackendManager::getdb_void(const string &, const string &)
{
    throw OmInvalidArgumentError("Attempted to open a disabled database");
}

OmDatabase
BackendManager::getdb_inmemory(const string &dbname1,
					  const string &dbname2)
{
    OmWritableDatabase db("inmemory", make_strvec());
    index_file_to_database(db, datadir + "/" + dbname1 + ".txt");
    if (dbname2.length() > 0) {
	index_file_to_database(db, datadir + "/" + dbname2 + ".txt");
    }

    return db;
}

/** Create the directory dirname if needed.  Returns true if the
 *  directory was created and false if it was already there.  Throws
 *  an exception if there was an error (eg not a directory).
 */
bool create_dir_if_needed(const string &dirname)
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

/** Return true if the file fname exists.
 */
bool file_exists(const string &fname)
{
    // create a directory for sleepy indexes if not present
    struct stat sbuf;
    int result = stat(fname.c_str(), &sbuf);
    if (result < 0) {
	return false;
    } else {
	if (!S_ISREG(sbuf.st_mode)) {
	    return false;
	}
	return true;
    }
}

OmDatabase BackendManager::getdb_sleepy(const string &dbname1,
					const string &dbname2)
{
    string parent_dir = ".sleepy";
    create_dir_if_needed(parent_dir);

    string dbdir = parent_dir + "/" + dbname1 + "#" + dbname2;

    if (file_exists(datadir + "/" + dbname1 + ".txt") &&
	((dbname2.length() == 0) ||
	 file_exists(datadir + "/" + dbname2 + ".txt"))) {
	bool created = create_dir_if_needed(dbdir);

	if (created) {
	    // directory was created, so do the indexing.
	    OmWritableDatabase db("sleepycat", make_strvec(dbdir));
	    index_file_to_database(db, datadir + "/" + dbname1 + ".txt");
	    if (dbname2.length() > 0) {
		index_file_to_database(db, datadir + "/" + dbname2 + ".txt");
	    }
	    return db;
	} else {
	    // else just return a read-only db.
	    return OmDatabase("sleepycat", make_strvec(dbdir));
	}
    } else {
	// open a non-existant database
	return OmDatabase("sleepycat", make_strvec(dbdir));
    }
}

OmDatabase BackendManager::getdb_net(const string &dbname1,
				     const string &dbname2)
{
    // run an omprogsrv for now.  Later we should also use omtcpsrv
    vector<string> args;
    args.push_back("prog");
    args.push_back("../netprogs/omprogsrv");
    args.push_back(datadir + "/" + dbname1 + ".txt");
    if (dbname2.length() > 0) {
	args.push_back(datadir + "/" + dbname2 + ".txt");
    }
    OmDatabase db("net", args);

    return db;
}

OmDatabase BackendManager::get_database(const string &dbname1,
					const string &dbname2)
{
    return (this->*do_getdb)(dbname1, dbname2);
}
