/* db_database.cc: C++ class for dbtype access routines
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
#include "db_database.h"
#include "db_document.h"
#include "dbread.h"
#include "omdebug.h"

#include <xapian/document.h>
#include <xapian/error.h>

DBPostList::DBPostList(const string & tname_,
		       struct DB_postings * postlist_,
		       Xapian::doccount termfreq_,
		       Xapian::Internal::RefCntPtr<const DBDatabase> this_db_)
	: postlist(postlist_), currdoc(0), tname(tname_), termfreq(termfreq_),
	  this_db(this_db_)
{
}

DBPostList::~DBPostList()
{
    DB_close_postings(postlist);
}

PostList * DBPostList::next(Xapian::weight /*w_min*/)
{
    Assert(currdoc == 0 || !at_end());
    DEBUGLINE(DB, "DBPostList::next(/*w_min*/): current docid = " << currdoc);
    if (currdoc && currdoc < Xapian::docid(postlist->E)) {
	currdoc++;
    } else {
	DB_read_postings(postlist, 1, 0);
	currdoc = Xapian::docid(postlist->Doc);
    }
    DEBUGLINE(DB, "DBPostList::next(" << w_min <<
	      "): new docid = " << currdoc);
    return NULL;
}

PostList * DBPostList::skip_to(Xapian::docid did, Xapian::weight /*w_min*/)
{
    Assert(currdoc == 0 || !at_end());
    Assert(did >= currdoc);
    DEBUGLINE(DB, "DBPostList::skip_to(" << did <<
	      ", /*w_min*/): current docid = " << currdoc);
    if (currdoc && did <= Xapian::docid(postlist->E)) {
	DEBUGLINE(DB, "skip within range (end of range is " <<
		  Xapian::docid(postlist->E) << ")");
	currdoc = did;
    } else {
	DB_read_postings(postlist, 1, did);
	DEBUGLINE(DB, "reading more postings");
	currdoc = Xapian::docid(postlist->Doc);
    }
    DEBUGLINE(DB, "DBPostList::skip_to(" << did <<
	      ", /*w_min*/): new docid = " << currdoc);
    return NULL;
}

PositionList *
DBPostList::read_position_list()
{
    return NULL;
}

PositionList *
DBPostList::open_position_list() const
{
    // This tells the level above to return begin() = end()
    return NULL;
}



DBTermList::DBTermList(struct termvec *tv, Xapian::doccount dbsize_,
		       Xapian::Internal::RefCntPtr<const DBDatabase> this_db_)
	: have_started(false), dbsize(dbsize_), this_db(this_db_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = reinterpret_cast<char *>(tv->term);

	Xapian::doccount freq = tv->freq;
	terms.push_back(DBTermListItem(string(term + 1, unsigned(term[0]) - 1),
				       tv->wdf, freq));
	M_read_terms(tv);
    }
    M_lose_termvec(tv);

    pos = terms.begin();
}

Xapian::doccount
DBTermList::get_termfreq() const
{   
    Assert(!at_end());
    Assert(have_started);
    // FIXME: really icky cast
    if (pos->termfreq == Xapian::termcount(-1)) {
	// Not available - read from database
	DEBUGLINE(DB, "DBTermList::get_termfreq - termfreq for `" << pos->tname
		  << "' not available, reading from database");
	pos->termfreq = this_db->get_termfreq(pos->tname);
    }
    return pos->termfreq;
}

OmExpandBits
DBTermList::get_weighting() const
{
    Assert(!at_end());
    Assert(have_started);
    Assert(wt != NULL);

    // FIXME: want to use document length
    return wt->get_bits(pos->wdf, 1, get_termfreq(), dbsize);
}




DBDatabase::DBDatabase(const string &filename, const string &filename_v,
		       int cache_size) : DB(0), valuefile(0)
{    
    // Actually open
    DB = DB_open(filename.c_str(), cache_size);
    if (DB == 0) {
	throw Xapian::DatabaseOpeningError(string("When opening ") + filename + ": " + strerror(errno));
    }

    if (filename_v.empty()) return;
    
    // Open valuefile
    valuefile = fopen(filename_v.c_str(), "rb");
    if (valuefile == 0) {
	throw Xapian::DatabaseOpeningError(string("When opening ") + filename_v +
			     ": " + strerror(errno));
    }

    // Check for magic string at beginning of file.
    try {
	char input[8];
	size_t bytes_read = fread(input, 1, 8, valuefile);
	if (bytes_read < 8) {
	    throw Xapian::DatabaseOpeningError(string("When opening ") + filename_v +
				 ": couldn't read magic - " + strerror(errno));
	}
	if (memcmp(input, "omrocks!", 8)) {
	    throw Xapian::DatabaseOpeningError(string("When opening ") + filename_v +
				 ": couldn't read magic - got `" +
				 input + "'");
	}
    }
    catch (...) {
	fclose(valuefile);
	DB_close(DB);
	DB = 0;
	throw;
    }
}

