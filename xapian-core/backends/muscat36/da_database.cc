/* da_database.cc: C++ class for datype access routines
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
#include "omdebug.h"

#include <om/omdocument.h>
#include <om/omerror.h>

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
	DEBUGMSG(DB, "Skip within range " << did << endl);
	return NULL;
    }
    //printf("%p:From %d skip_to ", this, currdoc);
    DA_read_postings(postlist, 1, did);
    currdoc = om_docid(postlist->Doc);
    //printf("%d - get_id %d\n", did, currdoc);
    return NULL;
}

PositionList *
DAPostList::get_position_list()
{
    throw OmUnimplementedError("DAPostList::get_position_list() unimplemented");
}




DATermList::DATermList(struct termvec *tv, om_doccount dbsize_)
	: have_started(false), dbsize(dbsize_)
{
    // FIXME - read terms as we require them, rather than all at beginning?
    M_read_terms(tv);
    while(tv->term != 0) {
	char *term = (char *)tv->term;

	om_doccount freq = tv->freq;
	terms.push_back(DATermListItem(std::string(term + 1, (unsigned)term[0] - 1),
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




DADatabase::DADatabase(const DatabaseBuilderParams & params, int heavy_duty_)
	: DA_r(0),
	  DA_t(0),
	  keyfile(0),
	  heavy_duty(heavy_duty_)
{
    // Check validity of parameters
    if(params.readonly != true) {
	throw OmInvalidArgumentError("DADatabase must be opened readonly.");
    }

    if(params.subdbs.size() != 0) {
	throw OmInvalidArgumentError("DADatabase cannot have sub databases.");
    }
    if(params.paths.size() < 1 || params.paths.size() > 3) {
	throw OmInvalidArgumentError("DADatabase requires 1, 2 or 3 parameters.");
    }

    // Work out file paths
    std::string filename_r;
    std::string filename_t;
    std::string filename_k;

    if(params.paths.size() == 1) {
	filename_r = params.paths[0] + "/R";
	filename_t = params.paths[0] + "/T";
	filename_k = params.paths[0] + "/keyfile";
    } else {
	filename_r = params.paths[0];
	filename_t = params.paths[1];
	if(params.paths.size() == 3) {
	    filename_k = params.paths[2];
	}
    }


    // Open database with specified path
    DA_r = DA_open(filename_r.c_str(), DA_RECS, heavy_duty);
    if (DA_r == 0) {
	throw OmOpeningError(std::string("When opening ") + filename_r + ": " + strerror(errno));
    }
    DA_t = DA_open(filename_t.c_str(), DA_TERMS, heavy_duty);
    if (DA_t == 0) {
	DA_close(DA_r);
	DA_r = 0;
	throw OmOpeningError(std::string("When opening ") + filename_t + ": " + strerror(errno));
    }

    // Open keyfile, if we can
    keyfile = fopen(filename_k.c_str(), "rb");
    if (keyfile != 0) {
	// Check for magic string at beginning of file.
	char input[9];
	size_t bytes_read = fread(input, sizeof(char), 8, keyfile);
	if(bytes_read < 8) {
	    fclose(keyfile);
	    keyfile = 0;
	    DA_close(DA_t);
	    DA_t = 0;
	    DA_close(DA_r);
	    DA_r = 0;
	    throw OmOpeningError(std::string("When opening ") + filename_k + ": couldn't read magic - " + strerror(errno));
	} else {
	    input[8] = '\0';
	    if(strcmp(input, "omrocks!")) {
		fclose(keyfile);
		keyfile = 0;
		DA_close(DA_t);
		DA_t = 0;
		DA_close(DA_r);
		DA_r = 0;
		throw OmOpeningError(std::string("When opening ") + filename_k + ": couldn't read magic - got `" + input + "'");
	    }
	}
    } else if(params.paths.size() == 3) {
	throw OmOpeningError(std::string("When opening ") + filename_k + ": " + strerror(errno));
    }

    return;
}

DADatabase::~DADatabase()
{
    if(keyfile != 0) {
	fclose(keyfile);
	keyfile = 0;
    }
    if(DA_r != NULL) {
	DA_close(DA_r);
	DA_r = NULL;
    }
    if(DA_t != NULL) {
	DA_close(DA_t);
	DA_t = NULL;
    }
}

om_doccount
DADatabase::get_doccount() const
{
    OmLockSentry sentry(mutex);
    return get_doccount_internal();
}

om_doccount
DADatabase::get_doccount_internal() const
{
    return DA_r->itemcount;
}

om_doclength
DADatabase::get_avlength() const
{
    // FIXME - want this mutex back if implement get_avlength_internal()
    //OmLockSentry sentry(mutex);
    return get_avlength_internal();
}

om_doclength
DADatabase::get_avlength_internal() const
{
    // FIXME - actually want to return real avlength.
    return 1;
}

om_doclength
DADatabase::get_doclength(om_docid did) const
{
    // FIXME: should return actual length.
    //OmLockSentry sentry(mutex);
    return get_avlength_internal();
}

om_doccount
DADatabase::get_termfreq(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);

    if(term_lookup(tname).get() == 0) return 0;

    PostList *pl = open_post_list_internal(tname);
    om_doccount freq = 0;
    if(pl) freq = pl->get_termfreq();
    delete pl;
    return freq;
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DADatabase::open_post_list(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);
    return open_post_list_internal(tname);
}

// Returns a new posting list, for the postings in this database for given term
LeafPostList *
DADatabase::open_post_list_internal(const om_termname & tname) const
{
    // Make sure the term has been looked up
    OmRefCntPtr<const DATerm> the_term = term_lookup(tname);
    Assert(the_term.get() != 0);

    struct DA_postings * postlist;
    postlist = DA_open_postings(the_term->get_ti(), DA_t);

    LeafPostList * pl = new DAPostList(tname, postlist, the_term->get_ti()->freq);
    return pl;
}

// Returns a new term list, for the terms in this database for given document
LeafTermList *
DADatabase::open_term_list(om_docid did) const
{
    OmLockSentry sentry(mutex);

    struct termvec *tv = M_make_termvec();
    int found = DA_get_termvec(DA_r, did, tv);

    if(found == 0) {
	M_lose_termvec(tv);
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }

    M_open_terms(tv);

    DATermList *tl = new DATermList(tv, DADatabase::get_doccount_internal());
    return tl;
}

struct record *
DADatabase::get_record(om_docid did) const
{
    OmLockSentry sentry(mutex);

    struct record *r = M_make_record();
    int found = DA_get_record(DA_r, did, r);

    if(found == 0) {
	M_lose_record(r);
	throw OmDocNotFoundError(std::string("Docid ") + om_tostring(did) +
				 std::string(" not found"));
    }

    return r;
}

/// Get the specified key for given document from the fast lookup file.
OmKey
DADatabase::get_key(om_docid did, om_keyno keyid) const
{
    OmLockSentry sentry(mutex);

    OmKey key;
    DEBUGMSG(DB, "Looking in keyfile for keyno " << keyid << " in document " << did);

    if (keyfile == 0) {
	DEBUGMSG(DB, ": don't have keyfile - using record" << endl);
    } else {
	int seekok = fseek(keyfile, (long)did * 8, SEEK_SET);
	if(seekok == -1) {
	    DEBUGMSG(DB, ": seek off end of keyfile - using record" << endl);
	} else {
	    char input[9];
	    size_t bytes_read = fread(input, sizeof(char), 8, keyfile);
	    if(bytes_read < 8) {
		DEBUGMSG(DB, ": read off end of keyfile - using record" << endl);
	    } else {
		key.value = std::string(input, 8);
		DEBUGMSG(DB, ": found - value is `" << key.value << "'" << endl);
	    }
	}
    }
    return key;
}

LeafDocument *
DADatabase::open_document(om_docid did) const
{
    OmLockSentry sentry(mutex);

    return new DADocument(this, did, heavy_duty);
}

OmRefCntPtr<const DATerm>
DADatabase::term_lookup(const om_termname & tname) const
{
    DEBUGMSG(DB, "DADatabase::term_lookup(`" << tname.c_str() << "'): ");

    std::map<om_termname, OmRefCntPtr<const DATerm> >::const_iterator p;
    p = termmap.find(tname);

    OmRefCntPtr<const DATerm> the_term;
    if (p == termmap.end()) {
	std::string::size_type len = tname.length();
	if(len > 255) return 0;
	byte * k = (byte *) malloc(len + 1);
	if(k == NULL) throw std::bad_alloc();
	k[0] = len + 1;
	tname.copy((char*)(k + 1), len, 0);

	struct DA_term_info ti;
	int found = DA_term(k, &ti, DA_t);
	free(k);

	if(found == 0) {
	    DEBUGMSG(DB, "Not in collection" << endl);
	} else {
	    // FIXME: be a bit nicer on the cache than this
	    if(termmap.size() > 500) {
		DEBUGMSG(DB, "cache full, wiping");
		termmap.clear();
	    }

	    DEBUGMSG(DB, "found, adding to cache" << endl);
	    std::pair<om_termname, OmRefCntPtr<const DATerm> > termpair(tname, new DATerm(&ti, tname));
	    termmap.insert(termpair);
	    the_term = termmap.find(tname)->second;
	}
    } else {
	the_term = (*p).second;
	DEBUGMSG(DB, "found in cache" << endl);
    }
    return the_term;
}

bool
DADatabase::term_exists(const om_termname & tname) const
{
    OmLockSentry sentry(mutex);
    if(term_lookup(tname).get() != 0) return true;
    return false;
}
