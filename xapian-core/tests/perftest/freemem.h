/* freemem.h: determine how much free physical memory there is.
 *
 * Copyright (C) 2007 Olly Betts
 * Copyright (C) 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_FREEMEM_H
#define XAPIAN_INCLUDED_FREEMEM_H

/** Determine how much free physical memory there is.
 *
 *  Returns the amount of free physical memory, in bytes, or -1 if this isn't
 *  known.
 */
long get_free_physical_memory();

/** Determine how much physical memory there is.
 *
 *  Returns the amount of physical memory, in bytes, or -1 if this isn't known.
 */
long get_total_physical_memory();

#endif // XAPIAN_INCLUDED_FREEMEM_H
