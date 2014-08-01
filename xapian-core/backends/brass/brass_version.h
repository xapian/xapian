/** @file brass_version.h
 * @brief BrassVersion class
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2013,2014 Olly Betts
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

#include "brass_changes.h"
#include "brass_defs.h"
#include "omassert.h"

#include <cstring>
#include <string>

#include "common/safeuuid.h"

namespace Brass {

class RootInfo {
    brass_block_t root;
    unsigned level;
    brass_tablesize_t num_entries;
    bool root_is_fake;
    bool sequential_mode;
    unsigned blocksize;
    std::string fl_serialised;

  public:
    void init(unsigned blocksize_);

    void serialise(std::string &s) const;

    bool unserialise(const char ** p, const char * end);

    brass_block_t get_root() const { return root; }
    int get_level() const { return int(level); }
    brass_tablesize_t get_num_entries() const { return num_entries; }
    bool get_root_is_fake() const { return root_is_fake; }
    bool get_sequential_mode() const { return sequential_mode; }
    unsigned get_blocksize() const {
	AssertRel(blocksize,>=,2048);
	return blocksize;
    }
    const std::string & get_free_list() const { return fl_serialised; }

    void set_level(int level_) { level = unsigned(level_); }
    void set_num_entries(brass_tablesize_t n) { num_entries = n; }
    void set_root_is_fake(bool f) { root_is_fake = f; }
    void set_sequential_mode(bool f) { sequential_mode = f; }
    void set_root(brass_block_t root_) { root = root_; }
    void set_blocksize(unsigned b) {
	AssertRel(b,>=,2048);
	blocksize = b;
    }
    void set_free_list(const std::string & s) { fl_serialised = s; }
};

}

using Brass::RootInfo;

/** The BrassVersion class manages the revision files.
 *
 *  The "iambrass" file (currently) contains a "magic" string identifying
 *  that this is a brass database, a database format version number, the UUID
 *  of the database, the revision of the database, and the root block info for
 *  each table.
 */
class BrassVersion {
    brass_revision_number_t rev;

    RootInfo root[Brass::MAX_];
    RootInfo old_root[Brass::MAX_];

    /** The UUID of this database.
     *
     *  This is mutable for older uuid libraries which take non-const uuid_t.
     */
    mutable uuid_t uuid;

    int fd;

    /// The database directory.
    std::string db_dir;

    BrassChanges * changes;

  public:
    BrassVersion(const std::string & db_dir_)
	: rev(0), db_dir(db_dir_), changes(NULL) { }

    /** Create the version file. */
    void create(unsigned blocksize, int flags);

    void set_changes(BrassChanges * changes_) { changes = changes_; }

    /** Read the version file and check it's a version we understand.
     *
     *  On failure, an exception is thrown.
     */
    void read();

    void cancel();

    const std::string write(brass_revision_number_t new_rev);

    bool sync(const std::string & tmpfile,
	      brass_revision_number_t new_rev, int flags);

    brass_revision_number_t get_revision() const { return rev; }

    // Only used by compaction code to handle reading an old brass DB.
    void set_revision(brass_revision_number_t rev_) { rev = rev_; }

    const RootInfo & get_root(Brass::table_type tbl) const {
	return root[tbl];
    }

    RootInfo * root_to_set(Brass::table_type tbl) {
	return &root[tbl];
    }

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

#if 0 // Unused currently.
    /// Set the UUID from 16 byte binary value @a data.
    void set_uuid(const void * data) {
	std::memcpy(uuid, data, 16);
    }

    /** Set the UUID from the standard 36 character string format.
     *
     *  @return true if @a s was successfully parsed; false otherwise.
     */
    bool set_uuid_string(const std::string & s) {
	return uuid_parse(s.c_str(), uuid);
    }
#endif
};

#endif // XAPIAN_INCLUDED_BRASS_VERSION_H
