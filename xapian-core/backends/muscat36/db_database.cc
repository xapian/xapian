/* db_database.cc: C++ class for datype access routines
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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>

#include "utils.h"
#include "database.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "db_database.h"
#include "db_document.h"
#include "dbread.h"

DBPostList::DBPostList(const om_termname & tname_,
		       struct DB_postings * postlist_,
		       om_doccount termfreq_)
	: postlist(postlist_), currdoc(0), tname(tname_), termfreq(termfreq_)
{
}

DBPostList::~DBPostList()
{
    DB_close_postings(postlist);
}

om_weight DBPostList::get_weight() const
{
    Assert(!at_end());
    Assert(currdoc != 0);
    Assert(ir_wt != NULL);

    // NB ranges from dbread share the same wdf value
    return ir_wt->get_sumpart(postlist->wdf, 1.0);
}

PostList * DBPostList::next(om_weight w_min)
{
    Assert(currdoc == 0 || !at_end());
    if (currdoc && currdoc < om_docid(postlist->E)) {	
	currdoc++;
	return NULL;
    }
    DB_read_postings(postlist, 1, 0);
    currdoc = om_docid(postlist->Doc);
    return NULL;
}

PostList * DBPostList::skip_to(om_docid did, om_weight w_min)
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
    DB_read_postings(postlist, 1, did);
    currdoc = om_docid(postlist->Doc);
    //printf("%d - get_id %d\n", did, currdoc);
    return NULL;
}



DBTermList::DBTermList(struct termvec *tv, om_doccount dbsize_)
	: have_started(false), dbsize(dbsize_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;

	om_doccount freq = tv->freq;
	terms.push_back(DBTermListItem(string(term + 1, (unsigned)term[0] - 1),
				       tv->wdf, freq));
	M_read_terms(tv);
    }
    M_lose_termvec(tv);

    pos = terms.begin();
}

OmExpandBits
DBTermList::get_weighting() const
{
    Assert(!at_end());
    Assert(have_started);
    Assert(wt != NULL);

    return wt->get_bits(pos->wdf, 1.0, pos->termfreq, dbsize);
}




DBDatabase::DBDatabase(int heavy_duty_)
	: heavy_duty(heavy_duty_)
{
    DB = NULL;
    opened = false;
}

DBDatabase::~DBDatabase()
{
    if(DB != NULL) {
	DB_close(DB);
	DB = NULL;
    }
}

void
DBDatabase::open(const DatabaseBuilderParams & params)
{
    Assert(!opened);

    // Check validity of parameters
    Assert(params.readonly == true);
    Assert(params.subdbs.size() == 0);
    Assert(params.paths.size() <= 1);
    Assert(params.paths.size() >= 2);

    // Open database with specified path
    string filename_r = params.paths[0] + "/R";
    string filename_t = params.paths[0] + "/T";

    // Get the cache_size
    int cache_size = 30;
    if (params.paths.size() == 2) {
	cache_size = atoi(params.paths[1].c_str());
    }

    // Actually open
    DB = DB_open(filename_r.c_str(), cache_size, heavy_duty);
    if(DB == NULL) {
	throw OmOpeningError(string("When opening ") + filename_r + ": " + strerror(errno));
    }

    opened = true;

    return;
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DBDatabase::open_post_list(const om_termname & tname, RSet * rset) const
{
    Assert(opened);

    // Make sure the term has been looked up
    const DBTerm * the_term = term_lookup(tname);
    Assert(the_term != NULL);

    struct DB_postings * postlist;
    postlist = DB_open_postings(the_term->get_ti(), DB);

    LeafPostList * pl = new DBPostList(tname, postlist, the_term->get_ti()->freq);
    return pl;
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DBDatabase::open_term_list(om_docid did) const
{
    Assert(opened);

    struct termvec *tv = M_make_termvec();
    int found = DB_get_termvec(DB, did, tv);

    if(found == 0) {
	M_lose_termvec(tv);
	throw OmDocNotFoundError(string("Docid ") + inttostring(did) +
				 string(" not found"));
    }

    M_open_terms(tv);

    DBTermList *tl = new DBTermList(tv, DBDatabase::get_doccount());
    return tl;
}

struct record *
DBDatabase::get_record(om_docid did) const
{
    Assert(opened);

    struct record *r = M_make_record();
    int found = DB_get_record(DB, did, r);

    if(found == 0) {
	M_lose_record(r);
	throw OmDocNotFoundError(string("Docid ") + inttostring(did) +
				 string(" not found"));
    }

    return r;
}

LeafDocument *
DBDatabase::open_document(om_docid did) const
{
    Assert(opened);

    return new DBDocument(this, did, heavy_duty);
}

const DBTerm *
DBDatabase::term_lookup(const om_termname & tname) const
{
    Assert(opened);
    //DebugMsg("DBDatabase::term_lookup(`" << tname.c_str() << "'): ");

    map<om_termname, DBTerm>::const_iterator p = termmap.find(tname);

    const DBTerm * the_term = NULL;
    if (p == termmap.end()) {
	string::size_type len = tname.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	struct DB_term_info ti;
	int found = DB_term(k, &ti, DB);
	free(k);

	if(found == 0) {
	    DebugMsg("Not in collection" << endl);
	} else {
	    DebugMsg("found, adding to cache" << endl);
	    pair<om_termname, DBTerm> termpair(tname, DBTerm(&ti, tname));
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
DBDatabase::term_exists(const om_termname & tname) const
{
    if(term_lookup(tname) != NULL) return true;
    return false;
}
