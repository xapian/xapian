/* omenquire.h
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
 * -----END-LICENCE-----
 */

#ifndef _omenquire_h_
#define _omenquire_h_

#include "omtypes.h"
#include <vector>
#include <set>

class OMEnquireInternal; // Internal state of enquire
class OMMatch;           // Class which performs queries

///////////////////////////////////////////////////////////////////
// OMQuery class
// =============
// Class representing a query

typedef enum {
    OM_MOP_LEAF,      // For internal use - must never be specified as parameter

    OM_MOP_AND,       // Return iff both subqueries are satisfied
    OM_MOP_OR,        // Return if either subquery is satisfied
    OM_MOP_AND_NOT,   // Return if left but not right satisfied
    OM_MOP_XOR,       // Return if one query satisfied, but not both
    OM_MOP_AND_MAYBE, // Return iff left satisfied, but use weights from both
    OM_MOP_FILTER     // As AND, but use only weights from left subquery
} om_queryop;

class OMQuery {
    friend class OMMatch;
    private:
	vector<OMQuery *> subqs;
	termname tname;
	om_queryop op;

	void initialise_from_copy(const OMQuery &);
	void initialise_from_vector(const vector<OMQuery>::const_iterator,
				    const vector<OMQuery>::const_iterator);
    public:
	// A query consisting of a single term
	OMQuery(const termname &);

	// A query consisting of two subqueries, opp-ed together
	OMQuery(om_queryop, const OMQuery &, const OMQuery &);

	// A set of OMQuery's, merged together with specified operator.
	// The only operators allowed are AND and OR
	OMQuery(om_queryop, const vector<OMQuery> &);

	// As before, except subqueries are all individual terms.
	OMQuery(om_queryop, const vector<termname> &);

	// As before, but use begin and end iterators
	OMQuery(om_queryop,
		const vector<OMQuery>::const_iterator,
		const vector<OMQuery>::const_iterator);

	// Copy constructor
	OMQuery(const OMQuery &);

	// Assignment
	OMQuery & operator=(const OMQuery &);

	// Default constructor: creates a null query
	OMQuery();

	// Destructor
	~OMQuery();
};

///////////////////////////////////////////////////////////////////
// OMQueryOptions class
// =============
// Used to specify options for running a query

class OMQueryOptions {
    friend OMEnquireInternal;
    private:
	bool  do_collapse;
	keyno collapse_key;
    public:
	void set_collapse_key(keyno);
	void set_no_collapse();
	OMQueryOptions();
};

///////////////////////////////////////////////////////////////////
// OMRSet class
// =============
// Class representing a relevance set

class OMRSet {
    private:
    public:
	set<docid> reldocs;
	void add_document(docid);
	void remove_document(docid);
};

inline void
OMRSet::add_document(docid did)
{
    reldocs.insert(did);
}

inline void
OMRSet::remove_document(docid did)
{
    set<docid>::iterator i = reldocs.find(did);
    if(i != reldocs.end()) reldocs.erase(i);
}

///////////////////////////////////////////////////////////////////
// OMMSet class
// =============
// Class representing a match result set

// An item in the MSet
class OMMSetItem {
    friend class OMMatch;
    private:
	OMMSetItem(weight wt_new, docid did_new) : wt(wt_new), did(did_new) {}
    public:
	weight wt;
	docid did;
};

// Encapsulation of match set
class OMMSet {
    private:
    public:
	OMMSet() : mbound(0) {}
	vector<OMMSetItem> items;
	doccount mbound;
};

///////////////////////////////////////////////////////////////////
// OMEnquire class
// ===============
// This class provides an interface to the information retrieval
// system for the purpose of searching.

class OMEnquire {
    private:
	OMEnquireInternal *internal;
    public:
        OMEnquire();
        ~OMEnquire();

	// Add a new database to use.
	//
	// First parameter is a string describing the database type.
	// Second parameter is a vector of parameters to be used to open the
	// database: meaning and number required depends on database type.
	// Third parameter is a flag: if set, the database will be opened
	// read-only.
	void add_database(const string &,
			  const vector<string> &,
			  bool = true);

	// Set the query to run.
	void set_query(const OMQuery &);

	// Set the relevance set to use.
	void set_rset(const OMRSet &);

	// Set a key to collapse (remove duplicates) on
	void set_options(const OMQueryOptions &);

	// Get (a portion of) the match set for the current query
	void get_mset(OMMSet &, doccount first, doccount maxitems) const;
};

#endif /* _omenquire_h_ */
