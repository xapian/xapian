/** @file
 * @brief Class for merging ValueList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2017 Olly Betts
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

#ifndef XAPIAN_INCLUDED_MULTI_VALUELIST_H
#define XAPIAN_INCLUDED_MULTI_VALUELIST_H

#include "backends/valuelist.h"

#include "backends/databaseinternal.h"
#include "backends/multi.h"

#include <string>

struct SubValueList {
    ValueList * valuelist;
    unsigned shard;

    SubValueList(ValueList * vl, unsigned shard_)
	: valuelist(vl), shard(shard_) { }

    ~SubValueList() {
	delete valuelist;
    }

    void skip_to(Xapian::docid did, Xapian::doccount n_shards) {
	// Calculate the docid in this shard which is the same or later than
	// did (which may be in a different shard).
	Xapian::docid shard_did = shard_docid(did, n_shards);
	if (shard_number(did, n_shards) > shard)
	    ++shard_did;
	valuelist->skip_to(shard_did);
    }

    Xapian::docid get_docid() const {
	return valuelist->get_docid();
    }

    Xapian::docid get_merged_docid(unsigned n_shards) const {
	return unshard(valuelist->get_docid(), shard, n_shards);
    }

    std::string get_value() const { return valuelist->get_value(); }

    void next() {
	valuelist->next();
    }

    bool at_end() const { return valuelist->at_end(); }
};

/// Class for merging ValueList objects from subdatabases.
class MultiValueList : public ValueList {
    /// Don't allow assignment.
    void operator=(const MultiValueList &);

    /// Don't allow copying.
    MultiValueList(const MultiValueList &);

    /// Current docid (or 0 if we haven't started yet).
    Xapian::docid current_docid = 0;

    /// Number of SubValueList* entries in valuelists.
    Xapian::doccount count;

    /// Array of sub-valuelists which we use as a heap.
    SubValueList** valuelists;

    /// The value slot we're iterating over.
    Xapian::valueno slot;

    Xapian::doccount n_shards;

  public:
    /// Constructor.
    MultiValueList(Xapian::doccount n_shards_,
		   SubValueList** valuelists_,
		   Xapian::valueno slot_)
	: count(n_shards_),
	  valuelists(valuelists_),
	  slot(slot_),
	  n_shards(n_shards_)
    {
    }

    /// Destructor.
    ~MultiValueList();

    /// Return the docid at the current position.
    Xapian::docid get_docid() const;

    /// Return the value at the current position.
    std::string get_value() const;

    /// Return the value slot for the current position/this iterator.
    Xapian::valueno get_valueno() const;

    /// Return true if the current position is past the last entry in this list.
    bool at_end() const;

    /** Advance the current position to the next document in the value stream.
     *
     *  The list starts before the first entry in the list, so next(),
     *  skip_to() or check() must be called before any methods which need the
     *  context of the current position.
     */
    void next();

    /** Skip forward to the specified docid.
     *
     *  If the specified docid isn't in the list, position ourselves on the
     *  first document after it (or at_end() if no greater docids are present).
     */
    void skip_to(Xapian::docid);

    /** Check if the specified docid occurs in this valuestream.
     *
     *  The caller is required to ensure that the specified @a docid actually
     *  exists in the database.
     *
     *  This method acts like skip_to() if that can be done at little extra
     *  cost, in which case it then sets @a valid to true.
     *
     *  Otherwise it simply checks if a particular docid is present.  If it
     *  is, it returns true.  If it isn't, it returns false, and leaves the
     *  position unspecified (and hence the result of calling methods which
     *  depend on the current position, such as get_docid(), are also
     *  unspecified).  In this state, next() will advance to the first matching
     *  position after @a docid, and skip_to() will act as it would if the
     *  position was the first matching position after @a docid.
     *
     *  The default implementation calls skip_to().
     */
    bool check(Xapian::docid did);

    /// Return a string description of this object.
    std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_MULTIVALUELIST_H
