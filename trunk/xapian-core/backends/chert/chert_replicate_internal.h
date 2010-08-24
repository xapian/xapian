/** @file chert_replicate_internal.h
 * @brief Internal definitions for chert database replication
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#ifndef XAPIAN_INCLUDED_CHERT_REPLICATE_INTERNAL_H
#define XAPIAN_INCLUDED_CHERT_REPLICATE_INTERNAL_H

// Magic string used to recognise a changeset file.
#define CHANGES_MAGIC_STRING "ChertChanges"

// The current version of changeset files.
// 1  - initial implementation
#define CHANGES_VERSION 1u

// Must be big enough to ensure that the start of the changeset (up to the new
// revision number) will fit in this much space.
#define REASONABLE_CHANGESET_SIZE 1024

#endif /* XAPIAN_INCLUDED_CHERT_REPLICATE_INTERNAL_H */
