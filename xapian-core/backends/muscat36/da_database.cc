/* da_database.cc: C++ class for datype access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <config.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <string>
#include <vector>
#include <algorithm>
using std::bad_alloc;
using std::pair;

#include "utils.h"
#include "database.h"
#include "leafpostlist.h"
#include "termlist.h"
#include "da_database.h"
#include "da_document.h"
#include "da_alltermslist.h"
#include "daread.h"
#include "omdebug.h"

#include <xapian/document.h>
#include <xapian/error.h>

DAPostList::DAPostList(const string & tname_,
		       struct DA_postings * postlist_,
		       Xapian::doccount termfreq_,
		       Xapian::Internal::RefCntPtr<const DADatabase> this_db_)
	: postlist(postlist_), currdoc(0), tname(tname_), termfreq(termfreq_),
	  this_db(this_db_)
{
}

DAPostList::~DAPostList()
{
    DA_close_postings(postlist);
}

PostList * DAPostList::next(Xapian::weight /*w_min*/)
{
    Assert(currdoc == 0 || !at_end());
    DEBUGLINE(DB, "DAPostList::next(/*w_min*/): current docid = " << currdoc);
    if (currdoc && currdoc < Xapian::docid(postlist->E)) {
	currdoc++;
    } else {
	DA_read_postings(postlist, 1, 0);
	currdoc = Xapian::docid(postlist->Doc);
    }
    DEBUGLINE(DB, "DAPostList::next(/*w_min*/): new docid = " << currdoc);
    return NULL;
}

PostList * DAPostList::skip_to(Xapian::docid did, Xapian::weight /*w_min*/)
{
    Assert(currdoc == 0 || !at_end());
    Assert(did >= currdoc);
    DEBUGLINE(DB, "DAPostList::skip_to(" << did <<
	      ", /*w_min*/): current docid = " << currdoc);
    if (currdoc && did <= Xapian::docid(postlist->E)) {
	currdoc = did;
    } else {
	DA_read_postings(postlist, 1, did);
	currdoc = Xapian::docid(postlist->Doc);
    }
    DEBUGLINE(DB, "DAPostList::skip_to(" << did <<
	      ", /*w_min*/): new docid = " << currdoc);
    return NULL;
}

PositionList *
DAPostList::read_position_list()
{
    return NULL;
}

PositionList *
DAPostList::open_position_list() const
{
    // This tells the level above to return begin() = end()
    return NULL;
}




