/** @file
 * @brief OmegaScript $sort function
 */
/* Copyright (C) 2001,2004,2012,2016,2018 Olly Betts
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

static int
natcmp(const string& a, const string& b)
{
    size_t shorter = min(a.size(), b.size());
    size_t i = 0;
    while (i != shorter) {
	int cha = static_cast<unsigned char>(a[i]);
	int chb = static_cast<unsigned char>(b[i]);
	if (!C_isdigit(cha)) {
	    if (cha == chb) {
		// Matching non-digits.
		++i;
		continue;
	    }

	    if (C_isdigit(chb)) return 1;
	    return cha - chb;
	}

	// Sort embedded digit spans by numeric value and before non-digits.
	if (!C_isdigit(chb)) return -1;

	// Skip any leading zeros on each.
	size_t sa = i;
	while (a[sa] == '0') ++sa;
	size_t sb = i;
	while (b[sb] == '0') ++sb;

	size_t ea = sa;
	size_t eb = sb;
	int res = 0;
	while (true) {
	    if (!C_isdigit(a[ea])) {
		if (C_isdigit(b[eb])) {
		    // Number in b is longer and so larger.
		    return -1;
		}
		// Digit sequences are the same length.
		break;
	    }
	    if (a[ea] != b[eb]) {
		if (!C_isdigit(b[eb])) {
		    // Number in a is longer and so larger.
		    return 1;
		}
		// Record first difference between digits.
		if (res == 0) res = a[ea] - b[eb];
	    }
	    ++ea;
	    ++eb;
	}

	if (res == 0) {
	    // More leading zeros sorts first.
	    res = int(sb) - int(sa);
	}
	if (res) return res;

	// Digit sequences were identical, so keep going.
	i = ea;
    }
    return int(a.size()) - int(b.size());
}

void
omegascript_sort(const vector<string>& args,
		 string& value)
{
    const string &list = args[0];
    if (list.empty()) return;

    // 0 => string sort, '#' => natural, 'n' => numeric.
    char mode = 0;
    bool uniq = false;
    bool rev = false;
    if (args.size() > 1) {
	for (auto opt_ch : args[1]) {
	    switch (opt_ch) {
		case '#':
		case 'n':
		    if (mode != 0) {
			string m = "Invalid $sort option combination: ";
			m += args[1];
			throw m;
		    }
		    mode = opt_ch;
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

    if (mode != 'n') {
	if (mode == 0) {
	    // String sort.
	    if (!rev) {
		sort(items.begin(), items.end());
	    } else {
		sort(items.begin(), items.end(),
		     [](const string& a, const string& b) {
			 return a > b;
		     });
	    }
	} else {
	    // "Natural" sort - embedded natural numbers are handled specially.
	    if (!rev) {
		sort(items.begin(), items.end(),
		     [](const string& a, const string& b) {
			 return natcmp(a, b) < 0;
		     });
	    } else {
		sort(items.begin(), items.end(),
		     [](const string& a, const string& b) {
			 return natcmp(a, b) > 0;
		     });
	    }
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
	size_t i = 0;
	while (i < part_f) {
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
