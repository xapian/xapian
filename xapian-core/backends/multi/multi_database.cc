/* multi_database.cc: interface to multiple database access */

#include <stdio.h>

#include "omassert.h"
#include "multi_database.h"

#include <string>
#include <vector>
#include <list>

//////////////
// Postlist //
//////////////

MultiPostList::MultiPostList(const IRDatabase *db,
			     list<MultiPostListInternal> &pls)
	: postlists(pls), finished(false), currdoc(0),
	  freq_initialised(false)
{
    own_wt.set_stats(db, get_termfreq());

    // Make all the sub-termlists use the same (our) termweight
    set_termweight(&own_wt);
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
    
    docid newdoc = 0;

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
	    (*i).currdoc = (*i).pl->get_docid() * (*i).multiplier + (*i).offset;
	}

	// Check if it might be the newdoc
	if(newdoc > (*i).currdoc || newdoc == 0) newdoc = (*i).currdoc;
	i++;
    }

    currdoc = newdoc;

    if(postlists.begin() == postlists.end())
	finished = true;
    return NULL;
}

// FIXME - implement skip_to() for efficiency

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
MultiDatabase::open_subdatabase(IRDatabase * db,
				const string &pathname, bool readonly) {
    Assert(!used);
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
MultiDatabase::open_post_list(termid tid) const {
    Assert(opened);
    Assert((used = true) == true);

    termname tname = term_id_to_name(tid);

    doccount offset = 0;
    doccount multiplier = databases.size();

    list<MultiPostListInternal> pls;
    vector<IRDatabase *>::const_iterator i = databases.begin();
    while(i != databases.end()) {
	termid local_tid = (*i)->term_name_to_id(tname);
	if(local_tid) {
	    MultiPostListInternal pl((*i)->open_post_list(local_tid),
				     offset, multiplier);
	    pls.push_back(pl);
	}
	offset++;
	i++;
    }
    Assert(pls.begin() != pls.end());
    
    DBPostList * newpl = new MultiPostList(root, pls);
    return newpl;
}

TermList *
MultiDatabase::open_term_list(docid did) const {
    Assert(opened);
    Assert((used = true) == true);

    doccount multiplier = databases.size();

    docid realdid = did / multiplier;
    doccount dbnumber = did % multiplier;

    TermList *newtl;
    newtl = (*(databases.begin() + dbnumber))->open_term_list(realdid);
    return new MultiTermList(newtl, *(databases.begin() + dbnumber), this);
}

IRDocument *
MultiDatabase::open_document(docid did) const {
    Assert(opened);
    Assert((used = true) == true);

    doccount multiplier = databases.size();

    docid realdid = did / multiplier;
    doccount dbnumber = did % multiplier;

    return (*(databases.begin() + dbnumber))->open_document(realdid);
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
    Assert((used = true) == true);

    printf("Looking up term `%s': ", tname.c_str());
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	printf("Looking through sub-databases:");
	bool found = false;

	vector<IRDatabase *>::const_iterator i = databases.begin();
	while(i != databases.end()) {
	    termid thisid = (*i)->term_name_to_id(tname);
	    if(thisid) {
		found = true;
		break;
	    }
	    i++;
	}

	if(found) {
	    tid = termvec.size() + 1;
	    printf("Adding as ID %d\n", tid);
	    termvec.push_back(MultiTerm(tname));
	    termidmap[tname] = tid;
	} else {
	    printf("Not in collection\n");
	}
    } else {
	tid = (*p).second;
	printf("found, ID %d\n", tid);
    }
    return tid;
}
