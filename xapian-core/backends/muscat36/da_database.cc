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

#include "utils.h"
#include "database.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "da_database.h"
#include "da_document.h"
#include "daread.h"

DAPostList::DAPostList(const om_termname & tname_,
		       struct DA_postings * postlist_,
		       om_doccount termfreq_)
	: postlist(postlist_), currdoc(0), tname(tname_), termfreq(termfreq_)
{
}

DAPostList::~DAPostList()
{
    DA_close_postings(postlist);
}

om_weight DAPostList::get_weight() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    Assert(ir_wt != NULL);

    // NB ranges from daread share the same wdf value
    return ir_wt->get_sumpart(postlist->wdf, 1.0);
}

PostList * DAPostList::next(om_weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    if (currdoc && currdoc < om_docid(postlist->E)) {	
	currdoc++;
	return NULL;
    }
    DA_read_postings(postlist, 1, 0);
    currdoc = om_docid(postlist->Doc);
    return NULL;
}

PostList * DAPostList::skip_to(om_docid did, om_weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    Assert(did >= currdoc);
    if (currdoc && did <= om_docid(postlist->E)) {
	// skip_to later in the current range
	currdoc = did;
	//DebugMsg("Skip within range " << did << endl);
	return NULL;
    }
    //printf("%p:From %d skip_to ", this, currdoc);
    DA_read_postings(postlist, 1, did);
    currdoc = om_docid(postlist->Doc);
    //printf("%d - get_id %d\n", did, currdoc);
    return NULL;
}



DATermList::DATermList(struct termvec *tv, om_doccount dbsize_)
	: have_started(false), dbsize(dbsize_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;

	om_doccount freq = tv->freq;
	terms.push_back(DATermListItem(string(term + 1, (unsigned)term[0] - 1),
				       tv->wdf, freq));
	M_read_terms(tv);
    }
    M_lose_termvec(tv);

    pos = terms.begin();
}

OmExpandBits
DATermList::get_weighting() const
{
    Assert(!at_end());
    Assert(have_started);
    Assert(wt != NULL);

    return wt->get_bits(pos->wdf, 1.0, pos->termfreq, dbsize);
}




DADatabase::DADatabase(int heavy_duty_)
	: heavy_duty(heavy_duty_)
{
    DA_r = NULL;
    DA_t = NULL;
    opened = false;
}

DADatabase::~DADatabase()
{
    if(DA_r != NULL) {
	DA_close(DA_r);
	DA_r = NULL;
    }
    if(DA_t != NULL) {
	DA_close(DA_t);
	DA_t = NULL;
    }
}

void
DADatabase::open(const DatabaseBuilderParams & params)
{
    Assert(!opened);

    // Check validity of parameters
    Assert(params.readonly == true);
    Assert(params.subdbs.size() == 0);
    Assert(params.paths.size() == 1);

    // Open database with specified path
    string filename_r = params.paths[0] + "/R";
    string filename_t = params.paths[0] + "/T";

    DA_r = DA_open(filename_r.c_str(), DA_RECS, heavy_duty);
    if(DA_r == NULL) {
	throw OmOpeningError(string("When opening ") + filename_r + ": " + strerror(errno));
    }

    DA_t = DA_open(filename_t.c_str(), DA_TERMS, heavy_duty);
    if(DA_t == NULL) {
	DA_close(DA_r);
	DA_r = NULL;
	throw OmOpeningError(string("When opening ") + filename_t + ": " + strerror(errno));
    }

    opened = true;

    return;
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DADatabase::open_post_list(const om_termname & tname, RSet * rset) const
{
    Assert(opened);

    // Make sure the term has been looked up
    const DATerm * the_term = term_lookup(tname);
    Assert(the_term != NULL);

    struct DA_postings * postlist;
    postlist = DA_open_postings(the_term->get_ti(), DA_t);

    LeafPostList * pl = new DAPostList(tname, postlist, the_term->get_ti()->freq);
    return pl;
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DADatabase::open_term_list(om_docid did) const
{
    Assert(opened);

    struct termvec *tv = M_make_termvec();
    int found = DA_get_termvec(DA_r, did, tv);

    if(found == 0) {
	M_lose_termvec(tv);
	throw OmDocNotFoundError(string("Docid ") + inttostring(did) +
				 string(" not found"));
    }

    M_open_terms(tv);

    DATermList *tl = new DATermList(tv, DADatabase::get_doccount());
    return tl;
}

struct record *
DADatabase::get_record(om_docid did) const
{
    Assert(opened);

    struct record *r = M_make_record();
    int found = DA_get_record(DA_r, did, r);

    if(found == 0) {
	M_lose_record(r);
	throw OmDocNotFoundError(string("Docid ") + inttostring(did) +
				 string(" not found"));
    }

    return r;
}

OmDocument *
DADatabase::open_document(om_docid did) const
{
    Assert(opened);

    return new DADocument(this, did, heavy_duty);
}

const DATerm *
DADatabase::term_lookup(const om_termname & tname) const
{
    Assert(opened);
    //DebugMsg("DADatabase::term_lookup(`" << tname.c_str() << "'): ");

    map<om_termname, DATerm>::const_iterator p = termmap.find(tname);

    const DATerm * the_term = NULL;
    if (p == termmap.end()) {
	string::size_type len = tname.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	struct DA_term_info ti;
	int found = DA_term(k, &ti, DA_t);
	free(k);

	if(found == 0) {
	    DebugMsg("Not in collection" << endl);
	} else {
	    DebugMsg("found, adding to cache" << endl);
	    pair<om_termname, DATerm> termpair(tname, DATerm(&ti, tname));
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
DADatabase::term_exists(const om_termname & tname) const
{
    if(term_lookup(tname) != NULL) return true;
    return false;
}
