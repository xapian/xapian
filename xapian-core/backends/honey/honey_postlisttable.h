/** @file
 * @brief Subclass of HoneyTable which holds postlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014,2015,2016,2018 Olly Betts
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

#ifndef XAPIAN_INCLUDED_HONEY_POSTLISTTABLE_H
#define XAPIAN_INCLUDED_HONEY_POSTLISTTABLE_H

#include <xapian/constants.h>
#include <xapian/types.h>

#include "honey_inverter.h"
#include "honey_postlist.h"
#include "honey_table.h"
#include "pack.h"

#include <string>

class HoneyDatabase;
class PostingChanges;

class HoneyPostListTable : public HoneyTable {
  public:
    /** Create a new HoneyPostListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the honey database is stored in.
     *  @param readonly	    true if we're opening read-only, else false.
     */
    HoneyPostListTable(const std::string& dbdir, bool readonly)
	: HoneyTable("postlist", dbdir + "/postlist.", readonly) { }

    HoneyPostListTable(int fd, off_t offset_, bool readonly)
	: HoneyTable("postlist", fd, offset_, readonly) { }

    bool term_exists(const std::string& term) const {
	return key_exists(pack_honey_postlist_key(term));
    }

    HoneyPostList* open_post_list(const HoneyDatabase* db,
				  const std::string& term,
				  bool need_read_pos) const;

    void get_freqs(const std::string& term,
		   Xapian::doccount* termfreq_ptr,
		   Xapian::termcount* collfreq_ptr) const;

    void get_used_docid_range(Xapian::doccount doccount,
			      Xapian::docid& first,
			      Xapian::docid& last) const;

    Xapian::termcount get_wdf_upper_bound(const std::string& term) const;

    std::string get_metadata(const std::string& key) const {
	std::string value;
	(void)get_exact_entry(std::string("\0", 2) + key, value);
	return value;
    }

    void merge_doclen_changes(const std::map<Xapian::docid,
			      Xapian::termcount>& changes) {
	// Honey doesn't support update currently.
	(void)changes;
    }

    void merge_changes(const std::string& term,
		       const HoneyInverter::PostingChanges& changes) {
	// Honey doesn't support update currently.
	(void)term;
	(void)changes;
    }
};

#endif // XAPIAN_INCLUDED_HONEY_POSTLISTTABLE_H
