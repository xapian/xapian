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

#include <vector>

///////////////////////////////////////////////////////////////////
// OMQuery class
// =============
// Class representing a query

typedef enum {
    OM_MOP_AND,       // Return document only if both subqueries are satisfied
    OM_MOP_OR,        // Return document if either subquery is satisfied
    OM_MOP_AND_NOT,   // Return document if first query but not second satisfied
    OM_MOP_XOR,       // Return document if one query satisfied, but not both
    OM_MOP_AND_MAYBE, // Return document iff 
    OM_MOP_FILTER     // Return document only if 
} om_queryop;

class OMQuery {
    private:
	OMQuery *left;
	OMQuery *right;
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

	// Convenience functions:
	// A set of OMQuery's, merged together with specified operator.
	// The only operators allowed are AND and OR
	OMQuery(om_queryop, const vector<OMQuery> &);

	// As before, except subqueries are all individual terms.
	OMQuery(om_queryop, const vector<termname> &);

	// As before, but use begin and end iterators
	OMQuery(om_queryop,
		const vector<OMQuery>::const_iterator,
		const vector<OMQuery>::const_iterator);

	// Copy constructors
	OMQuery(const OMQuery &);
	OMQuery(const OMQuery *);

	// Destructor
	~OMQuery();
};

///////////////////////////////////////////////////////////////////
// OMEnquire class
// ===============
// This class provides an interface to the information retrieval
// system for the purpose of searching.

class OMEnquireState; // Internal state
class OMEnquire {
    private:
	OMEnquireState *state;
    public:
        OMEnquire();
        ~OMEnquire();

	// Set the database to use.
	//
	// First parameter is a string describing the database type.
	// Second parameter is a vector of parameters to be used to open the
	// database: meaning and number required depends on database type.
	// Third parameter is a flag: if set, the database will be opened
	// read-only.
	void set_database(const string &,
			  const vector<string> &,
			  bool = true);

	// Set the query to run.
	void set_query(const OMQuery &);
};

#endif /* _omenquire_h_ */
