/* da_database.cc: C++ class for datype access routines */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>

#include "database.h"
#include "da_database.h"
#include "da_record.h"
#include "daread.h"
#include "damuscat.h"

DAPostList::DAPostList(const IRDatabase *db, struct postings *pl, doccount tf)
	: postlist(pl), currdoc(0), termfreq(tf)
{
    own_wt.set_stats(db, tf);
}

DAPostList::~DAPostList()
{
    DAclosepostings(postlist);
}

/* This is the biggie */
weight DAPostList::get_weight() const
{
    Assert(!at_end());
    Assert(currdoc != 0);

    // NB ranges from daread share the same wdf value
    return ir_wt->get_weight(postlist->wdf, 1.0);
}

// return an upper bound on the termweight
weight DAPostList::get_maxweight() const
{
    // FIXME - too much indirection?
    return ir_wt->get_maxweight();
}

PostList * DAPostList::next(weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    if (currdoc && currdoc < docid(postlist->E)) {	
	currdoc++;
	return NULL;
    }
    DAreadpostings(postlist, 1, 0);
    currdoc = docid(postlist->Doc);
    return NULL;
}

PostList * DAPostList::skip_to(docid id, weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    Assert(id >= currdoc);
    if (currdoc && id <= docid(postlist->E)) {
	// skip_to later in the current range
	currdoc = id;
	//cout << "Skip within range " << id << endl;
	return NULL;
    }
    //printf("%p:From %d skip_to ", this, currdoc);
    DAreadpostings(postlist, 1, id);
    currdoc = docid(postlist->Doc);
    //printf("%d - get_id %d\n", id, currdoc);
    return NULL;
}



DATermList::DATermList(const IRDatabase *db, struct termvec *tv)
	: have_started(false)
{
    readterms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;
	termid id;
	// FIXME - Next line is inefficient - checks for term in term list
	id = db->term_name_to_id(string(term + 1, (unsigned)term[0] - 1));

	doccount freq = tv->freq;
	terms.push_back(DATermListItem(id, tv->wdf, freq));
	readterms(tv);
    }
    losetermvec(tv);

    sort(terms.begin(), terms.end());

    pos = terms.begin();
}



DADatabase::DADatabase()
{
    DA_r = NULL;
    DA_t = NULL;
    opened = false;
}

DADatabase::~DADatabase()
{
    close();
}

void
DADatabase::open(const string &pathname, bool readonly)
{
    DADatabase::close();

    string filename_r = pathname + "/R";
    string filename_t = pathname + "/T";

    DA_r = DAopen((byte *)(filename_r.c_str()), DARECS);
    if(DA_r == NULL)
	throw OpeningError(string("When opening ") + filename_r + ": " + strerror(errno));

    DA_t = DAopen((byte *)(filename_t.c_str()), DATERMS);
    if(DA_t == NULL) {
	DAclose(DA_r);
	DA_r = NULL;
	throw OpeningError(string("When opening ") + filename_t + ": " + strerror(errno));
    }

    opened = true;

    return;
}

void
DADatabase::close()
{
    if(DA_r != NULL) {
	DAclose(DA_r);
	DA_r = NULL;
    }
    if(DA_t != NULL) {
	DAclose(DA_t);
	DA_t = NULL;
    }
    opened = false;
}

// Returns a new posting list, for the postings in this database for given term
DBPostList * DADatabase::open_post_list(termid id) const
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());

    termname name = term_id_to_name(id);

    struct postings * postlist;
    postlist = DAopenpostings((terminfo *)&(termvec[id - 1].ti), DA_t);

    DBPostList * pl = new DAPostList(root, postlist, termvec[id - 1].ti.freq);
    return pl;
}

// Returns a new term list, for the terms in this database for given document
TermList * DADatabase::open_term_list(docid id) const
{
    Assert(opened);

    struct termvec *tv = maketermvec();
    int found = DAgettermvec(DA_r, id, tv);

    if(found == 0) {
	losetermvec(tv);
	throw RangeError("Docid not found");
    }

    openterms(tv);

    DATermList *tl = new DATermList(this, tv);
    return tl;
}

IRDocument * DADatabase::open_document(docid id) const
{
    Assert(opened);

    struct record *r = makerecord();
    int found = DAgetrecord(DA_r, id, r);

    if(found == 0) {
	loserecord(r);
	throw RangeError("Docid not found");
    }

    DADocument *rec = new DADocument(r);
    return rec;
}

termid
DADatabase::term_name_to_id(const termname &name) const
{
    Assert(opened);
    //printf("Looking up term `%s': ", name.c_str());

    map<termname,termid>::const_iterator p = termidmap.find(name);

    termid id = 0;
    if (p == termidmap.end()) {
	int len = name.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw OmError(strerror(ENOMEM));
	k[0] = len + 1;
	name.copy((char*)(k + 1), len);

	struct terminfo ti;
	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) {
	    //printf("Not in collection\n");
	} else {
	    id = termvec.size() + 1;
	    //printf("Adding as ID %d\n", id);
	    termvec.push_back(DATerm(&ti, name));
	    termidmap[name] = id;
	}
    } else {
	id = (*p).second;
	//printf("found, ID %d\n", id);
    }
    return id;
}

termname
DADatabase::term_id_to_name(termid id) const
{
    Assert(opened);
    Assert(id > 0 && id <= termvec.size());
    //printf("Looking up termid %d: name = `%s'\n", id, termvec[id - 1].name.c_str());
    return termvec[id - 1].name;
}
