/** @file
 * @brief functions to convert classes to strings and back
 */
/* Copyright (C) 2006,2007,2008,2009,2012,2014 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SERIALISE_ERROR_H
#define XAPIAN_INCLUDED_SERIALISE_ERROR_H

#include <string>
#include "noreturn.h"

// Forward class declarations:

namespace Xapian {
    class Error;
}

/** Serialise a Xapian::Error object to a string.
 *
 *  @param e	The Xapian::Error object to serialise.
 *
 *  @return	Serialisation of @a e.
 */
std::string serialise_error(const Xapian::Error &e);

/** Unserialise a Xapian::Error object and throw it.
 *
 *  Note: does not return!
 *
 *  @param error_string		The string to unserialise.
 *  @param prefix		Optional prefix to prepend to the unserialised
 *				Error's @a msg field.
 *  @param new_context		Optional context to replace the context in
 *				the error.  If this is specified, any existing
 *				context will be noted in the Error's @a msg
 *				field.
 */
XAPIAN_NORETURN(
void unserialise_error(const std::string &error_string,
		       const std::string &prefix,
		       const std::string &new_context));

#endif