DBDatabase::~DBDatabase()
{
    if (valuefile != 0) {
	fclose(valuefile);
	valuefile = 0;
    }
    if (DB != 0) {
	DB_close(DB);
	DB = 0;
    }
}

Xapian::doccount
DBDatabase::get_doccount() const
{
    return DB->doc_count;
}

Xapian::doclength
DBDatabase::get_avlength() const
{
    // FIXME - actually want to return real avlength.
    return 1;
}

Xapian::doclength
DBDatabase::get_doclength(Xapian::docid /*did*/) const
{
    // FIXME: should return actual length.
    return get_avlength();
}

Xapian::doccount
DBDatabase::get_termfreq(const string & tname) const
{
    if (!term_exists(tname)) return 0;
    LeafPostList *pl = open_post_list_internal(tname);
    Xapian::doccount freq = 0;
    if (pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

bool
DBDatabase::term_exists(const string & tname) const
{
    Assert(tname.size() != 0);
    return (term_lookup(tname).get() != 0);
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DBDatabase::open_post_list_internal(const string & tname) const
{
    // Make sure the term has been looked up
    Xapian::Internal::RefCntPtr<const DBTerm> the_term = term_lookup(tname);
    Assert(the_term.get() != 0);

    struct DB_postings * postlist;
    postlist = DB_open_postings(the_term->get_ti(), DB);

    return new DBPostList(tname, postlist, the_term->get_ti()->freq,
			  Xapian::Internal::RefCntPtr<const DBDatabase>(this));
}

LeafPostList *
DBDatabase::do_open_post_list(const string & tname) const
{
    return open_post_list_internal(tname);
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DBDatabase::open_term_list(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    struct termvec *tv = M_make_termvec();

    if (DB_get_termvec(DB, did, tv) == 0) {
	M_lose_termvec(tv);
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }

    M_open_terms(tv);

    return new DBTermList(tv, DBDatabase::get_doccount(),
			  Xapian::Internal::RefCntPtr<const DBDatabase>(this));
}

struct record *
DBDatabase::get_record(Xapian::docid did) const
{
    if (did == 0) throw Xapian::InvalidArgumentError("Docid 0 invalid");

    struct record *r = M_make_record();
    int found = DB_get_record(DB, did, r);

    if (found == 0) {
	M_lose_record(r);
	throw Xapian::DocNotFoundError(string("Docid ") + om_tostring(did) +
				 string(" not found"));
    }

    return r;
}

/// Get the specified value for given document from the fast lookup file.
string
DBDatabase::get_value(Xapian::docid did, Xapian::valueno valueid) const
{
    string value;
    DEBUGLINE(DB, "Looking in valuefile for valueno " << valueid << " in document " << did);
    if (valueid) return value;

    if (valuefile == 0) {
	DEBUGLINE(DB, ": don't have valuefile - using record");
    } else {
	int seekok = fseek(valuefile, long(did) * 8, SEEK_SET);
	if (seekok == -1) {
	    DEBUGLINE(DB, ": seek off end of valuefile - using record");
	} else {
	    char input[9];
	    size_t bytes_read = fread(input, sizeof(char), 8, valuefile);
	    if (bytes_read < 8) {
		DEBUGLINE(DB, ": read off end of valuefile - using record");
	    } else {
		value = string(input, 8);
		DEBUGLINE(DB, ": found - value is `" << value << "'");
	    }
	}
    }
    return value;
}

Xapian::Document::Internal *
DBDatabase::open_document(Xapian::docid did, bool lazy) const
{
    return new DBDocument(this, did, DB->heavy_duty, lazy);
}

PositionList * 
DBDatabase::open_position_list(Xapian::docid /*did*/,
			       const string & /*tname*/) const
{
    // This tells the level above to return begin() = end()
    return NULL;
}

Xapian::Internal::RefCntPtr<const DBTerm>
DBDatabase::term_lookup(const string & tname) const
{
    //DEBUGLINE(DB, "DBDatabase::term_lookup(`" << tname.c_str() << "'): ");

    map<string, Xapian::Internal::RefCntPtr<const DBTerm> >::const_iterator p;
    p = termmap.find(tname);

    Xapian::Internal::RefCntPtr<const DBTerm> the_term;
    if (p == termmap.end()) {
	string::size_type len = tname.length();
	if (len > 255) return 0;
	byte * k = reinterpret_cast<byte *>(malloc(len + 1));
	if (k == NULL) throw bad_alloc();
	k[0] = len + 1;
	tname.copy(reinterpret_cast<char*>(k + 1), len, 0);

	struct DB_term_info ti;
	int found = DB_term(k, &ti, DB);
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
	    pair<string, Xapian::Internal::RefCntPtr<const DBTerm> > termpair(tname, new DBTerm(&ti, tname));
	    termmap.insert(termpair);
	    the_term = termmap.find(tname)->second;
	}
    } else {
	the_term = (*p).second;
	DEBUGLINE(DB, "found in cache");
    }
    return the_term;
}

TermList *
DBDatabase::open_allterms() const
{
    throw Xapian::UnimplementedError("open_allterms() not implemented yet");
}
