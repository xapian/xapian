/* enquire.cc: External interface for running queries
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

#include "omerror.h"
#include "omassert.h"
#include "omenquire.h"

#include "rset.h"
#include "match.h"
#include "database.h"
#include "database_builder.h"
#include "irdocument.h"

#include <vector>

///////////////////////////////////////////////////////////////////
// Mapping of database types as strings to enum om_database_type //
///////////////////////////////////////////////////////////////////

template<class X> struct stringToType {
    string name;
    X type;
};

// Note: this just uses a list of entrys, and searches linearly through
// them.  Could at make this do a binary chop, but probably not worth
// doing so, unless list gets large.  Only used when openeing a database,
// after all.
template<class X> class stringToTypeMap {
    public:
	static stringToType<X> types[];
	static X get_type(string needle) {
	    stringToType<X>* haystack = types;
	    while(haystack->name.size() != 0) {
		if(haystack->name == needle) break;
		haystack++;
	    }
	    return haystack->type;
	}
};

// Table of names of database types
stringToType<om_database_type> stringToTypeMap<om_database_type>::types[] = {
    { "da_flimsy",		OM_DBTYPE_DA		},
    { "inmemory",		OM_DBTYPE_INMEMORY	},
    { "multidb",		OM_DBTYPE_MULTI		},
    { "sleepycat",		OM_DBTYPE_SLEEPY	},
    { "",			OM_DBTYPE_NULL		}  // End
};

/////////////////////////
// Methods for OMQuery //
/////////////////////////

OMQuery::OMQuery(const termname & _tname)
	: isnull(false), tname(_tname), op(OM_MOP_LEAF)
{
}

OMQuery::OMQuery(om_queryop _op, const OMQuery &query1, const OMQuery &query2)
	: isnull(false), op(_op)
{
    Assert(op != OM_MOP_LEAF); // FIXME throw exception rather than Assert
    // FIXME; if sub query has same op, which is OR or AND, add to list
 
    // Handle null sub-queries.
    // Table for result of operations when one of the operands is null:
    //
    //            | a,b                    | a,0 | 0,b | 0,0 |
    // -----------+------------------------+-----+-----+-----+
    // OR         | a or b                 |  a  |  b  |  0  |
    // AND        | a and b                |  a  |  b  |  0  |
    // FILTER     | a and b                |  a  |  b  |  0  |
    // AND_MAYBE  | a (weights from b)     |  a  |  0  |  0  |
    // AND_NOT    | a but not b            |  a  | n/a |  0  |
    // XOR        | (a or b) not (a and b) | n/a | n/a |  0  |
    // -----------+------------------------+-----+-----+-----+
    if(query1.isnull || query2.isnull) {
	switch (op) {
	    case OM_MOP_OR:
	    case OM_MOP_AND:
	    case OM_MOP_FILTER:
		if (!query1.isnull) {
		    initialise_from_copy(query1);
		} else {
		    if (!query2.isnull) initialise_from_copy(query2);
		    else isnull = true;
		}
		break;
	    case OM_MOP_AND_MAYBE:
		if (!query1.isnull) {
		    initialise_from_copy(query1);
		} else {
		    isnull = true;
		}
		break;
	    case OM_MOP_AND_NOT:
		if (!query1.isnull) {
		    initialise_from_copy(query1);
		} else {
		    if (!query2.isnull) Assert(false); // FIXME: throw exception
		    else isnull = true;
		}
		break;
	    case OM_MOP_XOR:
		if (query1.isnull && query2.isnull) {
		    isnull = true;
		} else {
		    Assert(false); // FIXME: throw exception
		}
		break;
	    case OM_MOP_LEAF:
		Assert(false); // Shouldn't have got this far
	}
    } else {
	subqs.push_back(new OMQuery(query1));
	subqs.push_back(new OMQuery(query2));
    }
}

OMQuery::OMQuery(om_queryop _op,
		 const vector<OMQuery *>::const_iterator qbegin,
		 const vector<OMQuery *>::const_iterator qend)
	: isnull(false), op(_op)
{   
    initialise_from_vector(qbegin, qend);
}

OMQuery::OMQuery(om_queryop _op,
		 const vector<OMQuery>::const_iterator qbegin,
		 const vector<OMQuery>::const_iterator qend)
	: isnull(false), op(_op)
{   
    initialise_from_vector(qbegin, qend);
}

OMQuery::OMQuery(om_queryop _op,
		 const vector<termname>::const_iterator tbegin,
		 const vector<termname>::const_iterator tend)
	: isnull(false), op(_op)
{
    vector<OMQuery> subqueries;
    vector<termname>::const_iterator i;
    for(i = tbegin; i != tend; i++) {
	subqueries.push_back(OMQuery(*i));
    }
    initialise_from_vector(subqueries.begin(), subqueries.end());
}

// Copy constructor
OMQuery::OMQuery(const OMQuery &copyme)
{
    initialise_from_copy(copyme);
}

// Assignment
OMQuery &
OMQuery::operator=(const OMQuery &copyme)
{
    initialise_from_copy(copyme);
    return *this;
}

// Default constructor
OMQuery::OMQuery()
	: isnull(true)
{}

// Destructor
OMQuery::~OMQuery()
{
    vector<OMQuery *>::const_iterator i;
    for(i = subqs.begin(); i != subqs.end(); i++) {
	delete *i;
    }
    subqs.clear();
}

// Copy an OMQuery object into self
void
OMQuery::initialise_from_copy(const OMQuery &copyme)
{
    isnull = copyme.isnull;
    op = copyme.op;
    if(op == OM_MOP_LEAF) {
	tname = copyme.tname;
    } else {
	vector<OMQuery *>::const_iterator i;
	for(i = copyme.subqs.begin(); i != copyme.subqs.end(); i++) {
	    subqs.push_back(new OMQuery(**i));
	}
    }
}

void
OMQuery::initialise_from_vector(const vector<OMQuery>::const_iterator qbegin,
				const vector<OMQuery>::const_iterator qend)
{
    Assert(op == OM_MOP_AND || op == OM_MOP_OR); // FIXME throw exception rather than Assert

    vector<OMQuery>::const_iterator i;
    for(i = qbegin; i != qend; i++) {
	if(!i->isnull) subqs.push_back(new OMQuery(*i));
    }

    if(subqs.size() == 0) {
	isnull = true;
    } else if(subqs.size() == 1) {
	// Should just have copied into self
	OMQuery * copyme = subqs[0];
	subqs.clear();
	initialise_from_copy(*copyme);
	delete copyme;
    }
}

// FIXME: this function generated by cut and paste of previous: use a template?
void
OMQuery::initialise_from_vector(const vector<OMQuery *>::const_iterator qbegin,
				const vector<OMQuery *>::const_iterator qend)
{
    Assert(op == OM_MOP_AND || op == OM_MOP_OR); // FIXME throw exception rather than Assert

    vector<OMQuery *>::const_iterator i;
    for(i = qbegin; i != qend; i++) {
	if(!(*i)->isnull) subqs.push_back(new OMQuery(**i));
    }

    if(subqs.size() == 0) {
	isnull = true;
    } else if(subqs.size() == 1) {
	// Should just have copied into self
	OMQuery * copyme = subqs[0];
	subqs.clear();
	initialise_from_copy(*copyme);
	delete copyme;
    }
}

////////////////////////////////
// Methods for OMQueryOptions //
////////////////////////////////

OMQueryOptions::OMQueryOptions()
	: do_collapse(false)
{}

void
OMQueryOptions::set_collapse_key(keyno _key)
{
    do_collapse = true;
    collapse_key = _key;
}

void
OMQueryOptions::set_no_collapse()
{
    do_collapse = false;
}

/////////////////////////////////
// Internals of enquire system //
/////////////////////////////////

class OMEnquireInternal {
    public:
	IRDatabase * database;
	mutable OMQuery * query;
	OMQueryOptions options;
	OMRSet omrset;

	OMEnquireInternal();
	~OMEnquireInternal();

	void add_database(IRDatabase *);
	void set_query(const OMQuery &);
	void set_rset(const OMRSet &);
	void set_options(const OMQueryOptions &);
	void get_mset(OMMSet &, doccount, doccount) const;
};

//////////////////////////////////////////
// Inline methods for OMEnquireInternal //
//////////////////////////////////////////

inline
OMEnquireInternal::OMEnquireInternal()
	: database(NULL), query(NULL)
{
}

inline
OMEnquireInternal::~OMEnquireInternal()
{
    add_database(NULL); // FIXME
    if(query) {
	delete query;
	query = NULL;
    }
}

inline void
OMEnquireInternal::add_database(IRDatabase * _database)
{
    // FIXME (and in destructor): actually add database, rather than replace
    if(database) delete database;
    database = _database;
}

inline void
OMEnquireInternal::set_query(const OMQuery &_query)
{
    if(query) {
	delete query;
	query = NULL;
    }
    query = new OMQuery(_query);
}

void
OMEnquireInternal::set_rset(const OMRSet &_rset)
{
    omrset = _rset;
}

void
OMEnquireInternal::set_options(const OMQueryOptions &_options)
{
    options = _options;
}

void
OMEnquireInternal::get_mset(OMMSet &mset,
			    doccount first, doccount maxitems) const
{
    Assert(database != NULL);
    Assert(query != NULL);

    // Set Database
    OMMatch match(database);

    // Set Rset
    if(omrset.reldocs.size() != 0) {
	RSet *rset = new RSet(database, omrset);
	match.set_rset(rset);
    }

    // Set options
    if(options.do_collapse) {
	match.set_collapse_key(options.collapse_key);
    }

    // Set Query
    match.set_query(query);

    // Run query and get results into supplied OMMSet object
    match.match(first, maxitems, mset.items, msetcmp_forward, &(mset.mbound));
}

////////////////////////////////////////////
// Initialise and delete OMEnquire object //
////////////////////////////////////////////

OMEnquire::OMEnquire()
{
    internal = new OMEnquireInternal();
}

OMEnquire::~OMEnquire()
{
    delete internal;
    internal = NULL;
}

//////////////////
// Set database //
//////////////////

void
OMEnquire::add_database(const string & type,
			const vector<string> & entries,
			bool readonly)
{
    // Convert type into an om_database_type
    om_database_type dbtype = OM_DBTYPE_NULL;
    dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Prepare params to build database with
    DatabaseBuilderParams params(dbtype, readonly);
    params.paths = entries;

    // Use params to create database, and add it to the list of databases
    internal->add_database(DatabaseBuilder::create(params));
}

void
OMEnquire::set_query(const OMQuery &query)
{
    internal->set_query(query);
}

void
OMEnquire::set_rset(const OMRSet &rset)
{
    internal->set_rset(rset);
}

void
OMEnquire::set_options(const OMQueryOptions &opts)
{
    internal->set_options(opts);
}

void
OMEnquire::get_mset(OMMSet &mset, doccount first, doccount maxitems) const
{
    internal->get_mset(mset, first, maxitems);
}
