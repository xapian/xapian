/* btree_base.h: Btree base file implementation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef OM_HGUARD_BTREE_BASE_H
#define OM_HGUARD_BTREE_BASE_H

#include "btree_types.h"
#include "btree_util.h"

class Btree_base {
    public:
	/** Initialise a Btree_Base object with all zero fields.
	 */
	Btree_base();

	/** Read a base file from disk into a structure in memory.
	 *
	 *  @param name			The base filename name
	 *  @param ch			The suffix
	 */
	Btree_base(const std::string &name_, char ch);

	/** Copy constructor */
	Btree_base(const Btree_base &other);
	/** Assignment operator */
	void operator=(const Btree_base &other);

	/** Destructor - frees resources. */
	~Btree_base();

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
	bool read(const std::string &name, char ch,
		  std::string &err_msg);

	uint4 get_revision();
	uint4 get_block_size();
	int4 get_root();
	int4 get_level();
	int4 get_bit_map_size();
	int4 get_item_count();
	int4 get_last_block();
	bool get_have_fakeroot();

	void set_revision(uint4 revision_);
	void set_block_size(uint4 block_size_);
	void set_root(int4 root_);
	void set_level(int4 level_);
	void set_bit_map_size(int4 bit_map_size_);
	void set_item_count(int4 item_count_);
	void set_last_block(int4 last_block_);
	void set_have_fakeroot(bool have_fakeroot_);

	/** Write the btree base file to disk. */
	void write_to_file(const std::string &filename);
    private:

	/** The actual data from the base file. */
	byte *data;

	/* Decoded values from the base file follow */
	uint4 revision;
	uint4 block_size;
	int4 root;
	int4 level;
	int4 bit_map_size;
	int4 item_count;
	int4 last_block;
	bool have_fakeroot;
};

#endif /* OM_HGUARD_BTREE_BASE_H */

