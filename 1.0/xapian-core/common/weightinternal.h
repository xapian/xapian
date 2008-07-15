/** @file weightinternal.h
 * @brief Internals of weight object.
 */
/* Copyright 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_WEIGHTINTERNAL_H
#define XAPIAN_INCLUDED_WEIGHTINTERNAL_H

#include "xapian/enquire.h" // For Xapian::Weight::Internal
#include <string>

// Forward declaration.
class Stats;

/** Class to hold statistics for a given collection. */
class Xapian::Weight::Internal {
    public:
	/** Number of documents in the collection. */
	Xapian::doccount collection_size;

	/** Number of relevant documents in the collection. */
	Xapian::doccount rset_size;

	/** Average length of documents in the collection. */
	Xapian::doclength average_length;

	/** Term frequency.
	 *
	 *  ie, number of documents that the term for this weight object
	 *  occurs in.
	 */
	Xapian::doccount termfreq;

	/** Relevant term frequency.
	 *
	 *  ie, number of relevant documents that the term for this weight
	 *  object occurs in.
	 */
	Xapian::doccount reltermfreq;

	/** Create a Weight::Internal object with global statistics.
	 *
	 *  All term-specific statistics will be set to 0.
	 *
	 *  @param stats Object containing the statistics to use.
	 */
	Internal(const Stats & stats);

	/** Create a Weight::Internal object with global and term statistics.
	 *
	 *  @param stats Object containing the statistics to use.
	 *  @param tname The term to read the term-specific statistics for.
	 */
	Internal(const Stats & stats, const std::string & tname);
};

#endif // XAPIAN_INCLUDED_WEIGHTINTERNAL_H
