/** @file
 * @brief Edit distance calculation algorithm.
 */
/* Copyright (C) 2003 Richard Boulton
 * Copyright (C) 2007,2008,2017,2019 Olly Betts
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

#include <cstdlib>
#include <climits>
#include <vector>

#include "omassert.h"
#include "xapian/unicode.h"

/** Calculate edit distances to a target string.
 *
 *  Edit distance is defined as the minimum number of edit operations
 *  required to move from one sequence to another.  The edit operations
 *  considered are:
 *   - Insertion of a character at an arbitrary position.
 *   - Deletion of a character at an arbitrary position.
 *   - Substitution of a character at an arbitrary position.
 *   - Transposition of two neighbouring characters at an arbitrary position
 *     in the string.
 */
class EditDistanceCalculator {
    /// Don't allow assignment.
    EditDistanceCalculator& operator=(const EditDistanceCalculator&) = delete;

    /// Don't allow copying.
    EditDistanceCalculator(const EditDistanceCalculator&) = delete;

    /// Target in UTF-32.
    std::vector<unsigned> target;

    size_t target_bytes;

    /// Current candidate in UTF-32.
    mutable std::vector<unsigned> utf32;

    mutable int* array = nullptr;

    /** The type to use for the occurrence bitmaps.
     *
     *  There will be a trade-off between how good the bound is and how many
     *  bits we use.  We currently use an unsigned long long, which is
     *  typically 64 bits and seems to work pretty well (it makes sense it
     *  does for English, as each letter maps to a different bit).
     *
     *  At least on x86-64 and English, a 32-bit type seems to give an
     *  identical cycle count to a 64-bit type when profiled with cachegrind,
     *  but a 64-bit type is likely to work better for languages which have
     *  more than 32 commonly-used word characters.
     *
     *  FIXME: Profile other architectures and languages.
     */
    typedef unsigned long long freqs_bitmap;

    /** Occurrence bitmap for target sequence.
     *
     *  We set the bit corresponding to each codepoint in the word and then
     *  xor the bitmaps for the target and candidate and count the bits to
     *  compute a lower bound on the edit distance.  Rather than flagging
     *  each Unicode code point uniquely, we reduce the code points modulo
     *  the number of available bits which can only reduce the bound we
     *  calculate.
     */
    freqs_bitmap target_freqs = 0;

    static constexpr unsigned FREQS_MASK = sizeof(freqs_bitmap) * 8 - 1;

    /** Calculate edit distance.
     *
     *  Internal helper - the cheap case is inlined from the header.
     */
    int calc(const unsigned* ptr, int len, int max_distance) const;

  public:
    /** Constructor.
     *
     *  @param target_	Target string to calculate edit distances to.
     */
    explicit
    EditDistanceCalculator(const std::string& target_)
	: target_bytes(target_.size()) {
	using Xapian::Utf8Iterator;
	for (Utf8Iterator it(target_); it != Utf8Iterator(); ++it) {
	    unsigned ch = *it;
	    target.push_back(ch);
	    target_freqs |= freqs_bitmap(1) << (ch & FREQS_MASK);
	}
    }

    ~EditDistanceCalculator() {
	delete [] array;
    }

    /** Calculate edit distance for a sequence.
     *
     *  @param candidate	String to calculate edit distance for.
     *  @param max_distance	The greatest edit distance that's interesting
     *				to us.  If the true edit distance is >
     *				max_distance, any value > max_distance may be
     *				returned instead (which allows the edit
     *				distance algorithm to avoid work for poor
     *				matches).  The value passed for subsequent
     *				calls to this method on the same object must be
     *				the same or less.
     *
     *  @return The edit distance between candidate and the target.
     */
    int operator()(const std::string& candidate, int max_distance) const {
	// We have the UTF-32 target in target.
	size_t target_utf32_len = target.size();

	// We can quickly rule out some candidates just by comparing
	// lengths since each edit can change the number of UTF-32 characters
	// by at most 1.  But first we check the encoded UTF-8 length of the
	// candidate since we can do that without having to convert it to
	// UTF-32.

	// Each Unicode codepoint is 1-4 bytes in UTF-8 and one word in UTF-32,
	// so the number of UTF-32 characters in candidate must be <= the number
	// of bytes of UTF-8.
	if (target_utf32_len > candidate.size() + max_distance) {
	    // Candidate too short.
	    return INT_MAX;
	}

	// Each edit can change the number of UTF-8 bytes by up to 4 (addition
	// or deletion of any character which needs 4 bytes in UTF-8), which
	// gives us an alternative lower bound (which is sometimes tighter and
	// sometimes not) and the tightest upper bound.
	if (target_bytes > candidate.size() + 4 * max_distance) {
	    // Candidate too short.
	    return INT_MAX;
	}
	if (target_bytes + 4 * max_distance < candidate.size()) {
	    // Candidate too long.
	    return INT_MAX;
	}

	// Now convert to UTF-32.
	utf32.assign(Xapian::Utf8Iterator(candidate), Xapian::Utf8Iterator());

	// Check a cheap length-based lower bound based on UTF-32 lengths.
	int lb = std::abs(int(utf32.size()) - int(target_utf32_len));
	if (lb > max_distance) {
	    return lb;
	}

	// Actually calculate the edit distance.
	return calc(&utf32[0], utf32.size(), max_distance);
    }
};

#endif // XAPIAN_INCLUDED_EDITDISTANCE_H
