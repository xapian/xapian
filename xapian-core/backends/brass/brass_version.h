/** @file brass_version.h
 * @brief BrassVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010 Olly Betts
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

#ifndef XAPIAN_INCLUDED_BRASS_VERSION_H
#define XAPIAN_INCLUDED_BRASS_VERSION_H

#include "brass_defs.h"

#include <cstring>
#include <string>

#include "common/safeuuid.h"

#include "xapian/visibility.h"

/** The BrassVersion class manages the revision files.
 *
 *  Each "v<8 hex digits>" file (currently) contains a "magic" string
 *  identifying that this is a brass database and a database format version
 *  number, the uuid of the database, and the root block number for each table.
 */
class XAPIAN_VISIBILITY_DEFAULT BrassVersion {
    brass_revision_number_t rev;

    brass_block_t root[Brass::MAX_];

    brass_block_t new_root[Brass::MAX_];

    unsigned blocksize;

    /** The UUID of this database.
     *
     *  This is mutable for older uuid libraries which take non-const uuid_t.
     */
    mutable uuid_t uuid;

    int fd;

  public:
    BrassVersion() : rev(0), blocksize(0) { }

    /** Create the version file. */
    void create(unsigned blocksize_, const std::string & db_dir);

    void open_most_recent(const std::string & db_dir);

    /** Read the version file and check it's a version we understand.
     *
     *  On failure, an exception is thrown.
     */
    void read(const std::string & filename);

    const std::string write(const std::string & db_dir);

    void sync(const std::string & db_dir, const std::string & tmpfile,
	      brass_revision_number_t new_rev);

    brass_revision_number_t get_revision() const { return rev; }

    brass_block_t get_root_block(Brass::table_type tbl) const {
	return root[tbl];
    }

    unsigned get_block_size() const { return blocksize; }

    void set_root_block(Brass::table_type tbl, brass_block_t root_)  {
	new_root[tbl] = root_;
    }

    void set_block_size(unsigned blocksize_) { blocksize = blocksize_; }

    /// Return pointer to 16 byte UUID.
    const char * get_uuid() const {
	// uuid is unsigned char[].
	return reinterpret_cast<const char *>(uuid);
    }

    /// Return UUID in the standard 36 character string format.
    std::string get_uuid_string() const {
	char buf[37];
	uuid_unparse_lower(uuid, buf);
	return std::string(buf, 36);
    }

    /// Set the UUID from 16 byte binary value @a data.
    void set_uuid(void * data) {
	std::memcpy(uuid, data, 16);
    }

    /** Set the UUID from the standard 36 character string format.
     *
     *  @return true if @a s was successfully parsed; false otherwise.
     */
    bool set_uuid_string(const std::string & s) {
	return uuid_parse(s.c_str(), uuid);
    }
};

#endif // XAPIAN_INCLUDED_BRASS_VERSION_H
