/* textfile_database.cc: interface to text file access */

#include <stdio.h>

#include "omassert.h"
#include "textfile_database.h"

#include <string>
#include <vector>
#include <map>
#include <list>

/*
//////////////
// Postlist //
//////////////

MultiPostList::MultiPostList(const IRDatabase *db,
			     list<MultiPostListInternal> &pls)
	: postlists(pls), finished(false), currdoc(0),
	  freq_initialised(false)
{
    own_wt.set_stats(db, get_termfreq());
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

weight MultiPostList::get_maxweight() const
{
    Assert(freq_initialised);

    // Should AssertParanoid that all maxweights are the same
    
    return (*postlists.begin()).pl->get_maxweight();
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
*/

///////////////////////////
// Actual database class //
///////////////////////////

TextfileDatabase::TextfileDatabase()
{
    Assert((opened = false) == false);
}

TextfileDatabase::~TextfileDatabase() {
    close();
}

void TextfileDatabase::open(const string &pathname, bool readonly) {
    close();

    // Initialise
    docs = 0;
    avlength = 1.0;
    termvec.clear();
    termidmap.clear();

    // Index document

    opened = true;
}

void TextfileDatabase::close() {
    opened = false;
}

DBPostList *
TextfileDatabase::open_post_list(termid tid) const {
    Assert(opened);

    throw OmError("TextfileDatabase.open_term_list() not yet implemented");
    return NULL;
}

TermList *
TextfileDatabase::open_term_list(docid did) const {
    Assert(opened);

    throw OmError("TextfileDatabase.open_term_list() not yet implemented");
    return NULL;
}

IRDocument *
TextfileDatabase::open_document(docid did) const {
    Assert(opened);

    throw OmError("TextfileDatabase.open_document() not yet implemented");
    return NULL;
}


termid
TextfileDatabase::add_term(const termname &tname) {
    throw OmError("TextfileDatabase.add_term() not implemented");
}

docid
TextfileDatabase::add_doc(IRDocument &doc) {
    throw OmError("TextfileDatabase.add_doc() not implemented");
}

void
TextfileDatabase::add(termid tid, docid did, termpos tpos) {
    throw OmError("TextfileDatabase.add_term() not implemented");
}

termid
TextfileDatabase::term_name_to_id(const termname &tname) const
{
    Assert(opened);

    printf("Looking up term `%s': ", tname.c_str());
    map<termname,termid>::const_iterator p = termidmap.find(tname);

    termid tid = 0;
    if (p == termidmap.end()) {
	tid = termvec.size() + 1;
	termvec.push_back(tname);
	termidmap[tname] = tid;
    } else {
	tid = (*p).second;
    }
    return tid;
}
