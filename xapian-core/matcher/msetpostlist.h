/* msetpostlist.h: mset postlists from different databases
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

#ifndef OM_HGUARD_MSETPOSTLIST_H
#define OM_HGUARD_MSETPOSTLIST_H

#include "database.h"
#include "net_database.h"
#include "omenquireinternal.h"

/// A postlist taking postings from an already formed mset
class MSetPostList : public PostList {
    friend class RemoteSubMatch;
    private:
	OmMSet mset;    
	const NetworkDatabase *db;
	int current;

    public:
	om_doccount get_termfreq() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	const OmKey * get_collapse_key() const;

	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

	virtual PositionList * get_position_list();

        MSetPostList(const OmMSet mset_, const NetworkDatabase *db_);
        ~MSetPostList();
};

inline om_doccount
MSetPostList::get_termfreq() const
{
    // sum of termfreqs for all terms in query which formed mset
    om_doccount total = 0;
    static const std::map<om_termname, OmMSet::TermFreqAndWeight> &m =
	mset.get_all_terminfo();
    std::map<om_termname, OmMSet::TermFreqAndWeight>::const_iterator i;
    for (i = m.begin(); i != m.end(); i++) {
	total += i->second.termfreq;
    }
    return total;
}

inline om_docid
MSetPostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "MSetPostList::get_docid", "");
    Assert(current != -1);
    RETURN(mset.items[current].did);
}

inline om_weight
MSetPostList::get_weight() const
{
    Assert(current != -1);
    return mset.items[current].wt;
}

inline const OmKey *
MSetPostList::get_collapse_key() const
{
    Assert(current != -1);
    return &(mset.items[current].collapse_key);
}

inline om_weight
MSetPostList::get_maxweight() const
{
    // Before we've started, return max_possible...
    // FIXME: when current advances from -1 to 0, we should probably call
    // recalc_maxweight on the matcher...
    if (current == -1) return mset.max_possible;    
    if (mset.items.empty()) return 0;
    // mset.max_attained is bigger than this if firstitem != 0
    return mset.items[current].wt;
}

inline om_weight
MSetPostList::recalc_maxweight()
{
    return get_maxweight();
}

inline bool
MSetPostList::at_end() const
{
    Assert(current != -1);
    return (unsigned int)current >= mset.items.size();
}

inline std::string
MSetPostList::get_description() const
{
    return "( MSet " + mset.get_description() + " )";
}

inline om_doclength
MSetPostList::get_doclength() const
{
    Assert(current != -1);
    return 1; // FIXME: this info is unused with present weights
//    return db->get_doclength(mset.items[current].did);
}

inline PositionList *
MSetPostList::get_position_list()
{
    throw OmUnimplementedError("MSetPostList::get_position_list() unimplemented");
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
	// maxitems is an upper bound on the number of postings
	om_doccount get_termfreq() const { return maxitems; }

	om_docid  get_docid() const { Assert(false); }
	om_weight get_weight() const { Assert(false); }
	om_weight get_maxweight() const { Assert(false); }
	
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

	PostList *skip_to(om_docid did, om_weight w_min) {
	    // MSetPostList doesn't return documents in docid order, so skip_to
	    // isn't a meaningful operation.
	    throw OmUnimplementedError("PendingMSetPostList doesn't support skip_to");	    
	}

	bool at_end() const { Assert(false); }

	std::string get_description() const {
	    return "( PendingMSet )";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const { Assert(false); }

	virtual PositionList * get_position_list() { Assert(false); }

        PendingMSetPostList(const NetworkDatabase *db_, om_doccount maxitems_)
		: db(db_), pl(NULL), maxitems(maxitems_) { }
        ~PendingMSetPostList();
};

/// A postlist streamed across a network connection
class RemotePostList : public PostList {
    friend class RemoteSubMatch;
    private:
	const NetworkDatabase *db;
	om_docid did;
	om_weight w;
	OmKey key;
	
	om_doccount termfreq;
	om_weight maxw;

	std::map<om_termname, OmMSet::TermFreqAndWeight> term_info;

    public:
	om_doccount get_termfreq() const;

	om_docid  get_docid() const;
	om_weight get_weight() const;
	const OmKey * get_collapse_key() const;

	om_weight get_maxweight() const;

        om_weight recalc_maxweight();

	PostList *next(om_weight w_min);
	PostList *skip_to(om_docid did, om_weight w_min);
	bool   at_end() const;

	std::string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual om_doclength get_doclength() const;

	virtual PositionList * get_position_list();

	const std::map<om_termname, OmMSet::TermFreqAndWeight> get_terminfo() const {
	    return term_info;
	}

        RemotePostList(const NetworkDatabase *db_, om_doccount maxitems_);
        ~RemotePostList();
};

inline RemotePostList::RemotePostList(const NetworkDatabase *db_, om_doccount maxitems)
    : db(db_), did(0)
{
    DEBUGCALL(MATCH, void, "RemotePostList::RemotePostList", db_ << ", " << maxitems);
    db->link->open_postlist(0, maxitems, termfreq, maxw, term_info);
}

inline RemotePostList::~RemotePostList()
{
    DEBUGCALL(MATCH, void, "RemotePostList::~RemotePostList", "");
}

inline PostList *
RemotePostList::next(om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "RemotePostList::next", w_min);
    db->link->next(w_min, did, w, key);
    RETURN(NULL);
}

inline PostList *
RemotePostList::skip_to(om_docid new_did, om_weight w_min)
{
    DEBUGCALL(MATCH, PostList *, "RemotePostList::skip_to", new_did << ", " << w_min);
    if (new_did > did) db->link->skip_to(new_did, w_min, did, w, key);
    RETURN(NULL);
}

inline om_doccount
RemotePostList::get_termfreq() const
{
    return termfreq;
}

inline om_docid
RemotePostList::get_docid() const
{
    DEBUGCALL(MATCH, om_docid, "RemotePostList::get_docid", "");
    Assert(did != 0);    
    RETURN(did);
}

inline om_weight
RemotePostList::get_weight() const
{
    Assert(did != 0);
    return w;
}

inline const OmKey *
RemotePostList::get_collapse_key() const
{
    Assert(did != 0);
    return &key;
}

inline om_weight
RemotePostList::get_maxweight() const
{
    // FIXME: should decrease as postlist progresses
    return maxw;
}

inline om_weight
RemotePostList::recalc_maxweight()
{
    // FIXME: send over network
    return get_maxweight();
}

inline bool
RemotePostList::at_end() const
{
    return (did == 0);
}

inline std::string
RemotePostList::get_description() const
{
    return "( Remote )";
}

inline om_doclength
RemotePostList::get_doclength() const
{
    Assert(did != 0);
    return 1; // FIXME: this info is unused with present weights
//    return db->get_doclength(mset.items[current].did);
}

inline PositionList *
RemotePostList::get_position_list()
{
    throw OmUnimplementedError("RemotePostList::get_position_list() unimplemented");
}

#endif /* OM_HGUARD_MSETPOSTLIST_H */
