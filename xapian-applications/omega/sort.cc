/* @file sort.cc
 * @brief OmegaScript $sort function
 */
/* Copyright (C) 2018 Olly Betts
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

#include <config.h>

#include "sort.h"

// For option["decimal"].
#include "omega.h"
#include "stringutils.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

// Return -1, 0 or 1 depending on sign of floating point number starting at
// index i of string s.  Non-numbers give 0.
static int
string_sign(const string& s, const string& decimal, size_t i = 0)
{
    int r = 1;
    if (s[i] == '-') {
	r = -1;
	++i;
    }
    while (s[i] == '0') {
	++i;
    }
    if (s.compare(i, decimal.size(), decimal) == 0) {
	i += decimal.size();
	while (s[i] == '0') {
	    ++i;
	}
    }
    return C_isdigit(s[i]) ? r : 0;
}

// Compare strings of two non-zero floating point numbers, without loss of
// precision, and ignoring their signs.
//
// The format handled is (as a Perl regex):
//
// ^[ \t]*(-?[0-9]*($decimal[0-9]*)?)
//
// where $decimal is option["decimal"].
//
// Values which are equal by the numeric test are compared as strings.
static int
ncmp(const string& a, const string& b, const string& decimal)
{
    // Ignore leading blanks, an optional minus sign, and leading zeros.
    size_t ai = a.find_first_not_of(" \t");
    if (a[ai] == '-') ++ai;
    while (a[ai] == '0') ++ai;

    size_t bi = b.find_first_not_of(" \t");
    if (b[bi] == '-') ++bi;
    while (b[bi] == '0') ++bi;

    while (a[ai] == b[bi] && C_isdigit(a[ai])) {
	// Matching digits.
	++ai;
	++bi;
    }

    if (C_isdigit(a[ai])) {
	if (!C_isdigit(b[bi])) {
	    // a > b
	    return 1;
	}

	int r = a[ai] - b[bi];
	bool a_digit, b_digit;
	do {
	    ++ai;
	    ++bi;
	    a_digit = C_isdigit(a[ai]);
	    b_digit = C_isdigit(b[bi]);
	} while (a_digit && b_digit);

	if (!a_digit && !b_digit) {
	    // Same number of digits, but not the same digits.
	    return r;
	}
	return a_digit ? 1 : -1;
    }

    if (C_isdigit(b[bi])) {
	// a < b
	return -1;
    }

    // Non-fractional parts are the same - compare any fractional parts.
    bool a_frac = (a.compare(ai, decimal.size(), decimal) == 0);
    if (a_frac) ai += decimal.size();
    bool b_frac = (b.compare(bi, decimal.size(), decimal) == 0);
    if (b_frac) bi += decimal.size();
    if (!a_frac && !b_frac) return 0;
    if (!b_frac) {
	// Check if a's fractional part is zero.
	while (a[ai] == '0') ++ai;
	return C_isdigit(a[ai]) ? 1 : 0;
    }
    if (!a_frac) {
	// Check if b's fractional part is zero.
	while (b[bi] == '0') ++bi;
	return C_isdigit(b[bi]) ? -1 : 0;
    }

    // Both have fractional parts, so compare.
    while (a[ai] == b[bi] && C_isdigit(a[ai])) {
	++ai;
	++bi;
    }
    if (C_isdigit(a[ai])) return 1;
    if (C_isdigit(b[bi])) return -1;
    return 0;
}

void
omegascript_sort(const vector<string>& args,
		 string& value)
{
    const string &list = args[0];
    if (list.empty()) return;

    bool num = false;
    bool uniq = false;
    bool rev = false;
    if (args.size() > 1) {
	for (auto opt_ch : args[1]) {
	    switch (opt_ch) {
		case 'n':
		    num = true;
		    break;
		case 'r':
		    rev = true;
		    break;
		case 'u':
		    uniq = true;
		    break;
		default:
		    throw string("Unknown $sort option: ") + opt_ch;
	    }
	}
    }
    vector<string> items;
    string::size_type split = 0, split2;
    do {
	split2 = list.find('\t', split);
	items.emplace_back(list, split, split2 - split);
	split = split2 + 1;
    } while (split2 != string::npos);

    if (!num) {
	// String sort.
	if (!rev) {
	    sort(items.begin(), items.end());
	} else {
	    sort(items.begin(), items.end(),
		 [](const string& a, const string& b) {
		     return a > b;
		 });
	}

	value.reserve(list.size());
	bool tab = false;
	const string* prev = nullptr;
	for (auto&& item : items) {
	    // Skip duplicates if "u" flag specified.
	    if (prev && *prev == item) {
		continue;
	    }
	    if (uniq) {
		prev = &item;
	    }

	    if (tab) {
		value += '\t';
	    } else {
		tab = true;
	    }
	    value += item;
	}
	return;
    }

    // Numeric sort.
    //
    // We first partition items such that all the negative values come first,
    // then all the zero values, then all the positive values (for a forward
    // search that is - for a reverse search the order of the partitions is
    // reversed).
    //
    // Then we sort within each partition.
    const string& decimal = option["decimal"];
    size_t part_z = 0;
    size_t part_f = items.size();

    int first = rev ? 1 : -1;
    if (!uniq) {
	// Scan linearly through items, swapping to create the desired
	// partitioning.
	for (size_t i = 0; i < part_f; ) {
	    int sign = string_sign(items[i], decimal);
	    if (sign == first) {
		// Swap value to end of the first partition.
		if (part_z != i)
		    swap(items[part_z], items[i]);
		++part_z;
		++i;
	    } else if (sign == 0) {
		// Extend middle partition to include zero value.
		++i;
	    } else {
		// Swap value to start of final partition.
		swap(items[i], items[--part_f]);
	    }
	}
    } else {
	// Need to preserve order within each partition.
	auto z = stable_partition(items.begin(), items.end(),
				  [&decimal, first](const string& a) {
				      return string_sign(a, decimal) == first;
				  });
	part_z = z - items.begin();
	auto f = stable_partition(z, items.end(),
				  [&decimal](const string& a) {
				      return string_sign(a, decimal) == 0;
				  });
	part_f = f - items.begin();
    }

    value.reserve(list.size());
    bool tab = false;

    const string* prev = nullptr;
    if (part_z > 0) {
	// Sort the first partition.
	if (!uniq) {
	    if (!rev) {
		sort(items.begin(), items.begin() + part_z,
		     [&decimal](const string& a, const string& b) {
			 int r = ncmp(a, b, decimal);
			 return r ? r > 0 : a < b;
		     });
	    } else {
		sort(items.begin(), items.begin() + part_z,
		     [&decimal](const string& a, const string& b) {
			 int r = ncmp(a, b, decimal);
			 return r ? r > 0 : a > b;
		     });
	    }
	} else {
	    stable_sort(items.begin(), items.begin() + part_z,
		 [&decimal](const string& a, const string& b) {
		     return ncmp(a, b, decimal) > 0;
		 });

	    for (size_t i = 0; i != part_z; ++i) {
		const string& item = items[i];
		// Skip duplicates.
		if (prev && ncmp(*prev, item, decimal) == 0) {
		    continue;
		}
		prev = &item;

		if (tab) {
		    value += '\t';
		} else {
		    tab = true;
		}
		value += item;
	    }
	}
    }

    if (part_z != part_f) {
	// Handle all the zero values.
	if (!uniq) {
	    if (!rev) {
		sort(items.begin() + part_z, items.begin() + part_f,
		     [](const string& a, const string& b) {
			 return a < b;
		     });
	    } else {
		sort(items.begin() + part_z, items.begin() + part_f,
		     [](const string& a, const string& b) {
			 return a > b;
		     });
	    }
	} else {
	    if (tab) {
		value += '\t';
	    } else {
		tab = true;
	    }
	    // No need to actually sort this partition when uniq is true -
	    // we just take the first element.
	    value += items[part_z];
	}
    }

    if (part_f != items.size()) {
	// Sort the final partition.
	if (!uniq) {
	    if (!rev) {
		sort(items.begin() + part_f, items.end(),
		     [&decimal](const string& a, const string& b) {
			 int r = ncmp(a, b, decimal);
			 return r ? r < 0 : a < b;
		     });
	    } else {
		sort(items.begin() + part_f, items.end(),
		     [&decimal](const string& a, const string& b) {
			 int r = ncmp(a, b, decimal);
			 return r ? r < 0 : a > b;
		     });
	    }
	} else {
	    stable_sort(items.begin() + part_f, items.end(),
		 [&decimal](const string& a, const string& b) {
		     return ncmp(a, b, decimal) < 0;
		 });

	    for (size_t i = part_f; i != items.size(); ++i) {
		const string& item = items[i];
		// Skip duplicates.
		if (prev && ncmp(*prev, item, decimal) == 0) {
		    continue;
		}
		prev = &item;

		if (tab) {
		    value += '\t';
		} else {
		    tab = true;
		}
		value += item;
	    }
	}
    }

    // If uniq is true we already assembled the output in value as we processed
    // each partition.
    if (!uniq) {
	for (auto&& item : items) {
	    if (tab) {
		value += '\t';
	    } else {
		tab = true;
	    }
	    value += item;
	}
    }
}
