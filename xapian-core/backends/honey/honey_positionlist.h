/** @file
 * @brief A position list in a honey database.
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

#ifndef XAPIAN_INCLUDED_HONEY_POSITIONLIST_H
#define XAPIAN_INCLUDED_HONEY_POSITIONLIST_H

#include <xapian/types.h>

#include "backends/positionlist.h"
#include "bitstream.h"
#include "honey_cursor.h"
#include "honey_lazytable.h"
#include "pack.h"

#include <string>

class HoneyPositionTable : public HoneyLazyTable {
  public:
    static std::string make_key(Xapian::docid did, const std::string& term) {
	std::string key;
	pack_string_preserving_sort(key, term);
	pack_uint_preserving_sort(key, did);
	return key;
    }

    /** Create a new HoneyPositionTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir		The directory the honey database is stored in.
     *  @param readonly		true if we're opening read-only, else false.
     */
    HoneyPositionTable(const std::string& dbdir, bool readonly)
	: HoneyLazyTable("position", dbdir + "/position.", readonly) { }

    HoneyPositionTable(int fd, off_t offset_, bool readonly_)
	: HoneyLazyTable("position", fd, offset_, readonly_) { }

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

    /// Return the number of entries in specified position list.
    Xapian::termcount positionlist_count(Xapian::docid did,
					 const std::string& term) const;
};

/** Base-class for a position list in a honey database. */
class HoneyBasePositionList : public PositionList {
    /// Copying is not allowed.
    HoneyBasePositionList(const HoneyBasePositionList&) = delete;

    /// Assignment is not allowed.
    HoneyBasePositionList& operator=(const HoneyBasePositionList&) = delete;

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
    HoneyBasePositionList() {}

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

/** A position list in a honey database. */
class HoneyPositionList : public HoneyBasePositionList {
    /// The encoded positional data being read by rd.
    std::string pos_data;

    /// Copying is not allowed.
    HoneyPositionList(const HoneyPositionList&) = delete;

    /// Assignment is not allowed.
    HoneyPositionList& operator=(const HoneyPositionList&) = delete;

  public:
    /// Construct and initialise with data.
    explicit
    HoneyPositionList(std::string&& data);

    /// Construct and initialise with data.
    HoneyPositionList(const HoneyTable& table,
		      Xapian::docid did,
		      const std::string& term);
};

/** A reusable position list in a honey database. */
class HoneyRePositionList : public HoneyBasePositionList {
    /// Cursor for locating multiple entries efficiently.
    HoneyCursor cursor;

    /// Copying is not allowed.
    HoneyRePositionList(const HoneyRePositionList&) = delete;

    /// Assignment is not allowed.
    HoneyRePositionList& operator=(const HoneyRePositionList&) = delete;

  public:
    /// Constructor.
    explicit
    HoneyRePositionList(const HoneyTable& table)
	: cursor(&table) {}

    /** Fill list with data, and move the position to the start. */
    void assign_data(std::string&& data);

    /** Fill list with data, and move the position to the start. */
    void read_data(Xapian::docid did, const std::string& term);
};

#endif /* XAPIAN_INCLUDED_HONEY_POSITIONLIST_H */
