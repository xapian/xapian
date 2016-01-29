/** @file matchspy.cc
 * @brief MatchSpy implementation.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
 * Copyright (C) 2007,2009 Lemur Consulting Ltd
 * Copyright (C) 2010 Richard Boulton
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
#include "noreturn.h"
#include "omassert.h"
#include "net/length.h"
#include "stringutils.h"
#include "str.h"
#include "termlist.h"

#include <cfloat>
#include <cmath>

using namespace std;
using namespace Xapian;
using Xapian::Internal::intrusive_ptr;

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

XAPIAN_NORETURN(static void unsupported_method());
static void unsupported_method() {
    throw Xapian::InvalidOperationError("Method not supported for this type of termlist");
}

/// A termlist iterator over the contents of a ValueCountMatchSpy
class ValueCountTermList : public TermList {
  private:
    map<string, Xapian::doccount>::const_iterator it;
    bool started;
    intrusive_ptr<Xapian::ValueCountMatchSpy::Internal> spy;
  public:

    explicit ValueCountTermList(ValueCountMatchSpy::Internal * spy_)
	: spy(spy_)
    {
	it = spy->values.begin();
	started = false;
    }

    string get_termname() const {
	Assert(started);
	Assert(!at_end());
	return it->first;
    }

    Xapian::doccount get_termfreq() const {
	Assert(started);
	Assert(!at_end());
	return it->second;
    }

    TermList * next() {
	if (!started) {
	    started = true;
	} else {
	    Assert(!at_end());
	    ++it;
	}
	return NULL;
    }

    TermList * skip_to(const string & term) {
	while (it != spy->values.end() && it->first < term) {
	    ++it;
	}
	started = true;
	return NULL;
    }

    bool at_end() const {
	Assert(started);
	return it == spy->values.end();
    }

    Xapian::termcount get_approx_size() const { unsupported_method(); return 0; }
    Xapian::termcount get_wdf() const { unsupported_method(); return 0; }
    Xapian::PositionIterator positionlist_begin() const {
	unsupported_method();
	return Xapian::PositionIterator();
    }
    Xapian::termcount positionlist_count() const { unsupported_method(); return 0; }
};

/** A string with a corresponding frequency.
 */
class StringAndFrequency {
    std::string str;
    Xapian::doccount frequency;
  public:
    /// Construct a StringAndFrequency object.
    StringAndFrequency(const std::string & str_, Xapian::doccount frequency_)
	    : str(str_), frequency(frequency_) {}

    /// Return the string.
    std::string get_string() const { return str; }

    /// Return the frequency.
    Xapian::doccount get_frequency() const { return frequency; }
};

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
	return a.get_string() < b.get_string();
    }
};

/// A termlist iterator over a vector of StringAndFrequency objects.
class StringAndFreqTermList : public TermList {
  private:
    vector<StringAndFrequency>::const_iterator it;
    bool started;
  public:
    vector<StringAndFrequency> values;

    /** init should be called after the values have been set, but before
     *  iteration begins.
     */
    void init() {
	it = values.begin();
	started = false;
    }

    string get_termname() const {
	Assert(started);
	Assert(!at_end());
	return it->get_string();
    }

    Xapian::doccount get_termfreq() const {
	Assert(started);
	Assert(!at_end());
	return it->get_frequency();
    }

    TermList * next() {
	if (!started) {
	    started = true;
	} else {
	    Assert(!at_end());
	    ++it;
	}
	return NULL;
    }

    TermList * skip_to(const string & term) {
	while (it != values.end() && it->get_string() < term) {
	    ++it;
	}
	started = true;
	return NULL;
    }

    bool at_end() const {
	Assert(started);
	return it == values.end();
    }

    Xapian::termcount get_approx_size() const { unsupported_method(); return 0; }
    Xapian::termcount get_wdf() const { unsupported_method(); return 0; }
    Xapian::PositionIterator positionlist_begin() const {
	unsupported_method();
	return Xapian::PositionIterator();
    }
    Xapian::termcount positionlist_count() const { unsupported_method(); return 0; }
};

/** Get the most frequent items from a map from string to frequency.
 *
 *  This takes input such as that in ValueCountMatchSpy::Internal::values and
 *  returns a vector of the most frequent items in the input.
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
	 i != items.end(); ++i) {
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
ValueCountMatchSpy::operator()(const Document &doc, double) {
    Assert(internal.get());
    ++(internal->total);
    string val(doc.get_value(internal->slot));
    if (!val.empty()) ++(internal->values[val]);
}

TermIterator
ValueCountMatchSpy::values_begin() const
{
    Assert(internal.get());
    return Xapian::TermIterator(new ValueCountTermList(internal.get()));
}

TermIterator
ValueCountMatchSpy::top_values_begin(size_t maxvalues) const
{
    Assert(internal.get());
    AutoPtr<StringAndFreqTermList> termlist(new StringAndFreqTermList);
    get_most_frequent_items(termlist->values, internal->values, maxvalues);
    termlist->init();
    return Xapian::TermIterator(termlist.release());
}

MatchSpy *
ValueCountMatchSpy::clone() const {
    Assert(internal.get());
    return new ValueCountMatchSpy(internal->slot);
}

string
ValueCountMatchSpy::name() const {
    return "Xapian::ValueCountMatchSpy";
}

string
ValueCountMatchSpy::serialise() const {
    Assert(internal.get());
    string result;
    result += encode_length(internal->slot);
    return result;
}

MatchSpy *
ValueCountMatchSpy::unserialise(const string & s, const Registry &) const
{
    const char * p = s.data();
    const char * end = p + s.size();

    valueno new_slot;
    decode_length(&p, end, new_slot);
    if (p != end) {
	throw NetworkError("Junk at end of serialised ValueCountMatchSpy");
    }

    return new ValueCountMatchSpy(new_slot);
}

string
ValueCountMatchSpy::serialise_results() const {
    LOGCALL(REMOTE, string, "ValueCountMatchSpy::serialise_results", NO_ARGS);
    Assert(internal.get());
    string result;
    result += encode_length(internal->total);
    result += encode_length(internal->values.size());
    for (map<string, doccount>::const_iterator i = internal->values.begin();
	 i != internal->values.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second);
    }
    RETURN(result);
}

void
ValueCountMatchSpy::merge_results(const string & s) {
    LOGCALL_VOID(REMOTE, "ValueCountMatchSpy::merge_results", s);
    Assert(internal.get());
    const char * p = s.data();
    const char * end = p + s.size();

    Xapian::doccount n;
    decode_length(&p, end, n);
    internal->total += n;

    map<string, doccount>::size_type items;
    decode_length(&p, end, items);
    while (p != end) {
	while (items != 0) {
	    size_t vallen;
	    decode_length_and_check(&p, end, vallen);
	    string val(p, vallen);
	    p += vallen;
	    doccount freq;
	    decode_length(&p, end, freq);
	    internal->values[val] += freq;
	    --items;
	}
    }
}

string
ValueCountMatchSpy::get_description() const {
    string d = "ValueCountMatchSpy(";
    if (internal.get()) {
	d += str(internal->total);
	d += " docs seen, looking in ";
	d += str(internal->values.size());
	d += " slots)";
    } else {
	d += ")";
    }
    return d;
}
