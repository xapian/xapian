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
#include "expand.h"
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

OMQuery::OMQuery(const termname & tname_)
	: isnull(false), tname(tname_), op(OM_MOP_LEAF)
{
}

OMQuery::OMQuery(om_queryop op_, const OMQuery &left, const OMQuery &right)
	: isnull(false), op(op_)
{
    Assert(op != OM_MOP_LEAF); // FIXME throw exception rather than Assert
    // Handle null sub-queries.
    // See documentation for table for result of operations when one of the
    // operands is null:
    //
    if(left.isnull || right.isnull) {
	switch (op) {
	    case OM_MOP_OR:
	    case OM_MOP_AND:
	    case OM_MOP_FILTER:
		if (!left.isnull) {
		    initialise_from_copy(left);
		} else {
		    if (!right.isnull) initialise_from_copy(right);
		    else isnull = true;
		}
		break;
	    case OM_MOP_AND_MAYBE:
		if (!left.isnull) {
		    initialise_from_copy(left);
		} else {
		    isnull = true;
		}
		break;
	    case OM_MOP_AND_NOT:
		if (!left.isnull) {
		    initialise_from_copy(left);
		} else {
		    if (!right.isnull) Assert(false); // FIXME: throw exception
		    else isnull = true;
		}
		break;
	    case OM_MOP_XOR:
		if (left.isnull && right.isnull) {
		    isnull = true;
		} else {
		    Assert(false); // FIXME: throw exception
		}
		break;
	    case OM_MOP_LEAF:
		Assert(false); // Shouldn't have got this far
	}
    } else {
	DebugMsg(" (" << left.get_description() << " " << (int) op <<
		 " " << right.get_description() << ") => ");

	// If sub query has same op, which is OR or AND, add to list rather
	// than makeing sub-node.  Can then optimise the list at search time.
	if(op == OM_MOP_AND || op == OM_MOP_OR) {
	    if(left.op == op && right.op == op) {
		// Both queries have same operation as top
		initialise_from_copy(left);
		vector<OMQuery *>::const_iterator i;
		for(i = right.subqs.begin(); i != right.subqs.end(); i++) {
		    subqs.push_back(new OMQuery(**i));
		}
	    } else if(left.op == op) {
		// Query2 has different operation (or is a leaf)
		initialise_from_copy(left);
		subqs.push_back(new OMQuery(right));
	    } else if(right.op == op) { // left has different operation
		// Query1 has different operation (or is a leaf)
		initialise_from_copy(right);
		subqs.push_back(new OMQuery(left));
		// FIXME: this puts the
		// query in an different order from that entered.
	    } else {
		subqs.push_back(new OMQuery(left));
		subqs.push_back(new OMQuery(right));
	    }
	} else {
	    subqs.push_back(new OMQuery(left));
	    subqs.push_back(new OMQuery(right));
	}
	DebugMsg(get_description() << endl);
    }
}

OMQuery::OMQuery(om_queryop op_,
		 const vector<OMQuery *>::const_iterator qbegin,
		 const vector<OMQuery *>::const_iterator qend)
	: isnull(false), op(op_)
{   
    initialise_from_vector(qbegin, qend);
}

OMQuery::OMQuery(om_queryop op_,
		 const vector<OMQuery>::const_iterator qbegin,
		 const vector<OMQuery>::const_iterator qend)
	: isnull(false), op(op_)
{   
    initialise_from_vector(qbegin, qend);
}

OMQuery::OMQuery(om_queryop op_,
		 const vector<termname>::const_iterator tbegin,
		 const vector<termname>::const_iterator tend)
	: isnull(false), op(op_)
{
    vector<OMQuery> subqueries;
    vector<termname>::const_iterator i;
    for(i = tbegin; i != tend; i++) {
	subqueries.push_back(OMQuery(*i));
    }
    initialise_from_vector(subqueries.begin(), subqueries.end());
}

// Copy constructor
OMQuery::OMQuery(const OMQuery & copyme)
{
    initialise_from_copy(copyme);
}

// Assignment
OMQuery &
OMQuery::operator=(const OMQuery & copyme)
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
	for(i = subqs.begin(); i != subqs.end(); i++) {
	    delete *i;
	}
	subqs.clear();
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

