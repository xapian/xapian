/* multi_database.cc: interface to multiple database access */

#include <stdio.h>

#include "omassert.h"
#include "multi_postlist.h"
#include "multi_termlist.h"
#include "multi_database.h"
#include "database_builder.h"

#include <string>
#include <vector>
#include <list>

///////////////////////////
// Actual database class //
///////////////////////////

MultiDatabase::MultiDatabase()
	: length_initialised(false)
{
    Assert((opened = false) == false);
    Assert((used = false) == false);
}

MultiDatabase::~MultiDatabase() {
    // Close all databases
    while(databases.begin() != databases.end()) {
	delete *(databases.begin());
	databases.erase(databases.begin());
    }
}

void
MultiDatabase::set_root(IRDatabase *db) {
    Assert(!used);
    root = db;

    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	(*i)->set_root(db);
	i++;
    }
}

void
MultiDatabase::open(const DatabaseBuilderParams & params) {
    Assert(!used);

    // Check validity of parameters
    Assert(params.paths.size() == 0);
    Assert(params.subdbs.size() > 0);

    // Loop through all params in subdbs, creating a database for each one,
    // with the specified parameters.  Override some parameters though:
    // If params.readonly is set, everything should be opened readonly

    vector<DatabaseBuilderParams>::const_iterator p;
    for(p = params.subdbs.begin(); p != params.subdbs.end(); p++) {
	DatabaseBuilderParams sub_params = *p;
	if(params.readonly) sub_params.readonly = params.readonly;
	if(params.root != NULL) sub_params.root = params.root;

	databases.push_back(DatabaseBuilder::create(sub_params));
    }

    opened = true;
}

DBPostList *
MultiDatabase::open_post_list(const termname & tname, RSet *rset) const
{
    Assert(opened);
    Assert((used = true) == true);
    Assert(term_exists(tname));

    doccount offset = 1;
    doccount multiplier = databases.size();

    list<MultiPostListInternal> pls;
    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	if((*i)->term_exists(tname)) {
	    MultiPostListInternal pl((*i)->open_post_list(tname, rset),
				     offset, multiplier);
	    pls.push_back(pl);
	}
	offset++;
	i++;
    }
    Assert(pls.begin() != pls.end());
    
    DBPostList * newpl = new MultiPostList(pls);
    return newpl;
}

DBTermList *
MultiDatabase::open_term_list(docid did) const {
    Assert(opened);
    Assert((used = true) == true);

    doccount multiplier = databases.size();

    docid realdid = (did - 1) / multiplier + 1;
    doccount dbnumber = (did - 1) % multiplier;

    TermList *newtl;
    newtl = (*(databases.begin() + dbnumber))->open_term_list(realdid);
    return new MultiTermList(newtl, *(databases.begin() + dbnumber), this);
}

IRDocument *
MultiDatabase::open_document(docid did) const {
    Assert(opened);
    Assert((used = true) == true);

    doccount multiplier = databases.size();

    docid realdid = (did - 1) / multiplier + 1;
    doccount dbnumber = (did - 1) % multiplier;

    return (*(databases.begin() + dbnumber))->open_document(realdid);
}

bool
MultiDatabase::term_exists(const termname &tname) const
{
    Assert(opened);
    Assert((used = true) == true);

    //DebugMsg("MultiDatabase::term_exists(`" << tname.c_str() << "'): ");
    set<termname>::const_iterator p = terms.find(tname);

    bool found = false;

    if (p == terms.end()) {
	vector<IRDatabase *>::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    found = (*i)->term_exists(tname);
	    if(found) break;
	    i++;
	}

	if(found) {
	    //DebugMsg("found in sub-database" << endl);
	    terms.insert(tname);
	} else {
	    //DebugMsg("not in collection" << endl);
	}
    } else {
	found = true;
	//DebugMsg("found in cache" << endl);
    }
    return found;
}
