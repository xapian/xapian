/* omtermlistiteratorinternal.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003 Olly Betts
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
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_OMTERMLISTITERATORINTERNAL_H
#define OM_HGUARD_OMTERMLISTITERATORINTERNAL_H

#include "om/omtermlistiterator.h"
#include "termlist.h"

#include "omdocumentinternal.h"
#include "ompositionlistiteratorinternal.h"

#include "inmemory_positionlist.h"

class OmTermIterator::Internal {
    private:
	RefCntPtr<TermList> termlist;

	/// The database to read position lists from.
	OmDatabase db;

	/// The document ID in the database to read position lists from.
	om_docid did;

    public:
        Internal(TermList *termlist_, const OmDatabase &db_, om_docid did_)
		: termlist(termlist_), db(db_), did(did_)
	{
	    // A TermList starts before the start, iterators start at the start
	    termlist->next();
	}

        Internal(TermList *termlist_) : termlist(termlist_), did(0)
	{
	    // A TermList starts before the start, iterators start at the start
	    termlist->next();
	}

	string get_termname() const {	    
	    return termlist->get_termname();
	}

	om_termcount get_wdf() const {
	    return termlist->get_wdf();
	}

	om_doccount get_termfreq() const {
	    return termlist->get_termfreq();
	}
	
	OmPositionListIterator positionlist_begin() const {
	    if (did)
		return db.positionlist_begin(did, termlist->get_termname());
	    return termlist->positionlist_begin();
	}

	void next() {
	    termlist->next();
	}

	void skip_to(const string & tname) {
	    termlist->skip_to(tname);
	}

	bool at_end() const {
	    return termlist->at_end();
	}

	bool operator==(const OmTermIterator::Internal &other) const {
	    return termlist.get() == other.termlist.get();
	}
};

class VectorTermList : public TermList {
    private:
	std::vector<string> terms;
	std::vector<string>::size_type offset;
	bool before_start;

    public:
	VectorTermList(std::vector<string>::const_iterator begin,
		       std::vector<string>::const_iterator end)
	    : terms(begin, end), offset(0), before_start(true)
	{
	}

	// Gets size of termlist
	om_termcount get_approx_size() const {
	    return terms.size();
	}

	// Gets weighting info for current term
	OmExpandBits get_weighting() const {
	    Assert(false); // should never get called
            throw OmInvalidOperationError("VectorTermList::get_weighting() not supported");
	}
	    
	// Gets current termname
	string get_termname() const {
	    Assert(!before_start && offset < terms.size());
	    return terms[offset];
	}

	// Get wdf of current term
	om_termcount get_wdf() const {
	    Assert(!before_start && offset < terms.size());
	    return 1; // FIXME: or is OmInvalidOperationError better?
	}

	// Get num of docs indexed by term
	om_doccount get_termfreq() const {
            throw OmInvalidOperationError("VectorTermList::get_termfreq() not supported");
	}

	/** next() causes the TermList to move to the next term in the list.
	 *  It must be called before any other methods.
	 *  If next() returns a non-zero pointer P, then the original
	 *  termlist should be deleted, and the original pointer replaced
	 *  with P.
	 *  In LeafTermList, next() will always return 0.
	 */
	TermList * next() {
	    Assert(!at_end());
	    if(before_start)
		before_start = false;
	    else
		offset++;
	    return NULL;
	}

	TermList *skip_to(const string &/*tname*/) {
	    Assert(!at_end());
	    // termlist not ordered
	    Assert(false);
            throw OmInvalidOperationError("VectorTermList::skip_to() not supported");
	}
	
	// True if we're off the end of the list
	bool at_end() const {
	    return !before_start && offset == terms.size();
	}
};

class MapTermList : public TermList {
    private:
	OmDocument::Internal::document_terms::const_iterator it;
	OmDocument::Internal::document_terms::const_iterator it_end;
	om_termcount size;
	bool started;

    public:
        MapTermList(const OmDocument::Internal::document_terms::const_iterator &it_,
		    const OmDocument::Internal::document_terms::const_iterator &it_end_,
		    om_termcount size_)
		: it(it_), it_end(it_end_), size(size_), started(false)
	{}

	// Gets size of termlist
	om_termcount get_approx_size() const {
	    return size;
	}

	// Gets weighting info for current term
	OmExpandBits get_weighting() const {
	    Assert(false); // should never get called
	    abort();
#ifdef __SUNPRO_CC
	    // For compilers which worry abort() might return...
            return OmExpandBits(0, 0, 0);
#endif
	}
	    
	// Gets current termname
	string get_termname() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->first;
	}

	// Get wdf of current term
	om_termcount get_wdf() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->second.wdf;
	}

	// Get num of docs indexed by term
	om_doccount get_termfreq() const {
	    Assert(started);
	    Assert(!at_end());
	    return it->second.termfreq;
	}

	OmPositionListIterator positionlist_begin() const {
	    AutoPtr<InMemoryPositionList> pl(new InMemoryPositionList());
	    pl->set_data(it->second.positions);
	    return OmPositionListIterator(new OmPositionListIterator::Internal(
					AutoPtr<PositionList>(pl.release())));
	}

	// FIXME: needs to allow a next() before we start
	TermList * next() {
	    if (!started) {
		started = true;
	    } else {
		Assert(!at_end());
		it++;
	    }
	    return NULL;
	}

	// True if we're off the end of the list
	bool at_end() const {
	    Assert(started);
	    return it == it_end;
	}
};

#endif /* OM_HGUARD_OMTERMLISTITERATOR_H */
