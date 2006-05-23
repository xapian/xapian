/* msetpostlist.h: mset postlists from different databases
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OM_HGUARD_MSETPOSTLIST_H
#define OM_HGUARD_MSETPOSTLIST_H

#include <xapian/enquire.h>
#include "postlist.h"
#include "omdebug.h"

class RemoteSubMatch;
class NetworkDatabase;

/// A postlist taking postings from an already formed mset
class MSetPostList : public PostList {
    friend class RemoteSubMatch;
    private:
	// Prevent copying
	MSetPostList(const MSetPostList &);
	MSetPostList & operator=(const MSetPostList &);

	Xapian::MSet mset;
	NetworkDatabase *db;
	int current;

    public:
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::docid  get_docid() const;
	Xapian::weight get_weight() const;
	const string * get_collapse_key() const;

	Xapian::weight get_maxweight() const;

	Xapian::weight recalc_maxweight();

	PostList *next(Xapian::weight w_min);
	PostList *skip_to(Xapian::docid did, Xapian::weight w_min);
	bool   at_end() const;

	string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const;

	virtual PositionList * read_position_list();
	virtual PositionList * open_position_list() const;

	MSetPostList(const Xapian::MSet mset_, NetworkDatabase *db_);
	~MSetPostList();
};

/// Stands in for an MSetPostList until the MSet is available at which point
/// it prunes, returning an MSetPostList
class PendingMSetPostList : public PostList {
    friend class RemoteSubMatch;
    private:
	NetworkDatabase *db;
	MSetPostList *pl;
	Xapian::doccount maxitems;

	void make_pl();

    public:
	Xapian::doccount get_termfreq_max() const {
	    Assert(pl);
	    return pl->get_termfreq_max();
	}

	Xapian::doccount get_termfreq_min() const {
	    Assert(pl);
	    return pl->get_termfreq_min();
	}

	Xapian::doccount get_termfreq_est() const {
	    Assert(pl);
	    return pl->get_termfreq_est();
	}

	Xapian::docid  get_docid() const { Assert(false); return 0; }
	Xapian::weight get_weight() const { Assert(false); return 0; }
	Xapian::weight get_maxweight() const { Assert(false); return 0; }

	Xapian::weight recalc_maxweight() {
	    make_pl();
	    return pl->recalc_maxweight();
	}

	PostList *next(Xapian::weight w_min) {
	    make_pl();
	    PostList *pl2 = pl->next(w_min);
	    Assert(pl2 == NULL); // MSetPostList-s don't prune
	    pl2 = pl;
	    pl = NULL;
	    return pl2;
	}

	PostList *skip_to(Xapian::docid /*did*/, Xapian::weight /*w_min*/) {
	    // MSetPostList doesn't return documents in docid order, so skip_to
	    // isn't a meaningful operation.
	    throw Xapian::UnimplementedError("PendingMSetPostList doesn't support skip_to");
	}

	bool at_end() const { Assert(false); return true; }

	string get_description() const {
	    if (pl) return "PendingMset(" + pl->get_description() + ")";
	    return "PendingMSet()";
	}

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::doclength get_doclength() const { Assert(false); return 1; }

	virtual PositionList * read_position_list() { Assert(false); return 0; }
	virtual PositionList * open_position_list() const {
	    Assert(false);
	    return 0;
	}

	PendingMSetPostList(NetworkDatabase *db_, Xapian::doccount maxitems_)
		: db(db_), pl(NULL), maxitems(maxitems_) { }
	~PendingMSetPostList();
};

#endif /* OM_HGUARD_MSETPOSTLIST_H */
