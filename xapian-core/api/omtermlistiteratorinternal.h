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

	OmDatabase database;

	om_docid did;

    public:
        Internal(TermList *termlist_, const OmDatabase &database_,
		 om_docid did_)
	    : termlist(termlist_), database(database_), did(did_)
	{
	    // A TermList starts before the start, iterators start at the start
	    termlist->next();
	}

        Internal(const Internal &other) : termlist(other.termlist)
	{ }
};

class OmTermListIteratorMap::Internal {
    private:
	friend class OmTermListIteratorMap;
        friend bool operator==(const OmTermListIteratorMap &a, const OmTermListIteratorMap &b);

	OmDocument::Internal::document_terms::const_iterator it;

	OmDatabase database;

	om_docid did;

    public:
        Internal(const OmDocument::Internal::document_terms::const_iterator &it_,
		 const OmDatabase &database_, om_docid did_)
	    : it(it_), database(database_), did(did_)
	{ }

        Internal(const Internal &other) : it(other.it)
	{ }
};

#endif /* OM_HGUARD_OMTERMLISTITERATOR_H */
