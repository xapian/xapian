/** @file honey_postlisttable.h
 * @brief Subclass of HoneyTable which holds postlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2013,2014,2015,2016 Olly Betts
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
#include "honey_table.h"
#include "pack.h"

#include <string>

class PostingChanges;

class HoneyPostListTable : public HoneyTable {
  public:
    static std::string make_key(const std::string& term) {
	return pack_honey_postlist_key(term);
    }

    static std::string make_key(const std::string& term, Xapian::docid did) {
	return pack_honey_postlist_key(term, did);
    }

    /** Create a new HoneyPostListTable object.
     *
     *  This method does not create or open the table on disk - you
     *  must call the create() or open() methods respectively!
     *
     *  @param dbdir	    The directory the honey database is stored in.
     *  @param readonly	    true if we're opening read-only, else false.
     */
    HoneyPostListTable(const std::string & dbdir, bool readonly)
	: HoneyTable("postlist", dbdir + "/postlist.", readonly) { }

    HoneyPostListTable(int fd, off_t offset_, bool readonly)
	: HoneyTable("postlist", fd, offset_, readonly) { }

    bool term_exists(const std::string& term) const {
	return key_exists(make_key(term));
    }

    void merge_doclen_changes(const std::map<Xapian::docid, Xapian::termcount>& changes) {
	(void)changes;
    }

    void merge_changes(const std::string& term,
		       const HoneyInverter::PostingChanges& changes) {
	(void)term;
	(void)changes;
    }
};

#endif // XAPIAN_INCLUDED_HONEY_POSTLISTTABLE_H
