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

#include <algorithm>
#include <string>
#include <vector>

using namespace std;

void
omegascript_sort(const vector<string>& args,
		 string& value)
{
    const string &list = args[0];
    if (list.empty()) return;

    bool uniq = false;
    bool rev = false;
    if (args.size() > 1) {
	for (auto opt_ch : args[1]) {
	    switch (opt_ch) {
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
}
