/** @file glass_inverter.h
 * @brief Inverter class which "inverts the file".
 */
/* Copyright (C) 2009,2010,2013,2014 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_GLASS_INVERTER_H
#define XAPIAN_INCLUDED_GLASS_INVERTER_H

#include "xapian/types.h"

#include <map>
#include <string>
#include <vector>

#include "omassert.h"
#include "str.h"
#include "xapian/error.h"

class GlassPostListTable;
class GlassPositionListTable;

namespace Xapian {
class TermIterator;
}

/** Magic wdf value used for a deleted posting. */
const Xapian::termcount DELETED_POSTING = Xapian::termcount(-1);

/** Class which "inverts the file". */
class Inverter {
    friend class GlassPostListTable;

    /// Class for storing the changes in frequencies for a term.
    class PostingChanges {
	friend class GlassPostListTable;

	/// Change in term frequency,
	Xapian::termcount_diff tf_delta;

	/// Change in collection frequency.
	Xapian::termcount_diff cf_delta;

	/// Changes to this term's postlist.
	std::map<Xapian::docid, Xapian::termcount> pl_changes;

      public:
	/// Constructor for an added posting.
	PostingChanges(Xapian::docid did, Xapian::termcount wdf)
	    : tf_delta(1), cf_delta(Xapian::termcount_diff(wdf))
	{
	    pl_changes.insert(std::make_pair(did, wdf));
	}

	/// Constructor for a removed posting.
	PostingChanges(Xapian::docid did, Xapian::termcount wdf, bool)
	    : tf_delta(-1), cf_delta(-Xapian::termcount_diff(wdf))
	{
	    pl_changes.insert(std::make_pair(did, DELETED_POSTING));
	}

	/// Constructor for an updated posting.
	PostingChanges(Xapian::docid did, Xapian::termcount old_wdf,
		       Xapian::termcount new_wdf)
	    : tf_delta(0), cf_delta(Xapian::termcount_diff(new_wdf - old_wdf))
	{
	    pl_changes.insert(std::make_pair(did, new_wdf));
	}

	/// Add a posting.
	void add_posting(Xapian::docid did, Xapian::termcount wdf) {
	    ++tf_delta;
	    cf_delta += wdf;
	    // Add did to term's postlist
	    pl_changes[did] = wdf;
	}

	/// Remove a posting.
	void remove_posting(Xapian::docid did, Xapian::termcount wdf) {
	    --tf_delta;
	    cf_delta -= wdf;
	    // Remove did from term's postlist.
	    pl_changes[did] = DELETED_POSTING;
	}

	/// Update a posting.
	void update_posting(Xapian::docid did, Xapian::termcount old_wdf,
			    Xapian::termcount new_wdf) {
	    cf_delta += new_wdf - old_wdf;
	    pl_changes[did] = new_wdf;
	}

	/// Get the term frequency delta.
	Xapian::termcount_diff get_tfdelta() const { return tf_delta; }

	/// Get the collection frequency delta.
	Xapian::termcount_diff get_cfdelta() const { return cf_delta; }
    };

    /// Buffered changes to postlists.
    std::map<std::string, PostingChanges> postlist_changes;

    /// Buffered changes to positional data.
    std::map<std::string, std::map<Xapian::docid, std::string> > pos_changes;

    void store_positions(const GlassPositionListTable & position_table,
			 Xapian::docid did,
			 const std::string & tname,
			 const std::vector<Xapian::termpos> & posvec,
			 bool modifying);

    void set_positionlist(Xapian::docid did,
			  const std::string & term,
			  const std::string & s);

  public:
    /// Buffered changes to document lengths.
    std::map<Xapian::docid, Xapian::termcount> doclen_changes;

  public:
    void add_posting(Xapian::docid did, const std::string & term,
		     Xapian::doccount wdf) {
	std::map<std::string, PostingChanges>::iterator i;
	i = postlist_changes.find(term);
	if (i == postlist_changes.end()) {
	    postlist_changes.insert(
		std::make_pair(term, PostingChanges(did, wdf)));
	} else {
	    i->second.add_posting(did, wdf);
	}
    }

    void remove_posting(Xapian::docid did, const std::string & term,
			Xapian::doccount wdf) {
	std::map<std::string, PostingChanges>::iterator i;
	i = postlist_changes.find(term);
	if (i == postlist_changes.end()) {
	    postlist_changes.insert(
		std::make_pair(term, PostingChanges(did, wdf, false)));
	} else {
	    i->second.remove_posting(did, wdf);
	}
    }

    void update_posting(Xapian::docid did, const std::string & term,
			Xapian::termcount old_wdf,
			Xapian::termcount new_wdf) {
	std::map<std::string, PostingChanges>::iterator i;
	i = postlist_changes.find(term);
	if (i == postlist_changes.end()) {
	    postlist_changes.insert(
		std::make_pair(term, PostingChanges(did, old_wdf, new_wdf)));
	} else {
	    i->second.update_posting(did, old_wdf, new_wdf);
	}
    }

    void set_positionlist(const GlassPositionListTable & position_table,
			  Xapian::docid did,
			  const std::string & tname,
			  const Xapian::TermIterator & term,
			  bool modifying = false);

    void delete_positionlist(Xapian::docid did,
			     const std::string & term);

    bool get_positionlist(Xapian::docid did,
			  const std::string & term,
			  std::string & s) const;

    bool has_positions(const GlassPositionListTable & position_table) const;

    void clear() {
	doclen_changes.clear();
	postlist_changes.clear();
	pos_changes.clear();
    }

    void set_doclength(Xapian::docid did, Xapian::termcount doclen, bool add) {
	if (add) {
	    Assert(doclen_changes.find(did) == doclen_changes.end() || doclen_changes[did] == DELETED_POSTING);
	}
	doclen_changes[did] = doclen;
    }

    void delete_doclength(Xapian::docid did) {
	Assert(doclen_changes.find(did) == doclen_changes.end() || doclen_changes[did] != DELETED_POSTING);
	doclen_changes[did] = DELETED_POSTING;
    }

    bool get_doclength(Xapian::docid did, Xapian::termcount & doclen) const {
	std::map<Xapian::docid, Xapian::termcount>::const_iterator i;
	i = doclen_changes.find(did);
	if (i == doclen_changes.end())
	    return false;
	if (rare(i->second == DELETED_POSTING))
	    throw Xapian::DocNotFoundError("Document not found: " + str(did));
	doclen = i->second;
	return true;
    }

    /// Flush document length changes.
    void flush_doclengths(GlassPostListTable & table);

    /// Flush postlist changes for @a term.
    void flush_post_list(GlassPostListTable & table, const std::string & term);

    /// Flush postlist changes for all terms.
    void flush_all_post_lists(GlassPostListTable & table);

    /// Flush postlist changes for all terms which start with @a pfx.
    void flush_post_lists(GlassPostListTable & table, const std::string & pfx);

    /// Flush all postlist table changes.
    void flush(GlassPostListTable & table);

    /// Flush position changes.
    void flush_pos_lists(GlassPositionListTable & table);

    bool get_deltas(const std::string & term,
		    Xapian::termcount_diff & tf_delta,
		    Xapian::termcount_diff & cf_delta) const {
	std::map<std::string, PostingChanges>::const_iterator i;
	i = postlist_changes.find(term);
	if (i == postlist_changes.end()) {
	    return false;
	}
	tf_delta = i->second.get_tfdelta();
	cf_delta = i->second.get_cfdelta();
	return true;
    }
};

#endif // XAPIAN_INCLUDED_GLASS_INVERTER_H
