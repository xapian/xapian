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
	: left(NULL), right(NULL), tname(_tname)
{}

OMQuery::OMQuery(om_queryop _op, const OMQuery &query1, const OMQuery &query2)
	: op(_op)
{
    left = new OMQuery(query1);
    right = new OMQuery(query2);
}

OMQuery::OMQuery(om_queryop _op, const vector<OMQuery> &subqueries)
	: op(_op)
{
    Assert(op == OM_MOP_AND || op == OM_MOP_OR);
    initialise_from_vector(subqueries.begin(), subqueries.end());
}

OMQuery::OMQuery(om_queryop _op, const vector<OMQuery>::const_iterator qbegin,
		 const vector<OMQuery>::const_iterator qend)
	        : op(_op)
{   
    Assert(op == OM_MOP_AND || op == OM_MOP_OR);
    initialise_from_vector(qbegin, qend);
}

OMQuery::OMQuery(om_queryop _op, const vector<termname> &terms)
	: op(_op)
{
    Assert(op == OM_MOP_AND || op == OM_MOP_OR);

    vector<OMQuery> subqueries;
    vector<termname>::const_iterator i;
    for(i = terms.begin(); i != terms.end(); i++) {
	subqueries.push_back(OMQuery(*i));
    }
    initialise_from_vector(subqueries.begin(), subqueries.end());
}

OMQuery::OMQuery(const OMQuery &copyme)
{
    initialise_from_copy(copyme);
}

OMQuery::OMQuery(const OMQuery *copyme)
{
    initialise_from_copy(*copyme);
}

OMQuery::OMQuery()
	: left(NULL), right(NULL), tname("")
{
}

OMQuery::~OMQuery()
{
    delete left;
    delete right;
}

void
OMQuery::initialise_from_copy(const OMQuery &copyme)
{
    tname = copyme.tname;
    op    = copyme.op;
    if(copyme.left != NULL) {
	left = new OMQuery(*(copyme.left));
	right = new OMQuery(*(copyme.right));
    } else {
	left = NULL;
	right = NULL;
    }
}

void
OMQuery::initialise_from_vector(const vector<OMQuery>::const_iterator qbegin,
				const vector<OMQuery>::const_iterator qend)
{
    if(qbegin == qend) {
	left = NULL;
	right = NULL;
	tname = "";
    } else if(qbegin + 1 == qend) {
	// Copy into self
	initialise_from_copy(*qbegin);
    } else {
	left = new OMQuery(*qbegin);
	right = new OMQuery(op, qbegin + 1, qend);
    }
}

/////////////////////////
// Methods for OMQuery //
/////////////////////////

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
	OMQuery * query;
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
    om_database_type dbtype = stringToTypeMap<om_database_type>::get_type(type);

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
