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
#include <string>
#include <vector>
#include <set>

class OMEnquireInternal; // Internal state of enquire
class OMEnquire;         // Declare Enquire class
class OMMatch;           // Class which performs queries

///////////////////////////////////////////////////////////////////
// OMQuery class
// =============
// Representation of a query

// Enum of possible query operations
typedef enum {
    OM_MOP_LEAF,      // For internal use - must never be specified as parameter

    OM_MOP_AND,       // Return iff both subqueries are satisfied
    OM_MOP_OR,        // Return if either subquery is satisfied
    OM_MOP_AND_NOT,   // Return if left but not right satisfied
    OM_MOP_XOR,       // Return if one query satisfied, but not both
    OM_MOP_AND_MAYBE, // Return iff left satisfied, but use weights from both
    OM_MOP_FILTER     // As AND, but use only weights from left subquery
} om_queryop;

// Class representing a query
class OMQuery {
    friend class OMMatch;
    private:
	bool isnull;
	vector<OMQuery *> subqs;
	termname tname;
	om_queryop op;

	void initialise_from_copy(const OMQuery &);
	void initialise_from_vector(const vector<OMQuery>::const_iterator,
				    const vector<OMQuery>::const_iterator);
	void initialise_from_vector(const vector<OMQuery *>::const_iterator,
				    const vector<OMQuery *>::const_iterator);
    public:
	// A query consisting of a single term
	OMQuery(const termname &);

	// A query consisting of two subqueries, opp-ed together
	OMQuery(om_queryop, const OMQuery &, const OMQuery &);

	// A set of OMQuery's, merged together with specified operator.
	// (Takes begin and end iterators).
	// The only operators allowed are AND and OR.
	OMQuery(om_queryop,
		const vector<OMQuery>::const_iterator,
		const vector<OMQuery>::const_iterator);

	OMQuery(om_queryop,
		const vector<OMQuery *>::const_iterator,
		const vector<OMQuery *>::const_iterator);

	// As before, except subqueries are all individual terms.
	OMQuery(om_queryop,
		const vector<termname>::const_iterator,
		const vector<termname>::const_iterator);

	// Copy constructor
	OMQuery(const OMQuery &);

	// Assignment
	OMQuery & operator=(const OMQuery &);

	// Default constructor: makes a null query which can't be used
	// (Convenient to have a default constructor though)
	OMQuery();

	// Destructor
	~OMQuery();

	// Introspection method
	string get_description() const;
};

///////////////////////////////////////////////////////////////////
// OMMatchOptions class
// ====================
// Used to specify options for running a query

class OMMatchOptions {
    friend OMEnquire;
    private:
	bool  do_collapse;
	keyno collapse_key;

	bool  sort_forward;
    public:
	void set_collapse_key(keyno);
	void set_no_collapse();
	void set_sort_forward(bool = true);
	OMMatchOptions();
};

///////////////////////////////////////////////////////////////////
// OMExpandOptions class
// =====================
// Used to specify options for performing expand

class OMExpandOptions {
    friend OMEnquire;
    private:
    public:
	OMExpandOptions();
};

///////////////////////////////////////////////////////////////////
// OMRSet class
// =============
// Class representing a relevance set

class OMRSet {
    private:
    public:
	set<docid> items;
	void add_document(docid);
	void remove_document(docid);
};

inline void
OMRSet::add_document(docid did)
{
    items.insert(did);
}

inline void
OMRSet::remove_document(docid did)
{
    set<docid>::iterator i = items.find(did);
    if(i != items.end()) items.erase(i);
}

///////////////////////////////////////////////////////////////////
// OMMSet class
// =============
// Representaton of a match result set

// An item in the MSet
class OMMSetItem {
    friend class OMMatch;
    private:
	OMMSetItem(weight wt_new, docid did_new) : wt(wt_new), did(did_new) {}
    public:
	weight wt;
	docid did;
};

// Class representing an MSet
class OMMSet {
    private:
    public:
	OMMSet() : mbound(0) {}
	// FIXME - implement convert_to_percent
	int convert_to_percent(const OMMSetItem &) const;
	int convert_to_percent(weight) const;
	vector<OMMSetItem> items;
	doccount mbound;
	weight max_weight;
};

///////////////////////////////////////////////////////////////////
// OMESet class
// =============
// Representation a set of expand terms

// An item in the ESet
class OMESetItem {
    friend class OMExpand;
    private:
	OMESetItem(weight wt_new, termname tname_new)
		: wt(wt_new), tname(tname_new) {}
    public:
	weight wt;
	termname tname;
};

// Class representing an ESet
class OMESet {
    private:
    public:
	OMESet() : etotal(0) {}
	vector<OMESetItem> items;
	termcount etotal;
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
	//
	// The database will always be opened read-only.
	void add_database(const string &,
			  const vector<string> &);

	// Set the query to run.
	void set_query(const OMQuery &);

	// Get (a portion of) the match set for the current query
	void get_mset(OMMSet &,
                      doccount first,
                      doccount maxitems,
		      const OMRSet * = 0,
	              const OMMatchOptions * = 0) const;

	// Get the expand set for the given rset
	void get_eset(OMESet &,
                      termcount maxitems,
                      const OMRSet &,
                      const OMExpandOptions * = 0) const;
};

#endif /* _omenquire_h_ */
