/* match.h: class for performing a match
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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

#ifndef _match_h_
#define _match_h_

#include "database.h"
#include "omassert.h"

class IRWeight;

#include <stack>
#include <vector>

class MSetItem {
    public:
        weight wt;
        docid did;
        MSetItem(weight wt_new, docid did_new)
		: wt(wt_new), did(did_new)
		{ return; }
};

// Match operations
typedef enum {
    MOP_AND,
    MOP_OR,
    MOP_FILTER,
    MOP_AND_NOT,
    MOP_AND_MAYBE,
    MOP_XOR
} matchop;

class Match
{
    private:
        IRDatabase *database;

	matchop default_op;
   
        int min_weight_percent;
        weight max_weight;

	stack<PostList *> query;
	vector<IRWeight *> weights;

	RSet *rset;         // RSet to be used (affects weightings)

	bool do_collapse;   // Whether to perform collapsem operation
	keyno collapse_key; // Key to collapse on, if desired

	bool have_added_terms;
        bool recalculate_maxweight;

	bool query_ready; 
	PostList * build_query();

	DBPostList * mk_postlist(const termname& tname,
				 RSet * rset);
    public:
        Match(IRDatabase *);
        ~Match();

	///////////////////////////////////////////////////////////////////
	// Set the terms and operations which comprise the query
	// =====================================================
	
	// Add term to query stack
        void add_term(const termname &);

	// Apply operator to top items on stack
	bool add_op(matchop op);

	// Add list of terms op'd together to stack
	void add_oplist(matchop op, const vector<termname>&);

	// Set operator to use for any terms which are left over -
	// default is MOP_OR
	void set_default_op(matchop);

	///////////////////////////////////////////////////////////////////
	// Set additional options for performing the query
	// ===============================================

	// Set relevance information - the RSet object should not be
	// altered after this call
        void set_rset(RSet *);

	// Set cutoff at min percentage - defaults to -1, which means no cutoff
        void set_min_weight_percent(int);

	// Add a key number to collapse by.  Each key value will appear only
	// once in the result set.  Collapsing can only be done on one key
	// number.
	void set_collapse_key(keyno);

	// Remove the collapse key
	void set_no_collapse();

	///////////////////////////////////////////////////////////////////
	// Get information about result
	// ============================

	// Get an upper bound on the possible weights (unlikely to be attained)
        weight get_max_weight();

	// Perform the match operation, and get the matching items
	void match(doccount first,         // First item to return (start at 0)
		   doccount maxitems,      // Maximum number of items to return
		   vector<MSetItem> &      // Results will be put in this vector
		  );

	// Perform the match operation, but also return a lower bound on the
	// number of matching records in the database (mtotal).  Because of
	// some of the optimisations performed, this is likely to be much
	// lower than the actual number of matching records, but it is
	// expensive to do the calculation properly.
	//
	// It is generally considered that presenting the mtotal to users
	// causes them to worry about the large number of results, rather
	// than how useful those at the top of the mset are, and is thus
	// undesirable.
	void match(doccount first,         // First item to return (start at 0)
		   doccount maxitems,      // Maximum number of items to return
		   vector<MSetItem> &,     // Results will be put in this vector
		   doccount *);            // Mtotal will returned here

	///////////////////////////////////////////////////////////////////
	// Miscellaneous
	// =============

	// Called by postlists to indicate that they've rearranged themselves
	// and the maxweight now possible is smaller.
        void recalc_maxweight();
};

inline void
Match::set_default_op(matchop _default_op)
{
    default_op = _default_op;
}

inline void
Match::set_collapse_key(keyno key)
{
    do_collapse = true;
    collapse_key = key;
}

inline void
Match::set_no_collapse()
{
    do_collapse = false;
}

inline void
Match::set_rset(RSet *new_rset)
{
    Assert(!have_added_terms);
    query_ready = false;
    rset = new_rset;
}

inline void
Match::set_min_weight_percent(int pcent)
{
    min_weight_percent = pcent;
}

inline weight
Match::get_max_weight()
{
    (void) build_query();
    return max_weight;
}

#endif /* _match_h_ */
