/* msetpostlist.h: mset postlists from different databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#ifndef OM_HGUARD_MSETPOSTLIST_H
#define OM_HGUARD_MSETPOSTLIST_H

#include "database.h"
#include "net_database.h"
#include "omenquireinternal.h"

/// A postlist taking postings from an already formed mset
class MSetPostList : public PostList {
    friend class RemoteSubMatch;
    private:
        // Prevent copying
        MSetPostList(const MSetPostList &);
        MSetPostList & operator=(const MSetPostList &);

	OmMSet mset;    
	const NetworkDatabase *db;
	int current;

    public:
	om_doccount get_termfreq_max() const;
	om_doccount get_termfreq_min() const;
	om_doccount get_termfreq_est() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	const string * get_collapse_key() const;

	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

	virtual PositionList * read_position_list();
	virtual AutoPtr<PositionList> open_position_list() const;

        MSetPostList(const OmMSet mset_, const NetworkDatabase *db_);
        ~MSetPostList();
};

inline om_doccount
MSetPostList::get_termfreq_max() const
{
    DEBUGCALL(MATCH, om_doccount, "MSetPostList::get_termfreq_max", "");
    return mset.get_matches_upper_bound();
}

inline om_doccount
MSetPostList::get_termfreq_min() const
{
    DEBUGCALL(MATCH, om_doccount, "MSetPostList::get_termfreq_min", "");
    return mset.get_matches_lower_bound();
}

inline om_doccount
MSetPostList::get_termfreq_est() const
{
    DEBUGCALL(MATCH, om_doccount, "MSetPostList::get_termfreq_est", "");
    return mset.get_matches_estimated();
}

inline om_docid
MSetPostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "MSetPostList::get_docid", "");
    Assert(current != -1);
    RETURN(mset.internal->data->items[current].did);
}

inline om_weight
MSetPostList::get_weight() const
{
    DEBUGCALL(MATCH, om_weight, "MSetPostList::get_weight", "");
    Assert(current != -1);
    return mset.internal->data->items[current].wt;
}

inline const string *
MSetPostList::get_collapse_key() const
{
    DEBUGCALL(MATCH, string *, "MSetPostList::get_collapse_key", "");
    Assert(current != -1);
    return &(mset.internal->data->items[current].collapse_key);
}

inline om_weight
MSetPostList::get_maxweight() const
{
    DEBUGCALL(MATCH, om_weight, "MSetPostList::get_maxweight", "");
    // Before we've started, return max_possible...
    // FIXME: when current advances from -1 to 0, we should probably call
    // recalc_maxweight on the matcher...
    if (current == -1) return mset.get_max_possible();
    if (mset.empty()) return 0;
    // mset.max_attained is bigger than this if firstitem != 0
    return mset.internal->data->items[current].wt;
}

inline om_weight
MSetPostList::recalc_maxweight()
{
    DEBUGCALL(MATCH, om_weight, "MSetPostList::recalc_maxweight", "");
    return get_maxweight();
}

inline bool
MSetPostList::at_end() const
{
    DEBUGCALL(MATCH, bool, "MSetPostList::at_end", "");
    Assert(current != -1);
    return (unsigned int)current >= mset.size();
}

inline string
MSetPostList::get_description() const
{
    return "( MSet " + mset.get_description() + " )";
}

inline om_doclength
MSetPostList::get_doclength() const
{
    DEBUGCALL(MATCH, om_doclength, "MSetPostList::get_doclength", "");
    Assert(current != -1);
    return 1; // FIXME: this info is unused with present weights
//    return db->get_doclength(mset.internal->data->items[current].did);
}

inline PositionList *
MSetPostList::read_position_list()
{
    DEBUGCALL(MATCH, PositionList *, "MSetPostList::read_position_list", "");
    throw OmUnimplementedError("MSetPostList::read_position_list() unimplemented");
}

inline AutoPtr<PositionList>
MSetPostList::open_position_list() const
{
    throw OmUnimplementedError("MSetPostList::open_position_list() unimplemented");
}

class RemoteSubMatch;

/// Stands in for an MSetPostList until the MSet is available at which point
/// it prunes, returning an MSetPostList
class PendingMSetPostList : public PostList {
    friend class RemoteSubMatch;
    private:
	const NetworkDatabase *db;
	MSetPostList *pl;
	om_doccount maxitems;

	void make_pl() {
	    if (pl) return;
	    OmMSet mset;
	    while (!db->link->get_mset(0, maxitems, mset)) {
		db->link->wait_for_input();
	    }
	    pl = new MSetPostList(mset, db);
	}

    public:
	om_doccount get_termfreq_max() const {
	    Assert(pl);
	    return pl->get_termfreq_max();
	}

	om_doccount get_termfreq_min() const {
	    Assert(pl);
	    return pl->get_termfreq_min();
	}

	om_doccount get_termfreq_est() const {
	    Assert(pl);
	    return pl->get_termfreq_est();
	}

	om_docid  get_docid() const { Assert(false); return 0; }
	om_weight get_weight() const { Assert(false); return 0; }
	om_weight get_maxweight() const { Assert(false); return 0; }
	
        om_weight recalc_maxweight() {
	    make_pl();
	    return pl->recalc_maxweight();
	}

	PostList *next(om_weight w_min) {
	    make_pl();
	    PostList *pl2 = pl->next(w_min);
	    Assert(pl2 == NULL); // MSetPostList-s don't prune
	    pl2 = pl;
	    pl = NULL;
	    return pl2;
	}

	PostList *skip_to(om_docid /*did*/, om_weight /*w_min*/) {
	    // MSetPostList doesn't return documents in docid order, so skip_to
	    // isn't a meaningful operation.
	    throw OmUnimplementedError("PendingMSetPostList doesn't support skip_to");	    
	}

	bool at_end() const { Assert(false); return true; }

	string get_description() const {
	    if (pl) return "PendingMset(" + pl->get_description() + ")";
	    return "PendingMSet()";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const { Assert(false); return 1; }

	virtual PositionList * read_position_list() { Assert(false); return 0; }
	virtual AutoPtr<PositionList> open_position_list() const {
	    Assert(false); return AutoPtr<PositionList>(0);
	}

        PendingMSetPostList(const NetworkDatabase *db_, om_doccount maxitems_)
		: db(db_), pl(NULL), maxitems(maxitems_) { }
        ~PendingMSetPostList();
};

#endif /* OM_HGUARD_MSETPOSTLIST_H */
