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

#include <queue>
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

typedef enum { AND, OR, FILTER, AND_NOT, AND_MAYBE, XOR } matchop;

class Match {
    private:
        IRDatabase *DB;
   
        doccount max_msize;
        int min_weight_percent;
        weight max_weight;

	stack<PostList *> q;
	vector<IRWeight *> weights;

        PostList *merger;
	RSet *rset;
    
	bool have_added_terms;
        bool recalculate_maxweight;

	DBPostList * mk_postlist(IRDatabase *DB,
				 const termname& tname,
				 RSet * rset);
    public:
        Match(IRDatabase *);
        ~Match();
        void add_term(const termname &);
	bool add_op(matchop op);
	void add_oplist(matchop op, const vector<termname>&);

        void match();
        void set_max_msize(doccount n);
        void set_rset(RSet *new_rset);
        weight get_max_weight();
        void set_min_weight_percent(int pcent);
        void recalc_maxweight();

        vector<MSetItem> mset;
        doccount msize;
        doccount mtotal;
};

inline void
Match::set_rset(RSet *new_rset)
{
    Assert(!have_added_terms);
    rset = new_rset;
}

inline void
Match::set_max_msize(doccount n)
{
    max_msize = n;
}

inline void
Match::set_min_weight_percent(int pcent)
{
    min_weight_percent = pcent;
}

inline weight
Match::get_max_weight()
{
    return max_weight;
}

#endif /* _match_h_ */