// Introspection
string
OMQuery::get_description() const
{
    if(isnull) return "<NULL>";
    string opstr;
    switch(op) {
	case OM_MOP_LEAF:
		return tname;
		break;
	case OM_MOP_AND:
		opstr = " AND ";
		break;
	case OM_MOP_OR:
		opstr = " OR ";
		break;
	case OM_MOP_FILTER:
		opstr = " FILTER ";
		break;
	case OM_MOP_AND_MAYBE:
		opstr = " AND_MAYBE ";
		break;
	case OM_MOP_AND_NOT:
		opstr = " AND_NOT ";
		break;
	case OM_MOP_XOR:
		opstr = " XOR ";
		break;
    }
    string description;
    vector<OMQuery *>::const_iterator i;
    for(i = subqs.begin(); i != subqs.end(); i++) {
	if(description.size()) description += opstr;
	description += (**i).get_description();
    }
    return "(" + description + ")";
}

////////////////////////////////
// Methods for OMMatchOptions //
////////////////////////////////

OMMatchOptions::OMMatchOptions()
	: do_collapse(false),
	  sort_forward(true)
{}

void
OMMatchOptions::set_collapse_key(keyno key_)
{
    do_collapse = true;
    collapse_key = key_;
}

void
OMMatchOptions::set_no_collapse()
{
    do_collapse = false;
}

void
OMMatchOptions::set_sort_forward(bool forward_)
{
    sort_forward = forward_;
}

/////////////////////////////////
// Internals of enquire system //
/////////////////////////////////

class OMEnquireInternal {
    public:
	IRDatabase * database;
	mutable OMQuery * query;

	OMEnquireInternal();
	~OMEnquireInternal();

	void add_database(IRDatabase * database_);
	void set_query(const OMQuery & query_);
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
OMEnquireInternal::add_database(IRDatabase * database_)
{
    // FIXME (and in destructor): actually add database, rather than replace
    if(database) delete database;
    database = database_;
}

inline void
OMEnquireInternal::set_query(const OMQuery &query_)
{
    if(query) {
	delete query;
	query = NULL;
    }
    query = new OMQuery(query_);
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
			const vector<string> & params)
{
    // Convert type into an om_database_type
    om_database_type dbtype = OM_DBTYPE_NULL;
    dbtype = stringToTypeMap<om_database_type>::get_type(type);

    // Prepare dbparams to build database with (open it readonly)
    DatabaseBuilderParams dbparams(dbtype, true);
    dbparams.paths = params;

    // Use dbparams to create database, and add it to the list of databases
    internal->add_database(DatabaseBuilder::create(dbparams));
}

void
OMEnquire::set_query(const OMQuery & query_)
{
    internal->set_query(query_);
}

void
OMEnquire::get_mset(OMMSet &mset,
                    doccount first,
                    doccount maxitems,
                    const OMRSet *omrset,
                    const OMMatchOptions *moptions) const
{
    Assert(internal->database != NULL);
    Assert(internal->query != NULL);

    // Use default options if none supplied
    OMMatchOptions defmoptions;
    if (moptions == 0) {
        moptions = &defmoptions;
    }

    // Set Database
    OMMatch match(internal->database);

    // Set Rset
    RSet *rset = 0;
    if((omrset != 0) && (omrset->items.size() != 0)) {
	rset = new RSet(internal->database, *omrset);
	match.set_rset(rset);
    }

    // Set options
    if(moptions->do_collapse) {
	match.set_collapse_key(moptions->collapse_key);
    }

    // Set Query
    match.set_query(internal->query);

    // Run query and get results into supplied OMMSet object
    match.match(first, maxitems, mset.items, msetcmp_forward, &(mset.mbound));

    // Get max weight for an item in the MSet
    mset.max_weight = match.get_max_weight();

    // Clear up
    delete rset;
}

void
OMEnquire::get_eset(OMESet & eset,
	            termcount maxitems,
                    const OMRSet & omrset,
	            const OMExpandOptions * eoptions) const
{
    OMExpand expand(internal->database);
    RSet rset(internal->database, omrset);

    DebugMsg("rset size is " << rset.get_rsize() << endl);

    OMExpandDeciderAlways expanddecider;
    
    //  FIXME: only accept maxitems
    expand.expand(eset, &rset, &expanddecider);
}
