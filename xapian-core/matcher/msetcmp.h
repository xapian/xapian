/** @file
 * @brief MSetItem comparison functions.
 */
/* Copyright (C) 2006,2007,2011,2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_MSETCMP_H
#define XAPIAN_INCLUDED_MSETCMP_H

#include "api/omenquireinternal.h"

// typedef for MSetItem comparison function.
typedef bool (* MSetCmp)(const Xapian::Internal::MSetItem &,
			 const Xapian::Internal::MSetItem &);

/// Select the appropriate msetcmp function.
MSetCmp get_msetcmp_function(Xapian::Enquire::Internal::sort_setting sort_by, bool sort_forward, bool sort_value_forward);

#endif // XAPIAN_INCLUDED_MSETCMP_H
