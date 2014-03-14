/* flint_positionlist.h: A position list in a flint database.
 *
 * Copyright (C) 2005,2006,2008,2010 Olly Betts
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

#ifndef XAPIAN_HGUARD_FLINT_POSITIONLIST_H
#define XAPIAN_HGUARD_FLINT_POSITIONLIST_H

#include <xapian/types.h>

#include "flint_table.h"
#include "flint_utils.h"
#include "positionlist.h"

#include <string>
#include <vector>

using namespace std;

class FlintPositionListTable : public FlintTable {
    static string make_key(Xapian::docid did, const string & tname) {
	return F_pack_uint_preserving_sort(did) + tname;
    }

  public:
    /** Create a new FlintPositionListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the flint database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    FlintPositionListTable(const string & dbdir, bool readonly)
	: FlintTable("position", dbdir + "/position.", readonly, DONT_COMPRESS, true) { }

    /** Set the position list for term tname in document did.
     *
     *  @param check_for_update If true, check if the new list is the same as
     *				the existing list (if there is one).
     */
    void set_positionlist(Xapian::docid did, const string & tname,
			  Xapian::PositionIterator pos,
			  const Xapian::PositionIterator &pos_end,
			  bool check_for_update);

    /// Delete the position list for term tname in document did.
    void delete_positionlist(Xapian::docid did, const string & tname) {
	del(make_key(did, tname));
    }

    /// Return the number of entries in specified position list.
    Xapian::termcount positionlist_count(Xapian::docid did,
					 const string & term) const;
};

/** A position list in a flint database. */
class FlintPositionList : public PositionList {
    /// Vector of term positions.
    vector<Xapian::termpos> positions;

    /// Position of iteration through data.
    vector<Xapian::termpos>::const_iterator current_pos;

    /// Have we started iterating yet?
    bool have_started;

    /// Copying is not allowed.
    FlintPositionList(const FlintPositionList &);

    /// Assignment is not allowed.
    void operator=(const FlintPositionList &);

  public:
    /// Default constructor.
    FlintPositionList() : have_started(false) {}

    /// Construct and initialise with data.
    FlintPositionList(const FlintTable * table, Xapian::docid did,
		      const string & tname) {
	(void)read_data(table, did, tname);
    }

    /** Fill list with data, and move the position to the start.
     *
     *  @return true if position data was read.
     */
    bool read_data(const FlintTable * table, Xapian::docid did,
		   const string & tname);

    /// Returns size of position list.
    Xapian::termcount get_size() const;

    /** Returns current position.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::termpos get_position() const;

    /// Advance to the next term position in the list.
    void next();

    /// Advance to the first term position which is at least termpos.
    void skip_to(Xapian::termpos termpos);

    /// True if we're off the end of the list
    bool at_end() const;
};

#endif /* XAPIAN_HGUARD_FLINT_POSITIONLIST_H */
