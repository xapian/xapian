/* multi_database.cc: interface to multiple database access */

#include <stdio.h>

#include "omassert.h"
#include "multi_database.h"
#include "database_factory.h"

#include <string>
#include <vector>
#include <list>

//////////////
// Postlist //
//////////////

MultiPostList::MultiPostList(list<MultiPostListInternal> &pls)
	: postlists(pls), finished(false), currdoc(0),
	  freq_initialised(false)
{
}


MultiPostList::~MultiPostList()
{
    // Will be empty if we've got to the end of all lists,
    // but we might not have, so remove anything left.
    while(postlists.begin() != postlists.end()) {
	delete (*postlists.begin()).pl;
	postlists.erase(postlists.begin());
    }
}

weight MultiPostList::get_weight() const
{
    Assert(freq_initialised);

    weight wt = 0;
    list<MultiPostListInternal>::const_iterator i = postlists.begin();
    while(i != postlists.end()) {
	if((*i).currdoc == currdoc)
	    wt += (*i).pl->get_weight();
	i++;
    }
    return wt;
}

PostList * MultiPostList::next(weight w_min)
{
    Assert(!at_end());

    list<MultiPostListInternal>::iterator i = postlists.begin();
    while(i != postlists.end()) {
	// Check if it needs to be advanced
	if(currdoc >= (*i).currdoc) {
	    (*i).pl->next(w_min);
	    if((*i).pl->at_end()) {
		// Close sub-postlist
		delete (*i).pl;
		list<MultiPostListInternal>::iterator erase_iter = i;
		i++;
		postlists.erase(erase_iter);
		// erase iter is now invalid, but i is still valid because
		// i) this is a list, and ii) i wasn't pointing to a deleted
		// entry
		continue;
	    }
	    (*i).currdoc = ((*i).pl->get_docid() - 1) * (*i).multiplier +
		           (*i).offset;
	}
	i++;
    }
    if(postlists.size() == 0) {
	finished = true;
	return NULL;
    }

    i = postlists.begin();
    docid newdoc = (*i).currdoc;
    for(i++; i != postlists.end(); i++) {
	// Check if it might be the newdoc
	if((*i).currdoc < newdoc) newdoc = (*i).currdoc;
    }

    currdoc = newdoc;

    return NULL;
}

// FIXME - write implementation to use skip_to methods of sub-postlists
// for greater efficiency
PostList *
MultiPostList::skip_to(docid did, weight w_min)
{
    Assert(!at_end());
    while (!at_end() && currdoc < did) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
}

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
    close();
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
MultiDatabase::open(om_database_type type,
		    const string &pathname,
		    bool readonly) {
    Assert(!used);

    DatabaseFactory dbfact;
    IRSingleDatabase *db = dbfact.make(type);

    db->open(pathname, readonly);
    db->set_root(root);

    databases.push_back(db);

    opened = true;
}


void MultiDatabase::close() {
    if(opened) {
	// Close all databases
	while(databases.begin() != databases.end()) {
	    (*databases.begin())->close();
	    delete *databases.begin();
	    databases.erase(databases.begin());
	}
    }
    opened = false;
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

TermList *
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

termid
MultiDatabase::term_name_to_id(const termname &tname) const {
    Assert(false);
}

bool
MultiDatabase::term_exists(const termname &tname) const
{
    Assert(opened);
    Assert((used = true) == true);

    printf("Looking up term `%s': ", tname.c_str());
    set<termname>::const_iterator p = terms.find(tname);

    bool found = false;

    if (p == terms.end()) {
	printf("Looking through sub-databases:");

	vector<IRDatabase *>::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    found = (*i)->term_exists(tname);
	    if(found) break;
	    i++;
	}

	if(found) {
	    cout << "Found -- adding to cache" << endl;
	    terms.insert(tname);
	} else {
	    cout << "Not in collection" << endl;
	}
    } else {
	found = true;
	cout << "found" << endl;
    }
    return found;
}

IRDatabase *
MultiDatabase::get_database_of_doc(docid did) const
{
    Assert(opened);
    doccount multiplier = databases.size();

    return databases[(did - 1) % multiplier];
}
