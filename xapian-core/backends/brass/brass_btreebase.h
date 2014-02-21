/** @file brass_btreebase.h
 * @brief Btree base file implementation
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2004,2007,2008,2009,2011,2012,2013,2014 Olly Betts
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

#ifndef OM_HGUARD_BRASS_BTREEBASE_H
#define OM_HGUARD_BRASS_BTREEBASE_H

#include <string>

#include "brass_types.h"
#include "brass_freelist.h"
#include "xapian/constants.h"

class BrassChanges;

class BrassTable_base : public BrassFreeList {
    public:
	/** Construct an object with all zero fields. */
	BrassTable_base();

	/** Read values from a base file.
	 *
	 *  @param name		The base of the filename
	 *  @param ch		The suffix
	 *  @param err_msg	An error string which will be appended
	 *  			to for some errors instead of throwing
	 *  			an exception.
	 *
	 *  @return	true if the read succeeded, or false otherwise.
	 */
	bool read(const std::string &name, char ch, std::string &err_msg);

	uint4 get_root() const { return root; }
	uint4 get_level() const { return level; }
	brass_tablesize_t get_item_count() const { return item_count; }
	bool get_have_fakeroot() const { return have_fakeroot; }
	bool get_sequential() const { return sequential; }

	void set_block_size(int flags, uint4 block_size_) {
	    no_sync = (flags & Xapian::DB_NO_SYNC);
	    BrassFreeList::set_block_size(block_size_);
	}
	void set_root(uint4 root_) {
	    root = root_;
	}
	void set_level(uint4 level_) {
	    level = level_;
	}
	void set_item_count(brass_tablesize_t item_count_) {
	    item_count = item_count_;
	}
	void set_have_fakeroot(bool have_fakeroot_) {
	    have_fakeroot = have_fakeroot_;
	}
	void set_sequential(bool sequential_) {
	    sequential = sequential_;
	}

	/** Write the btree base file to disk. */
	void write_to_file(const std::string &filename,
			   char base_letter,
			   const char * tablename,
			   BrassChanges * changes);

	void swap(BrassTable_base &other);

    private:
	/** No copying. */
	BrassTable_base(const BrassTable_base &);

	/** private assignment operator - you probably want swap() instead */
	void operator=(const BrassTable_base &other);

	/* Decoded values from the base file follow */
	uint4 root;
	uint4 level;
	brass_tablesize_t item_count;
	bool have_fakeroot;
	bool sequential;

	/** Suppress calls to io_sync(). */
	bool no_sync;
};

#endif /* OM_HGUARD_BRASS_BTREEBASE_H */
