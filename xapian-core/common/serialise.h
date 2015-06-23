/* @file serialise.h
 * @brief functions to convert classes to strings and back
 *
 * Copyright (C) 2006,2007,2008,2009,2015 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SERIALISE_H
#define XAPIAN_INCLUDED_SERIALISE_H

#include <string>
#include "noreturn.h"
#include "xapian/visibility.h"
#include "xapian/weight.h"

// Forward class declarations:

namespace Xapian {
    class Document;
    class Error;
    class MSet;
    class RSet;
}

/** Serialise a Xapian::Error object to a string.
 *
 *  @param e	The Xapian::Error object to serialise.
 *
 *  @return	Serialisation of @a e.
 */
XAPIAN_VISIBILITY_DEFAULT
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
XAPIAN_VISIBILITY_DEFAULT
XAPIAN_NORETURN(
void unserialise_error(const std::string &error_string,
		       const std::string &prefix,
		       const std::string &new_context));

/** Serialise a stats object.
 *
 *  @param stats	The stats object to serialise.
 *
 *  @return	Serialisation of @a stats.
 */
std::string serialise_stats(const Xapian::Weight::Internal &stats);

/** Unserialise a serialised stats object.
 *
 *  @param s	The string to unserialise.
 *
 *  @return	The unserialised stats object.
 */
Xapian::Weight::Internal unserialise_stats(const std::string &s);

/** Serialise a Xapian::MSet object.
 *
 *  @param mset		The object to serialise.
 *
 *  @return		The serialisation of the Xapian::MSet object.
 */
std::string serialise_mset(const Xapian::MSet &mset);

/** Serialise a Xapian::MSet object with sort keys.
 *
 *  @param mset		The object to serialise.
 *
 *  @return		The serialisation of the Xapian::MSet object.
 */
std::string serialise_mset_new(const Xapian::MSet &mset);

/** Unserialise a serialised Xapian::MSet object.
 *
 *  @param p	 Pointer to the start of the string to unserialise.
 *  @param p_end Pointer to the end of the string to unserialise.
 *
 *  @return	The unserialised Xapian::MSet object.
 */
Xapian::MSet unserialise_mset(const char * p, const char * p_end);

/** Unserialise a serialised Xapian::MSet object with sort keys.
 *
 *  @param p	 Pointer to the start of the string to unserialise.
 *  @param p_end Pointer to the end of the string to unserialise.
 *
 *  @return	The unserialised Xapian::MSet object.
 */
Xapian::MSet unserialise_mset_new(const char * p, const char * p_end);

/** Serialise a Xapian::RSet object.
 *
 *  @param rset		The object to serialise.
 *
 *  @return		The serialisation of the Xapian::RSet object.
 */
std::string serialise_rset(const Xapian::RSet &omrset);

/** Unserialise a serialised Xapian::RSet object.
 *
 *  @param s		The serialised object as a string.
 *
 *  @return		The unserialised Xapian::RSet object.
 */
Xapian::RSet unserialise_rset(const std::string &s);

/** Serialise a Xapian::Document object.
 *
 *  @param doc		The object to serialise.
 *
 *  @return		The serialisation of the Xapian::Document object.
 */
XAPIAN_VISIBILITY_DEFAULT
std::string serialise_document(const Xapian::Document &doc);

/** Unserialise a serialised Xapian::Document object.
 *
 *  @param s		The serialised object as a string.
 *
 *  @return		The unserialised Xapian::Document object.
 */
XAPIAN_VISIBILITY_DEFAULT
Xapian::Document unserialise_document(const std::string &s);

#endif
