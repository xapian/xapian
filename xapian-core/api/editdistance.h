/** @file
 * @brief Edit distance calculation algorithm.
 */
/* Copyright (C) 2003 Richard Boulton
 * Copyright (C) 2007,2008 Olly Betts
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

#ifndef XAPIAN_INCLUDED_EDITDISTANCE_H
#define XAPIAN_INCLUDED_EDITDISTANCE_H

/** Calculate the edit distance between two sequences.
 *
 *  Edit distance is defined as the minimum number of edit operations
 *  required to move from one sequence to another.  The edit operations
 *  considered are:
 *   - Insertion of a character at an arbitrary position.
 *   - Deletion of a character at an arbitrary position.
 *   - Substitution of a character at an arbitrary position.
 *   - Transposition of two neighbouring characters at an arbitrary position
 *     in the string.
 *
 *  @param ptr1 A pointer to the start of the first sequence.
 *  @param len1 The length of the first sequence.
 *  @param ptr2 A pointer to the start of the second sequence.
 *  @param len2 The length of the first sequence.
 *  @param max_distance The greatest edit distance that's interesting to us.
 *			If the true edit distance is > max_distance, any
 *			value > max_distance may be returned instead (which
 *			allows the edit distance algorithm to avoid work for
 *			poor matches).
 *
 *  @return The edit distance from one item to the other.
 */
int edit_distance_unsigned(const unsigned* ptr1, int len1,
			   const unsigned* ptr2, int len2,
			   int max_distance);

#endif // XAPIAN_INCLUDED_EDITDISTANCE_H
