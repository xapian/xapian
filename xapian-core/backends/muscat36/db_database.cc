/* db_database.cc: C++ class for datype access routines
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
#include "omdebug.h"

#include <om/omdocument.h>
#include <om/omerror.h>

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
	//DEBUGLINE(DB, "Skip within range " << did);
	return NULL;
    }
    //printf("%p:From %d skip_to ", this, currdoc);
    DB_read_postings(postlist, 1, did);
    currdoc = om_docid(postlist->Doc);
    //printf("%d - get_id %d\n", did, currdoc);
    return NULL;
}

PositionList *
DBPostList::get_position_list()
{
    throw OmUnimplementedError("DBPostList::get_position_list() unimplemented");
}



DBTermList::DBTermList(struct termvec *tv, om_doccount dbsize_)
	: have_started(false), dbsize(dbsize_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;

	om_doccount freq = tv->freq;
	terms.push_back(DBTermListItem(std::string(term + 1, (unsigned)term[0] - 1),
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

    // FIXME: want to use document length
    return wt->get_bits(pos->wdf, 1, pos->termfreq, dbsize);
}




DBDatabase::DBDatabase(const OmSettings & params, bool readonly) : DB(0), keyfile(0)
{    
    // Check validity of parameters
    if (!readonly) {
	throw OmInvalidArgumentError("DBDatabase must be opened readonly.");
    }

    std::string filename = params.get("m36_db_file");

    std::string filename_k = params.get("m36_key_file", "");

    // Get the cache_size
    int cache_size = params.get_int("m36_db_cache_size", 30);

    // Actually open
    DB = DB_open(filename.c_str(), cache_size);
    if (DB == 0) {
	throw OmOpeningError(std::string("When opening ") + filename + ": " + strerror(errno));
    }

    if (filename_k.empty()) return;
    
    // Open keyfile
    keyfile = fopen(filename_k.c_str(), "rb");
    if (keyfile == 0) {
	throw OmOpeningError(std::string("When opening ") + filename_k +
			     ": " + strerror(errno));
    }

    // Check for magic string at beginning of file.
    try {
	char input[9];
	size_t bytes_read = fread(input, sizeof(char), 8, keyfile);
	if (bytes_read < 8) {
	    throw OmOpeningError(std::string("When opening ") + filename_k +
				 ": couldn't read magic - " + strerror(errno));
	}
	input[8] = '\0';
	if (strcmp(input, "omrocks!")) {
	    throw OmOpeningError(std::string("When opening ") + filename_k +
				 ": couldn't read magic - got `" +
				 input + "'");
	}
    }
    catch (...) {
	fclose(keyfile);
	DB_close(DB);
	DB = 0;
	throw;
    }

    return;
}

DBDatabase::~DBDatabase()
{
    try {
	internal_end_session();
    } catch (...) {
	// Ignore any exceptions, since we may be being called due to an
	// exception anyway.  internal_end_session() should have already
	// been called, in the normal course of events.
    }

    if(keyfile != 0) {
	fclose(keyfile);
	keyfile = 0;
    }
    if(DB != 0) {
	DB_close(DB);
	DB = 0;
    }
}

om_doccount
DBDatabase::get_doccount() const
{
    OmLockSentry sentry(mutex);
    return get_doccount_internal();
}

om_doccount
DBDatabase::get_doccount_internal() const
{
    return DB->doc_count;
}

om_doclength
DBDatabase::get_avlength() const
{
    // FIXME - want this mutex back if implement get_avlength_internal()
    //OmLockSentry sentry(mutex);
    return get_avlength_internal();
}

om_doclength
DBDatabase::get_avlength_internal() const
{
    // FIXME - actually want to return real avlength.
    return 1;
}

om_doclength
DBDatabase::get_doclength(om_docid did) const
{
    // FIXME: should return actual length.
    //OmLockSentry sentry(mutex);
    return get_avlength_internal();
}

om_doccount
DBDatabase::get_termfreq(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);

    if(!term_exists_internal(tname)) return 0;
    PostList *pl = open_post_list_internal(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

bool
DBDatabase::term_exists_internal(const om_termname & tname) const
{
    if(term_lookup(tname).get() != 0) return true;
    return false;
}

bool
DBDatabase::term_exists(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);

    return term_exists_internal(tname);
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DBDatabase::open_post_list_internal(const om_termname & tname) const
{
    // Make sure the term has been looked up
    OmRefCntPtr<const DBTerm> the_term = term_lookup(tname);
    Assert(the_term.get() != 0);

    struct DB_postings * postlist;
    postlist = DB_open_postings(the_term->get_ti(), DB);

    LeafPostList * pl = new DBPostList(tname, postlist, the_term->get_ti()->freq);
    return pl;
}

LeafPostList *
DBDatabase::do_open_post_list(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);

    return open_post_list_internal(tname);
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DBDatabase::open_term_list(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    OmLockSentry sentry(mutex);

    struct termvec *tv = M_make_termvec();
    int found = DB_get_termvec(DB, did, tv);

    if(found == 0) {
	M_lose_termvec(tv);
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }

    M_open_terms(tv);

    DBTermList *tl = new DBTermList(tv, DBDatabase::get_doccount_internal());
    return tl;
}

struct record *
DBDatabase::get_record(om_docid did) const
{
    if (did == 0) throw OmInvalidArgumentError("Docid 0 invalid");
    OmLockSentry sentry(mutex);

    struct record *r = M_make_record();
    int found = DB_get_record(DB, did, r);

    if(found == 0) {
	M_lose_record(r);
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }

    return r;
}

/// Get the specified key for given document from the fast lookup file.
OmKey
DBDatabase::get_key(om_docid did, om_keyno keyid) const
{
    OmLockSentry sentry(mutex);

    OmKey key;
    DEBUGLINE(DB, "Looking in keyfile for keyno " << keyid << " in document " << did);

    if (keyfile == 0) {
	DEBUGLINE(DB, ": don't have keyfile - using record");
    } else {
	int seekok = fseek(keyfile, (long)did * 8, SEEK_SET);
	if(seekok == -1) {
	    DEBUGLINE(DB, ": seek off end of keyfile - using record");
	} else {
	    char input[9];
	    size_t bytes_read = fread(input, sizeof(char), 8, keyfile);
	    if(bytes_read < 8) {
		DEBUGLINE(DB, ": read off end of keyfile - using record");
	    } else {
		key.value = std::string(input, 8);
		DEBUGLINE(DB, ": found - value is `" << key.value << "'");
	    }
	}
    }
    return key;
}

LeafDocument *
DBDatabase::open_document(om_docid did) const
{
    OmLockSentry sentry(mutex);

    return new DBDocument(this, did, DB->heavy_duty);
}

OmRefCntPtr<const DBTerm>
DBDatabase::term_lookup(const om_termname & tname) const
{
    //DEBUGLINE(DB, "DBDatabase::term_lookup(`" << tname.c_str() << "'): ");

    std::map<om_termname, OmRefCntPtr<const DBTerm> >::const_iterator p;
    p = termmap.find(tname);

    OmRefCntPtr<const DBTerm> the_term;
    if (p == termmap.end()) {
	std::string::size_type len = tname.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw std::bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	struct DB_term_info ti;
	int found = DB_term(k, &ti, DB);
	free(k);

	if(found == 0) {
	    DEBUGLINE(DB, "Not in collection");
	} else {
	    // FIXME: be a bit nicer on the cache than this
	    if(termmap.size() > 500) {
		DEBUGLINE(DB, "cache full, wiping");
		termmap.clear();
	    }

	    DEBUGLINE(DB, "found, adding to cache");
	    std::pair<om_termname, OmRefCntPtr<const DBTerm> > termpair(tname, new DBTerm(&ti, tname));
	    termmap.insert(termpair);
	    the_term = termmap.find(tname)->second;
	}
    } else {
	the_term = (*p).second;
	DEBUGLINE(DB, "found in cache");
    }
    return the_term;
}

