/** @file mergepostlist.h
 * @brief merge postlists from different databases
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2009,2011,2015,2016 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_MERGEPOSTLIST_H
#define OM_HGUARD_MERGEPOSTLIST_H

#include "api/postlist.h"

class MultiMatch;
class ValueStreamDocument;

/** A postlist comprising postlists from different databases merged together.
 */
class MergePostList : public PostList {
    private:
	// Prevent copying
	MergePostList(const MergePostList &);
	MergePostList & operator=(const MergePostList &);

	double w_max;

	vector<PostList *> plists;

	int current;

	/** The object which is using this postlist to perform
	 *  a match.  This object needs to be notified when the
	 *  tree changes such that the maximum weights need to be
	 *  recalculated.
	 */
	MultiMatch *matcher;

	/** Document proxy used for valuestream caching.
	 *
	 *  We need to notify this when the subdatabase changes, as then the
	 *  cached valuestreams need to be cleared as they will be for the
	 *  wrong subdatabase.
	 */
	ValueStreamDocument & vsdoc;

    public:
	Xapian::termcount get_wdf() const;
	Xapian::doccount get_termfreq_max() const;
	Xapian::doccount get_termfreq_min() const;
	Xapian::doccount get_termfreq_est() const;

	Xapian::docid get_docid() const;
	double get_weight() const;
	const string * get_sort_key() const;
	const string * get_collapse_key() const;

	double get_maxweight() const;

	double recalc_maxweight();

	PostList *next(double w_min);
	PostList *skip_to(Xapian::docid did, double w_min);
	bool at_end() const;

	string get_description() const;

	/** Return the document length of the document the current term
	 *  comes from.
	 */
	virtual Xapian::termcount get_doclength() const;

	/** Return the number of unique terms in the document. */
	virtual Xapian::termcount get_unique_terms() const;

	Xapian::termcount count_matching_subqs() const;

	MergePostList(const std::vector<PostList *> & plists_,
		      MultiMatch *matcher_,
		      ValueStreamDocument & vsdoc_)
	    : plists(plists_), current(-1), matcher(matcher_), vsdoc(vsdoc_)
	{ }

	~MergePostList();
};

#endif /* OM_HGUARD_MERGEPOSTLIST_H */
