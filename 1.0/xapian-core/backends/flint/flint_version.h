/** @file flint_version.h
 * @brief FlintVersion class
 */
/* Copyright (C) 2006,2007 Olly Betts
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

#include <string>

/** The FlintVersion class manages the "iamflint" file.
 *
 *  The "iamflint" file (currently) contains a "magic" string identifying
 *  that this is a flint database and a database format version number.
 */
class FlintVersion {
    std::string filename;

  public:
    FlintVersion(const std::string & dbdir) : filename(dbdir) {
	filename += "/iamflint";
    }

    /** Create the version file. */
    void create();

    /** Read the version file and check it's a version we understand.
     *
     *  @param readonly    true if the database is being opened readonly.
     * 
     *  On failure, an exception is thrown.
     */
    void read_and_check(bool readonly);
};

#endif
