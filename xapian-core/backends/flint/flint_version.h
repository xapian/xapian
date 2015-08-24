/** @file flint_version.h
 * @brief FlintVersion class
 */
/* Copyright (C) 2006,2007,2008,2009 Olly Betts
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

#ifndef OM_HGUARD_FLINT_VERSION_H
#define OM_HGUARD_FLINT_VERSION_H

#include <cstring>
#include <string>

#include "common/safeuuid.h"

/** The FlintVersion class manages the "iamflint" file.
 *
 *  The "iamflint" file (currently) contains a "magic" string identifying
 *  that this is a flint database and a database format version number.
 */
class FlintVersion {
    /// The filename of the version file.
    std::string filename;

    /// The UUID of this database.
    mutable uuid_t uuid;

    /// Generate a UUID if we don't already have one.
    void ensure_uuid() const;

  public:
    FlintVersion(const std::string & dbdir)
	: filename(dbdir + "/iamflint") { }

    /** Create the version file. */
    void create();

    /** Read the version file and check it's a version we understand.
     *
     *  @param readonly    true if the database is being opened readonly.
     * 
     *  On failure, an exception is thrown.
     */
    void read_and_check(bool readonly);

    /// Return pointer to 16 byte UUID.
    const char * get_uuid() const {
	ensure_uuid();
	return reinterpret_cast<const char *>(uuid);
    }

    /// Return UUID in the standard 36 character string format.
    std::string get_uuid_string() const {
	char buf[37];
	ensure_uuid();
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

#endif
