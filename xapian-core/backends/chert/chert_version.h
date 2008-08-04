/** @file chert_version.h
 * @brief ChertVersion class
 */
/* Copyright (C) 2006,2007,2008 Olly Betts
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

#ifndef OM_HGUARD_CHERT_VERSION_H
#define OM_HGUARD_CHERT_VERSION_H

#include <string>

/** The ChertVersion class manages the "iamchert" file.
 *
 *  The "iamchert" file (currently) contains a "magic" string identifying
 *  that this is a chert database and a database format version number.
 */
class ChertVersion {
    std::string filename;

  public:
    ChertVersion(const std::string & dbdir) : filename(dbdir) {
	filename += "/iamchert";
    }

    /** Create the version file. */
    void create();

    /** Read the version file and check it's a version we understand.
     *
     *  On failure, an exception is thrown.
     */
    void read_and_check();
};

#endif
