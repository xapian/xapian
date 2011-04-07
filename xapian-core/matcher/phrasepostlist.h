/* phrasepostlist.h: Return only items where terms are near or form a phrase
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2003,2004,2005 Olly Betts
 * Copyright 2009 Lemur Consulting Ltd
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

#ifndef OM_HGUARD_PHRASEPOSTLIST_H
#define OM_HGUARD_PHRASEPOSTLIST_H

#include "selectpostlist.h"
#include <vector>

typedef Xapian::PositionIterator::Internal PositionList;

/** a postlist comprising several postlists NEARed together.
 *
 *  This postlist returns a posting if and only if it is in all of the
 *  sub-postlists and all the terms occur within a specified distance of
 *  each other somewhere in the document.  The weight for a posting is the
 *  sum of the weights of the sub-postings.
 */
class NearPostList : public SelectPostList {
    private:
        Xapian::termpos window;
	std::vector<PostList *> terms;

    	bool test_doc();
        bool do_test(std::vector<PositionList *> &plists, Xapian::termcount i,
		     Xapian::termcount min, Xapian::termcount max);
    public:
	std::string get_description() const;
	Xapian::termcount get_wdf() const;

	Xapian::doccount get_termfreq_est() const
	{
	    // No idea how to estimate this - FIXME
	    return source->get_termfreq_est() / 2;
	}

	TermFreqs get_termfreq_est_using_stats(
            const Xapian::Weight::Internal & stats) const;

        NearPostList(PostList *source_, Xapian::termpos window_,
		     std::vector<PostList *>::const_iterator &terms_begin_,
		     std::vector<PostList *>::const_iterator &terms_end_)
	    : SelectPostList(source_), terms(terms_begin_, terms_end_)
        {
	    window = window_;
	}
};

/** A postlist comprising several postlists PHRASEd together.
 *
 *  This postlist returns a posting if and only if it is in all of the
 *  sub-postlists and all the terms occur IN THE GIVEN ORDER within a
 *  specified distance of each other somewhere in the document.  The weight
 *  for a posting is the sum of the weights of the sub-postings.
 */
class PhrasePostList : public SelectPostList {
    private:
        Xapian::termpos window;
	std::vector<PostList *> terms;

    	bool test_doc();
        bool do_test(std::vector<PositionList *> &plists, Xapian::termcount i,
		     Xapian::termcount min, Xapian::termcount max);
    public:
	std::string get_description() const;
	Xapian::termcount get_wdf() const;

	Xapian::doccount get_termfreq_est() const
	{
	    // No idea how to estimate this - FIXME
	    return source->get_termfreq_est() / 3;
	}

	TermFreqs get_termfreq_est_using_stats(
            const Xapian::Weight::Internal & stats) const;

        PhrasePostList(PostList *source_, Xapian::termpos window_,
		       std::vector<PostList *>::const_iterator &terms_begin_,
		       std::vector<PostList *>::const_iterator &terms_end_)
	    : SelectPostList(source_), terms(terms_begin_, terms_end_)
        {
	    window = window_;
	}
};

#endif /* OM_HGUARD_PHRASEPOSTLIST_H */