DATermList::DATermList(struct termvec *tv, Xapian::doccount dbsize_,
		       Xapian::Internal::RefCntPtr<const DADatabase> this_db_)
	: have_started(false), dbsize(dbsize_), this_db(this_db_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = reinterpret_cast<char *>(tv->term);

	Xapian::doccount freq = tv->freq;
	terms.push_back(DATermListItem(string(term + 1, unsigned(term[0]) - 1),
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

    // FIXME: want to use document length
    return wt->get_bits(pos->wdf, 1, pos->termfreq, dbsize);
}




DADatabase::DADatabase(const string &filename_r, const string &filename_t,
		       const string &filename_v, bool heavy_duty_)
	: DA_r(0), DA_t(0), valuefile(0), heavy_duty(heavy_duty_)
{
    // Open database with specified path
    DA_r = DA_open(filename_r.c_str(), DA_RECS, heavy_duty);
    if (DA_r == 0) {
	throw Xapian::DatabaseOpeningError("Couldn't open " + filename_r, errno);
    }
    DA_t = DA_open(filename_t.c_str(), DA_TERMS, heavy_duty);
    if (DA_t == 0) {
	DA_close(DA_r);
	DA_r = 0;
	throw Xapian::DatabaseOpeningError("Couldn't open " + filename_t, errno);
    }

    if (filename_v.empty()) return;
    
    // Open valuefile
    valuefile = fopen(filename_v.c_str(), "rb");
    if (valuefile == 0) {
	throw Xapian::DatabaseOpeningError("Couldn't open " + filename_v, errno);
    }

    // Check for magic string at beginning of file.
    try {
	char input[8];
	errno = 0;
	size_t bytes_read = fread(input, 1, 8, valuefile);
	if (bytes_read < 8) {
	    throw Xapian::DatabaseOpeningError(string("When opening ") + filename_v +
				 ": couldn't read magic", errno);
	}
	if (memcmp(input, "omrocks!", 8)) {
	    throw Xapian::DatabaseOpeningError(string("When opening ") + filename_v +
				 ": found wrong magic - got `" + input + "'");
	}
    }
    catch (...) {
	fclose(valuefile);
	valuefile = 0;
	DA_close(DA_t);
	DA_t = 0;
	DA_close(DA_r);
	DA_r = 0;
	throw;
    }

    return;
}

DADatabase::~DADatabase()
{
    if (valuefile != 0) {
	fclose(valuefile);
	valuefile = 0;
    }
    if (DA_r != NULL) {
	DA_close(DA_r);
	DA_r = NULL;
    }
    if (DA_t != NULL) {
	DA_close(DA_t);
	DA_t = NULL;
    }
}

Xapian::doccount
DADatabase::get_doccount() const
{
    return DA_r->itemcount;
}

Xapian::doclength
DADatabase::get_avlength() const
{
    // FIXME - actually want to return real avlength.
    return 1;
}

Xapian::doclength
DADatabase::get_doclength(Xapian::docid /*did*/) const
{
    // FIXME: should return actual length.
    return get_avlength();
}

Xapian::doccount
DADatabase::get_termfreq(const string & tname) const
{
    if (term_lookup(tname).get() == 0) return 0;

    LeafPostList *pl = open_post_list_internal(tname);
    Xapian::doccount freq = 0;
    if (pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DADatabase::do_open_post_list(const string & tname) const
{
    return open_post_list_internal(tname);
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DADatabase::open_post_list_internal(const string & tname) const
{
    // Make sure the term has been looked up
    Xapian::Internal::RefCntPtr<const DATerm> the_term = term_lookup(tname);
    Assert(the_term.get() != 0);

    struct DA_postings * postlist;
    postlist = DA_open_postings(the_term->get_ti(), DA_t);

    return new DAPostList(tname, postlist, the_term->get_ti()->freq,
			  Xapian::Internal::RefCntPtr<const DADatabase>(this));
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DADatabase::open_term_list(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    struct termvec *tv = M_make_termvec();

    if (DA_get_termvec(DA_r, did, tv) == 0) {
	M_lose_termvec(tv);
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }

    M_open_terms(tv);

    return new DATermList(tv, DADatabase::get_doccount(),
			  Xapian::Internal::RefCntPtr<const DADatabase>(this));
}

struct record *
DADatabase::get_record(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    struct record *r = M_make_record();
    int found = DA_get_record(DA_r, did, r);

    if (found == 0) {
	M_lose_record(r);
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }

    return r;
}

/// Get the specified value for given document from the fast lookup file.
string
DADatabase::get_value(Xapian::docid did, Xapian::valueno valueid) const
{
    string value;
    DEBUGLINE(DB, "Looking in valuefile for valueno " << valueid << " in document " << did);
    if (valueid) return value;

    if (valuefile == 0) {
	DEBUGLINE(DB, ": don't have valuefile");
    } else {
	int seekok = fseek(valuefile, long(did) * 8, SEEK_SET);
	if (seekok == -1) {
	    DEBUGLINE(DB, ": seek off end of valuefile");
	} else {
	    char input[9];
	    size_t bytes_read = fread(input, sizeof(char), 8, valuefile);
	    if (bytes_read < 8) {
		DEBUGLINE(DB, ": read off end of valuefile");
	    } else {
		value = string(input, 8);
		DEBUGLINE(DB, ": found - value is `" << value << "'");
	    }
	}
    }
    return value;
}

Xapian::Document::Internal *
DADatabase::open_document(Xapian::docid did, bool lazy) const
{
    return new DADocument(this, did, heavy_duty, lazy);
}

PositionList * 
DADatabase::open_position_list(Xapian::docid /*did*/, const string & /*tname*/) const
{
    // This tells the level above to return begin() = end()
    return NULL;
}

Xapian::Internal::RefCntPtr<const DATerm>
DADatabase::term_lookup(const string & tname) const
{
    DEBUGMSG(DB, "DADatabase::term_lookup(`" << tname.c_str() << "'): ");

    map<string, Xapian::Internal::RefCntPtr<const DATerm> >::const_iterator p;
    p = termmap.find(tname);

    Xapian::Internal::RefCntPtr<const DATerm> the_term;
    if (p == termmap.end()) {
	string::size_type len = tname.length();
	if (len > 255) return 0;
	byte * k = reinterpret_cast<byte *>(malloc(len + 1));
	if (k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy(reinterpret_cast<char*>(k + 1), len, 0);

	struct DA_term_info ti;
	int found = DA_term(k, &ti, DA_t);
	free(k);

	if (found == 0) {
	    DEBUGLINE(DB, "Not in collection");
	} else {
	    // FIXME: be a bit nicer on the cache than this
	    if (termmap.size() > 500) {
		DEBUGLINE(DB, "cache full, wiping");
		termmap.clear();
	    }

	    DEBUGLINE(DB, "found, adding to cache");
	    pair<string, Xapian::Internal::RefCntPtr<const DATerm> > termpair(tname, new DATerm(&ti, tname));
	    termmap.insert(termpair);
	    the_term = termmap.find(tname)->second;
	}
    } else {
	the_term = (*p).second;
	DEBUGLINE(DB, "found in cache");
    }
    return the_term;
}

bool
DADatabase::term_exists(const string & tname) const
{
    Assert(tname.size() != 0);
    return (term_lookup(tname).get() != 0);
}

TermList *
DADatabase::open_allterms() const
{
    DA_term_info daterm;
    string zero("", 1);
    DA_term(reinterpret_cast<const byte *>(zero.data()),
	    &daterm, DA_t);
    return new DAAllTermsList(Xapian::Internal::RefCntPtr<const DADatabase>(this),
			      daterm,
			      DA_t);
}
