/* da_database.cc: C++ class for datype access routines
 * 
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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
 * -----END-LICENCE----- */

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>

#include "database.h"
#include "dbpostlist.h"
#include "termlist.h"
#include "da_database.h"
#include "da_record.h"
#include "daread.h"
#include "damuscat.h"

DAPostList::DAPostList(const termname & tname_,
		       struct postings * postlist_,
		       doccount termfreq_)
	: postlist(postlist_), currdoc(0), tname(tname_), termfreq(termfreq_)
{
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
    Assert(ir_wt != NULL);

    // NB ranges from daread share the same wdf value
    return ir_wt->get_weight(postlist->wdf, 1.0);
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

PostList * DAPostList::skip_to(docid did, weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    Assert(did >= currdoc);
    if (currdoc && did <= docid(postlist->E)) {
	// skip_to later in the current range
	currdoc = did;
	//cout << "Skip within range " << did << endl;
	return NULL;
    }
    //printf("%p:From %d skip_to ", this, currdoc);
    DAreadpostings(postlist, 1, did);
    currdoc = docid(postlist->Doc);
    //printf("%d - get_id %d\n", did, currdoc);
    return NULL;
}



DATermList::DATermList(struct termvec *tv, doccount dbsize_)
	: have_started(false), dbsize(dbsize_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    readterms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;

	doccount freq = tv->freq;
	terms.push_back(DATermListItem(string(term + 1, (unsigned)term[0] - 1),
				       tv->wdf, freq));
	readterms(tv);
    }
    losetermvec(tv);

    pos = terms.begin();
}

OMExpandBits
DATermList::get_weighting() const
{
    Assert(!at_end());
    Assert(have_started);
    Assert(wt != NULL);

    return wt->get_bits(pos->wdf, 1.0, pos->termfreq, dbsize);
}




DADatabase::DADatabase()
{
    DA_r = NULL;
    DA_t = NULL;
    opened = false;
}

DADatabase::~DADatabase()
{
    if(DA_r != NULL) {
	DAclose(DA_r);
	DA_r = NULL;
    }
    if(DA_t != NULL) {
	DAclose(DA_t);
	DA_t = NULL;
    }
}

void
DADatabase::open(const DatabaseBuilderParams & params)
{
    Assert(!opened);

    // Check validity of parameters
    Assert(params.readonly == true);
    Assert(params.paths.size() == 1);
    Assert(params.subdbs.size() == 0);

    // Open database with specified path
    string filename_r = params.paths[0] + "/R";
    string filename_t = params.paths[0] + "/T";

    DA_r = DAopen(filename_r.c_str(), DARECS);
    if(DA_r == NULL)
	throw OpeningError(string("When opening ") + filename_r + ": " + strerror(errno));

    DA_t = DAopen(filename_t.c_str(), DATERMS);
    if(DA_t == NULL) {
	DAclose(DA_r);
	DA_r = NULL;
	throw OpeningError(string("When opening ") + filename_t + ": " + strerror(errno));
    }

    opened = true;

    return;
}

// Returns a new posting list, for the postings in this database for given term
DBPostList *
DADatabase::open_post_list(const termname & tname, RSet * rset) const
{
    Assert(opened);

    // Make sure the term has been looked up
    const DATerm * the_term = term_lookup(tname);
    Assert(the_term != NULL);

    struct postings * postlist;
    postlist = DAopenpostings(the_term->get_ti(), DA_t);

    DBPostList * pl = new DAPostList(tname, postlist, the_term->get_ti()->freq);
    return pl;
}

// Returns a new term list, for the terms in this database for given document
DBTermList *
DADatabase::open_term_list(docid did) const
{
    Assert(opened);

    struct termvec *tv = maketermvec();
    int found = DAgettermvec(DA_r, did, tv);

    if(found == 0) {
	losetermvec(tv);
	throw RangeError("Docid not found");
    }

    openterms(tv);

    DATermList *tl = new DATermList(tv, DADatabase::get_doccount());
    return tl;
}

struct record *
DADatabase::get_record(docid did) const
{
    Assert(opened);

    struct record *r = makerecord();
    int found = DAgetrecord(DA_r, did, r);

    if(found == 0) {
	loserecord(r);
	throw RangeError("Docid not found");
    }

    return r;
}

IRDocument *
DADatabase::open_document(docid did) const
{
    Assert(opened);

    return new DADocument(this, did);
}

const DATerm *
DADatabase::term_lookup(const termname & tname) const
{
    Assert(opened);
    //DebugMsg("DADatabase::term_lookup(`" << tname.c_str() << "'): ");

    map<termname, DATerm>::const_iterator p = termmap.find(tname);

    const DATerm * the_term = NULL;
    if (p == termmap.end()) {
	int len = tname.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw OmError(strerror(ENOMEM));
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len);

	struct terminfo ti;
	int found = DAterm(k, &ti, DA_t);
	free(k);

	if(found == 0) {
	    DebugMsg("Not in collection" << endl);
	} else {
	    DebugMsg("found, adding to cache" << endl);
	    pair<termname, DATerm> termpair(tname, DATerm(&ti, tname));
	    termmap.insert(termpair);
	    the_term = &(termmap.find(tname)->second);
	}
    } else {
	the_term = &((*p).second);
	DebugMsg("found in cache" << endl);
    }
    return the_term;
}

bool
DADatabase::term_exists(const termname & tname) const
{
    if(term_lookup(tname) != NULL) return true;
    return false;
}
