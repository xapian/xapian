/** @file
 * @brief A position list in a glass database.
 */
/* Copyright (C) 2005,2006,2008,2009,2010,2011,2013,2016,2017,2019 Olly Betts
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

#ifndef XAPIAN_INCLUDED_GLASS_POSITIONLIST_H
#define XAPIAN_INCLUDED_GLASS_POSITIONLIST_H

#include <xapian/types.h>

#include "bitstream.h"
#include "glass_cursor.h"
#include "glass_lazytable.h"
#include "pack.h"
#include "backends/positionlist.h"

#include <string>

class GlassPositionListTable : public GlassLazyTable {
  public:
    static std::string make_key(Xapian::docid did, const std::string& term) {
	std::string key;
	pack_string_preserving_sort(key, term);
	pack_uint_preserving_sort(key, did);
	return key;
    }

    /** Create a new GlassPositionListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the glass database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    GlassPositionListTable(const std::string& dbdir, bool readonly)
	: GlassLazyTable("position", dbdir + "/position.", readonly) { }

    GlassPositionListTable(int fd, off_t offset_, bool readonly_)
	: GlassLazyTable("position", fd, offset_, readonly_) { }

    /** Pack a position list into a string.
     *
     *  @param s The string to append the position list data to.
     */
    void pack(std::string& s, const Xapian::VecCOW<Xapian::termpos>& vec) const;

    /** Set the position list for term tname in document did.
     */
    void set_positionlist(Xapian::docid did, const std::string& tname,
			  const std::string& s) {
	add(make_key(did, tname), s);
    }

    /// Delete the position list for term tname in document did.
    void delete_positionlist(Xapian::docid did, const std::string& tname) {
	del(make_key(did, tname));
    }

    /// Return the number of entries in specified position list data.
    Xapian::termcount positionlist_count(const std::string& data) const;

    /// Return the number of entries in specified position list.
    Xapian::termcount positionlist_count(Xapian::docid did,
					 const std::string& term) const;
};

/** Base-class for a position list in a glass database. */
class GlassBasePositionList : public PositionList {
    /// Copying is not allowed.
    GlassBasePositionList(const GlassBasePositionList&) = delete;

    /// Assignment is not allowed.
    GlassBasePositionList& operator=(const GlassBasePositionList&) = delete;

  protected:
    /// Interpolative decoder.
    BitReader rd;

    /// Current entry.
    Xapian::termpos current_pos;

    /// Last entry.
    Xapian::termpos last;

    /// Number of entries.
    Xapian::termcount size;

    /// Have we started iterating yet?
    bool have_started;

    /** Set positional data and start to decode it.
     *
     *  @param data	The positional data.  Must stay valid
     *			while this object is using it.
     */
    void set_data(const std::string& data);

  public:
    /// Default constructor.
    GlassBasePositionList() {}

    /// Returns size of position list.
    Xapian::termcount get_approx_size() const;

    Xapian::termpos back() const;

    /** Returns current position.
     *
     *  Either next() or skip_to() must have been called before this
     *  method can be called.
     */
    Xapian::termpos get_position() const;

    /// Advance to the next term position in the list.
    bool next();

    /// Advance to the first term position which is at least termpos.
    bool skip_to(Xapian::termpos termpos);
};

/** A position list in a glass database. */
class GlassPositionList : public GlassBasePositionList {
    /// The encoded positional data being read by rd.
    std::string pos_data;

    /// Copying is not allowed.
    GlassPositionList(const GlassPositionList&) = delete;

    /// Assignment is not allowed.
    GlassPositionList& operator=(const GlassPositionList&) = delete;

  public:
    /// Construct and initialise with data.
    explicit
    GlassPositionList(std::string&& data);

    /// Construct and initialise with data.
    GlassPositionList(const GlassTable* table,
		      Xapian::docid did,
		      const std::string& term);
};

/** A reusable position list in a glass database. */
class GlassRePositionList : public GlassBasePositionList {
    /// Cursor for locating multiple entries efficiently.
    GlassCursor cursor;

    /// Copying is not allowed.
    GlassRePositionList(const GlassRePositionList&) = delete;

    /// Assignment is not allowed.
    GlassRePositionList& operator=(const GlassRePositionList&) = delete;

  public:
    /// Constructor.
    explicit
    GlassRePositionList(const GlassTable* table)
	: cursor(table) {}

    /** Fill list with data, and move the position to the start. */
    void assign_data(std::string&& data);

    /** Fill list with data, and move the position to the start. */
    void read_data(Xapian::docid did,
		   const std::string& term);
};

#endif /* XAPIAN_INCLUDED_GLASS_POSITIONLIST_H */
