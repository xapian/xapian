/* utils.h: Various useful utilities
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

#ifndef OM_HGUARD_UTILS_H
#define OM_HGUARD_UTILS_H

#include <string>
#include <vector>
#include <map>

/// Convert an integer to a string
std::string om_tostring(int a);

/// Convert an unsigned integer to a string
std::string om_tostring(unsigned int a);

/// Convert a long integer to a string
std::string om_tostring(long int a);

/// Convert an unsigned long integer to a string
std::string om_tostring(unsigned long int a);

/// Convert a double to a string
std::string om_tostring(double a);

/// Convert a bool to a string
std::string om_tostring(bool a);

///////////////////////////////////////////
// Mapping of types as strings to enums  //
///////////////////////////////////////////

struct StringAndValue {
    const char * name;
    int value;
};

/** Get the value associated with the given string.  If the string
 *  isn't found, the value returned is the value in the terminating
 *  object (which has a zero length string).
 *
 *  Note: this just uses a list of entrys, and searches linearly
 *  through them.  Could at make this do a binary chop, but probably
 *  not worth doing so, unless list gets large.
 */
int map_string_to_value(const StringAndValue * haystack,
			const std::string needle);

/** Return true if the file fname exists.
 */
bool file_exists(const std::string &fname);

/** Return true if all the files fnames exist.
 */
bool files_exist(const std::vector<std::string> &fnames);

#endif /* OM_HGUARD_UTILS_H */
