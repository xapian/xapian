/* omtermlistiteratorinternal.h
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
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_OMTERMLISTITERATORINTERNAL_H
#define OM_HGUARD_OMTERMLISTITERATORINTERNAL_H

#include "om/omtermlistiterator.h"
#include "termlist.h"

#include "omdocumentinternal.h"

class OmTermListIterator::Internal {
    private:
	friend class OmTermListIterator; // allow access to termlist
        friend bool operator==(const OmTermListIterator &a, const OmTermListIterator &b);

	RefCntPtr<TermList> termlist;

	/// The database to read position lists from.
	OmDatabase database;

	/// The document ID in the database to read position lists from.
	om_docid did;

	OmDocument::Internal::document_terms::const_iterator it;
	OmDocument::Internal::document_terms::const_iterator it_end;

	/// Whether we're using the termlist object, or the iterator
	bool using_termlist;

    public:
        Internal(TermList *termlist_,
		 const OmDatabase &database_,
		 om_docid did_)
		: termlist(termlist_),
		  database(database_),
		  did(did_),
		  using_termlist(true)
	{
	    // A TermList starts before the start, iterators start at the start
	    termlist->next();
	}

	Internal(const OmDocument::Internal::document_terms::const_iterator &it_,
		 const OmDocument::Internal::document_terms::const_iterator &it_end_)
		: it(it_),
		  it_end(it_end_),
		  using_termlist(false)
	{ }

        Internal(const Internal &other)
		: termlist(other.termlist),
		  database(other.database),
		  did(other.did),
		  it(other.it),
		  it_end(other.it_end),
		  using_termlist(other.using_termlist)
	{ }

	void next()
	{
	    if (using_termlist) termlist->next();
	    else it++;
	}

	void skip_to(const om_termname & tname)
	{
	    if (using_termlist) {
		// FIXME: termlists should have a skip_to method and use it
		// here
		while (!termlist->at_end() && termlist->get_termname() < tname)
		    termlist->next();
	    } else {
		// FIXME: use map operations to jump to correct position
		while (it != it_end && it->first < tname)
		    it++;
	    }
	}

	bool at_end()
	{
	    if (using_termlist) {
		return termlist->at_end();
	    } else {
		return it == it_end;
	    }
	}

	bool operator== (const OmTermListIterator::Internal &other)
	{
	    if (using_termlist != other.using_termlist) return false;
	    if (using_termlist) {
		return (termlist.get() == other.termlist.get());
	    } else {
		return (it == other.it);
	    }
	}
};

class OmTermIterator::Internal {
    private:
	friend class OmTermIterator; // allow access to iterators
        friend bool operator==(const OmTermIterator &a, const OmTermIterator &b);

	std::vector<om_termname> terms;
	std::vector<om_termname>::const_iterator it;

    public:
	Internal(const std::vector<om_termname>::const_iterator &begin,
		 const std::vector<om_termname>::const_iterator &end)
		: terms(begin, end), it(terms.begin())
	{ }

        Internal(const Internal &other)
		: terms(other.terms), it(other.it - other.terms.begin() + terms.begin())
	{ }
};

#endif /* OM_HGUARD_OMTERMLISTITERATOR_H */
