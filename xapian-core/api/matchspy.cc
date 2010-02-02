/** @file matchspy.cc
 * @brief MatchSpy implementation.
 */
/* Copyright (C) 2007,2008,2009 Olly Betts
 * Copyright (C) 2007,2009 Lemur Consulting Ltd
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
#include <xapian/matchspy.h>

#include <xapian/document.h>
#include <xapian/error.h>
#include <xapian/queryparser.h>
#include <xapian/registry.h>

#include <map>
#include <string>
#include <vector>

#include "autoptr.h"
#include "debuglog.h"
#include "omassert.h"
#include "serialise.h"
#include "stringutils.h"
#include "str.h"

#include <cfloat>
#include <cmath>

using namespace std;

namespace Xapian {

MatchSpy::~MatchSpy() {}

MatchSpy *
MatchSpy::clone() const {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - clone() method unimplemented");
}

string
MatchSpy::name() const {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - name() method unimplemented");
}

string
MatchSpy::serialise() const {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - serialise() method unimplemented");
}

MatchSpy *
MatchSpy::unserialise(const string &, const Registry &) const {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - unserialise() method unimplemented");
}

string
MatchSpy::serialise_results() const {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - serialise_results() method unimplemented");
}

void
MatchSpy::merge_results(const string &) {
    throw UnimplementedError("MatchSpy not suitable for use with remote searches - merge_results() method unimplemented");
}

string
MatchSpy::get_description() const {
    return "Xapian::MatchSpy()";
}


/** Compare two StringAndFrequency objects.
 *
 *  The comparison is firstly by frequency (higher is better), then by string
 *  (earlier lexicographic sort is better).
 */
class StringAndFreqCmpByFreq {
  public:
    /// Default constructor
    StringAndFreqCmpByFreq() {}

    /// Return true if a has a higher frequency than b.
    /// If equal, compare by the str, to provide a stable sort order.
    bool operator()(const StringAndFrequency &a,
		    const StringAndFrequency &b) const {
	if (a.get_frequency() > b.get_frequency()) return true;
	if (a.get_frequency() < b.get_frequency()) return false;
	if (a.get_string() > b.get_string()) return false;
	return true;
    }
};

/** Get the most frequent items from a map from string to frequency.
 *
 *  This takes input such as that returned by @a
 *  ValueCountMatchSpy::get_values(), and returns a vector of the most
 *  frequent items in the input.
 *
 *  @param result A vector which will be filled with the most frequent
 *                items, in descending order of frequency.  Items with
 *                the same frequency will be sorted in ascending
 *                alphabetical order.
 *
 *  @param items The map from string to frequency, from which the most
 *               frequent items will be selected.
 *
 *  @param maxitems The maximum number of items to return.
 */
static void
get_most_frequent_items(vector<StringAndFrequency> & result,
			const map<string, doccount> & items,
			size_t maxitems)
{
    result.clear();
    result.reserve(maxitems);
    StringAndFreqCmpByFreq cmpfn;
    bool is_heap(false);

    for (map<string, doccount>::const_iterator i = items.begin();
	 i != items.end(); i++) {
	Assert(result.size() <= maxitems);
	result.push_back(StringAndFrequency(i->first, i->second));
	if (result.size() > maxitems) {
	    // Make the list back into a heap.
	    if (is_heap) {
		// Only the new element isn't in the right place.
		push_heap(result.begin(), result.end(), cmpfn);
	    } else {
		// Need to build heap from scratch.
		make_heap(result.begin(), result.end(), cmpfn);
		is_heap = true;
	    }
	    pop_heap(result.begin(), result.end(), cmpfn);
	    result.pop_back();
	}
    }

    if (is_heap) {
	sort_heap(result.begin(), result.end(), cmpfn);
    } else {
	sort(result.begin(), result.end(), cmpfn);
    }
}

void
ValueCountMatchSpy::operator()(const Document &doc, weight) {
    ++total;
    string val(doc.get_value(slot));
    if (!val.empty()) ++values[val];
}

void
ValueCountMatchSpy::get_top_values(vector<StringAndFrequency> & result,
				   size_t maxvalues) const
{
    get_most_frequent_items(result, values, maxvalues);
}

MatchSpy *
ValueCountMatchSpy::clone() const {
    return new ValueCountMatchSpy(slot);
}

string
ValueCountMatchSpy::name() const {
    return "Xapian::ValueCountMatchSpy";
}

string
ValueCountMatchSpy::serialise() const {
    string result;
    result += encode_length(slot);
    return result;
}

MatchSpy *
ValueCountMatchSpy::unserialise(const string & s, const Registry &) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    valueno new_slot = decode_length(&p, end, false);
    if (p != end) {
	throw NetworkError("Junk at end of serialised ValueCountMatchSpy");
    }

    return new ValueCountMatchSpy(new_slot);
}

string
ValueCountMatchSpy::serialise_results() const {
    LOGCALL(REMOTE, string, "ValueCountMatchSpy::serialise_results", NO_ARGS);
    string result;
    result += encode_length(total);
    result += encode_length(values.size());
    for (map<string, doccount>::const_iterator i = values.begin();
	 i != values.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second);
    }
    RETURN(result);
}

void
ValueCountMatchSpy::merge_results(const string & s) {
    LOGCALL_VOID(REMOTE, "ValueCountMatchSpy::merge_results", s);
    const char * p = s.data();
    const char * end = p + s.size();

    total += decode_length(&p, end, false);

    map<string, doccount>::size_type items = decode_length(&p, end, false);
    while (p != end) {
	while (items != 0) {
	    size_t vallen = decode_length(&p, end, true);
	    string val(p, vallen);
	    p += vallen;
	    doccount freq = decode_length(&p, end, false);
	    values[val] += freq;
	    --items;
	}
    }
}

string
ValueCountMatchSpy::get_description() const {
    return "Xapian::ValueCountMatchSpy(" + str(total) +
	    " docs seen, looking in " + str(values.size()) + " slots)";
}

}
