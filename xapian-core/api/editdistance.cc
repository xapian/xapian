/** @file
 * @brief Edit distance calculation algorithm.
 *
 *  Based on that described in:
 *
 *  "An extension of Ukkonen's enhanced dynamic programming ASM algorithm"
 *  by Hal Berghel, University of Arkansas
 *  and David Roach, Acxiom Corporation
 *
 *  http://berghel.net/publications/asm/asm.php
 */
/* Copyright (C) 2003 Richard Boulton
 * Copyright (C) 2007,2008,2009,2017,2019,2020 Olly Betts
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

#include <config.h>

#include "editdistance.h"

#include "omassert.h"
#include "popcount.h"

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <cstring>

using namespace std;

template<class Char>
struct edist_seq {
    edist_seq(const Char* ptr_, int len_) : ptr(ptr_), len(len_) { }
    const Char* ptr;
    int len;
};

template<class Char>
class edist_state {
    /// Don't allow assignment.
    edist_state& operator=(const edist_state&) = delete;

    /// Don't allow copying.
    edist_state(const edist_state&) = delete;

    edist_seq<Char> seq1;
    edist_seq<Char> seq2;

    /* Array of f(k,p) values, where f(k,p) = the largest index i such that
     * d(i,j) = p and d(i,j) is on diagonal k.
     * ie: f(k,p) = largest i s.t. d(i,k+i) = p
     * Where: d(i,j) = edit distance between substrings of length i and j.
     */
    int* fkp;
    int fkp_rows;

    /* Maximum possible edit distance (this is referred to as ZERO_K in
     * the algorithm description by Berghel and Roach). */
    int maxdist;

    int calc_index(int k, int p) const {
	return k + maxdist + fkp_rows * (p + 1);
    }

  public:
    edist_state(const Char* ptr1, int len1, const Char* ptr2, int len2,
		int* fkp_)
	: seq1(ptr1, len1), seq2(ptr2, len2), fkp(fkp_), maxdist(len2) {
	Assert(len2 >= len1);
	// fkp is stored as a rectangular array, column by column.  Each entry
	// represents a value of p, from -1 to maxdist or a special value
	// close-ish to INT_MIN.
	fkp_rows = 2 * maxdist + 1;
	// It's significantly faster to memset() than std::fill_n() with an int
	// value, so fill with the msb of INT_MIN, which for 32-bit 2's
	// complement int means -2139062144 instead of -2147483648, which is
	// fine what we need here.
	memset(fkp, unsigned(INT_MIN) >> (8 * (sizeof(int) - 1)),
	       sizeof(int) * (calc_index(maxdist, maxdist - 2) + 1));
	set_f_kp(0, -1, -1);
	for (int k = 1; k <= maxdist; ++k) {
	    set_f_kp(k, k - 1, -1);
	    set_f_kp(-k, k - 1, k - 1);
	}
    }

    int get_f_kp(int k, int p) const {
	return fkp[calc_index(k, p)];
    }

    void set_f_kp(int k, int p, int val) {
	fkp[calc_index(k, p)] = val;
    }

    bool is_transposed(int pos1, int pos2) const {
	if (pos1 <= 0 || pos2 <= 0 || pos1 >= seq1.len || pos2 >= seq2.len)
	    return false;
	return (seq1.ptr[pos1 - 1] == seq2.ptr[pos2] &&
		seq1.ptr[pos1] == seq2.ptr[pos2 - 1]);
    }

    void edist_calc_f_kp(int k, int p);
};

template<class Char>
void edist_state<Char>::edist_calc_f_kp(int k, int p)
{
    int maxlen = get_f_kp(k, p - 1) + 1; /* dist if do substitute */
    int maxlen2 = get_f_kp(k - 1, p - 1); /* dist if do insert */
    int maxlen3 = get_f_kp(k + 1, p - 1) + 1; /* dist if delete */

    if (is_transposed(maxlen, maxlen + k)) {
	// Transposition.
	++maxlen;
    }

    if (maxlen >= maxlen2) {
	if (maxlen >= maxlen3) {
	    // Transposition or Substitution.
	} else {
	    // Deletion.
	    maxlen = maxlen3;
	}
    } else {
	if (maxlen2 >= maxlen3) {
	    // Insertion.
	    maxlen = maxlen2;
	} else {
	    // Deletion.
	    maxlen = maxlen3;
	}
    }

    /* Check for exact matches, and increase the length until we don't have
     * one. */
    while (maxlen < seq1.len &&
	   maxlen + k < seq2.len &&
	   seq1.ptr[maxlen] == seq2.ptr[maxlen + k]) {
	++maxlen;
    }
    set_f_kp(k, p, maxlen);
}

template<class Char>
static int
seqcmp_editdist(const Char* ptr1, int len1, const Char* ptr2, int len2,
		int* fkp_, int max_distance)
{
    int lendiff = len2 - len1;
    /* Make sure second sequence is longer (or same length). */
    if (lendiff < 0) {
	lendiff = -lendiff;
	swap(ptr1, ptr2);
	swap(len1, len2);
    }

    /* Special case for if one or both sequences are empty. */
    if (len1 == 0) return len2;

    edist_state<Char> state(ptr1, len1, ptr2, len2, fkp_);

    int p = lendiff; /* This is the minimum possible edit distance. */
    while (p <= max_distance) {
	for (int temp_p = 0; temp_p != p; ++temp_p) {
	    int inc = p - temp_p;
	    if (abs(lendiff - inc) <= temp_p) {
		state.edist_calc_f_kp(lendiff - inc, temp_p);
	    }
	    if (abs(lendiff + inc) <= temp_p) {
		state.edist_calc_f_kp(lendiff + inc, temp_p);
	    }
	}
	state.edist_calc_f_kp(lendiff, p);

	if (state.get_f_kp(lendiff, p) == len1) break;
	++p;
    }

    return p;
}

int
EditDistanceCalculator::calc(const unsigned* ptr, int len,
			     int max_distance) const
{
    // Calculate a cheap lower bound on the edit distance by considering
    // frequency histograms.
    freqs_bitmap freqs = 0;
    freqs_bitmap freqs2 = 0;
    for (int i = 0; i != len; ++i) {
	unsigned ch = ptr[i];
	auto bit = freqs_bitmap(1) << (ch & FREQS_MASK);
	freqs2 |= (freqs & bit);
	freqs |= bit;
    }
    // Each insertion or deletion adds at most 1 to total.  Each transposition
    // doesn't change it at all.  But each substitution can change it by 2 so
    // we need to divide it by 2.  We round up since the unpaired change must
    // be due to an actual edit.
    unsigned bits = 1;
    add_popcount(bits, (freqs ^ target_freqs) | (freqs2 ^ target_freqs2));
    int ed_lower_bound = bits / 2;
    if (ed_lower_bound > max_distance) {
	// It's OK to return any distance > max_distance if the true answer is
	// > max_distance.
	return ed_lower_bound;
    }

    if (!array) {
	// Allocate space for the largest case we need to consider, which is
	// when the second sequence is len + max_distance long.  Any second
	// sequence which is longer must be more than max_distance edits
	// away.
	int maxdist = target.size() + max_distance;
	int max_cols = maxdist * 2;
	int max_rows = maxdist * 2 + 1;
	array = new int[max_rows * max_cols];
    }

    return seqcmp_editdist<unsigned>(ptr, len, &target[0], target.size(),
				     array, max_distance);
}
