/* multi_database.cc: interface to multiple database access */

#include <stdio.h>

#include "omassert.h"
#include "multi_database.h"

#include <string>

///////////////////////////
// Actual database class //
///////////////////////////

MultiDatabase::MultiDatabase() {
    opened = false;
}

MultiDatabase::~MultiDatabase() {
    if(opened) close();
}

void
MultiDatabase::open_subdatabase(IRDatabase * db,
				const string &pathname, bool readonly) {
    db->open(pathname, readonly);
    databases.push_back(db);
    opened = true;
}


void MultiDatabase::close() {
    // Close all databases
    vector<IRDatabase *>::iterator i;
    i = databases.begin();
    while(i != databases.end()) {
	(*i)->close();
	delete *i;
	databases.erase(i);
	i++;
    }
    opened = false;
}

PostList *
MultiDatabase::open_post_list(termid tid) const {
    throw OmError("MultiDatabase.open_post_list() not implemented");
}

TermList *
MultiDatabase::open_term_list(docid tid) const {
    throw OmError("MultiDatabase.open_term_list() not implemented");
}

termid
MultiDatabase::add_term(const termname &tname) {
    throw OmError("MultiDatabase.add_term() not implemented");
}

docid
MultiDatabase::add_doc(IRDocument &doc) {
    throw OmError("MultiDatabase.add_doc() not implemented");
}

void
MultiDatabase::add(termid tid, docid did, termpos tpos) {
    throw OmError("MultiDatabase.add_term() not implemented");
}

termid
MultiDatabase::term_name_to_id(const termname &tname) const {
    Assert(opened);

    printf("Looking up term `%s': ", tname.c_str());
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid id = 0;
    if (p == termidmap.end()) {
	printf("Looking through sub-databases:\n");
	bool found = false;

	if(found) {
	    id = termvec.size() + 1;
	    printf("Adding as ID %d\n", id);
	    termvec.push_back(MultiTerm(tname));
	    termidmap[tname] = id;
	} else {
	    printf("Not in collection\n");
	}
    } else {
	id = (*p).second;
	printf("found, ID %d\n", id);
    }
    return id;
}

termname
MultiDatabase::term_id_to_name(termid tid) const {
    return "";
}
