/* omerr_string.h: utilities to convert OmError exceptions to strings
 *                 and vice versa.
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

#ifndef OM_HGUARD_OMERR_STRING_H
#define OM_HGUARD_OMERR_STRING_H

#include <string>
#include <stdexcept>

#include "om/omerror.h"

/** Convert an OmError reference into a string which describes it.
 *
 *  @param e		A reference to the exception to convert.
 */
std::string omerror_to_string(const OmError &e);

/** Take a string representation of an exception (from omerror_to_string)
 *  and effectively rethrow the exception, optionally with a prefix added
 *  to the message.
 *
 *  @param	except		The string describing the exception to be
 *                              thrown.
 *  @param	prefix		A string to be prepended to the message.
 */
void string_to_omerror(const std::string &except, const std::string &prefix = "");

#endif /* OM_HGUARD_OMERR_STRING_H */
