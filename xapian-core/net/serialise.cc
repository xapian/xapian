/** @file serialise.cc
 * @brief functions to convert Xapian objects to strings and back
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2014,2015,2017 Olly Betts
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

#include <xapian/document.h>
#include <xapian/positioniterator.h>
#include <xapian/termiterator.h>
#include <xapian/valueiterator.h>

#include "omassert.h"
#include "api/omenquireinternal.h"
#include "length.h"
#include "serialise.h"
#include "serialise-double.h"
#include "weight/weightinternal.h"

#include "autoptr.h"
#include <set>
#include <string>

using namespace std;

string
serialise_stats(const Xapian::Weight::Internal &stats)
{
    string result;

    result += encode_length(stats.total_length);
    result += encode_length(stats.collection_size);
    result += encode_length(stats.rset_size);
    result += encode_length(stats.total_term_count);
    result += static_cast<char>(stats.have_max_part);

    result += encode_length(stats.termfreqs.size());
    map<string, TermFreqs>::const_iterator i;
    for (i = stats.termfreqs.begin(); i != stats.termfreqs.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second.termfreq);
	if (stats.rset_size != 0)
	    result += encode_length(i->second.reltermfreq);
	result += encode_length(i->second.collfreq);
	if (stats.have_max_part)
	    result += serialise_double(i->second.max_part);
    }

    return result;
}

void
unserialise_stats(const string &s, Xapian::Weight::Internal & stat)
{
    const char * p = s.data();
    const char * p_end = p + s.size();

    decode_length(&p, p_end, stat.total_length);
    decode_length(&p, p_end, stat.collection_size);
    decode_length(&p, p_end, stat.rset_size);
    decode_length(&p, p_end, stat.total_term_count);
    // If p == p_end, the next decode_length() will report it.
    stat.have_max_part = (p != p_end && *p++);

    size_t n;
    decode_length(&p, p_end, n);
    while (n--) {
	size_t len;
	decode_length_and_check(&p, p_end, len);
	string term(p, len);
	p += len;
	Xapian::doccount termfreq;
	decode_length(&p, p_end, termfreq);
	Xapian::doccount reltermfreq;
	if (stat.rset_size == 0) {
	    reltermfreq = 0;
	} else {
	    decode_length(&p, p_end, reltermfreq);
	}
	Xapian::termcount collfreq;
	decode_length(&p, p_end, collfreq);
	double max_part = 0.0;
	if (stat.have_max_part)
	    max_part = unserialise_double(&p, p_end);
	stat.termfreqs.insert(make_pair(term,
					TermFreqs(termfreq,
						  reltermfreq,
						  collfreq,
						  max_part)));
    }
}

string
serialise_mset(const Xapian::MSet &mset)
{
    string result;

    result += encode_length(mset.get_firstitem());
    // Send back the raw matches_* values.  MSet::get_matches_estimated()
    // rounds the estimate lazily, but MSetPostList::get_termfreq_est()
    // returns the estimate, and the raw estimate is better for that.
    //
    // It is also cleaner that a round-trip through serialisation gives you an
    // object which is as close to the original as possible.
    result += encode_length(mset.internal->matches_lower_bound);
    result += encode_length(mset.internal->matches_estimated);
    result += encode_length(mset.internal->matches_upper_bound);
    result += encode_length(mset.get_uncollapsed_matches_lower_bound());
    result += encode_length(mset.get_uncollapsed_matches_estimated());
    result += encode_length(mset.get_uncollapsed_matches_upper_bound());
    result += serialise_double(mset.get_max_possible());
    result += serialise_double(mset.get_max_attained());

    result += serialise_double(mset.internal->percent_factor);

    result += encode_length(mset.size());
    for (size_t i = 0; i != mset.size(); ++i) {
	const Xapian::Internal::MSetItem & item = mset.internal->items[i];
	result += serialise_double(item.wt);
	result += encode_length(item.did);
	result += encode_length(item.sort_key.size());
	result += item.sort_key;
	result += encode_length(item.collapse_key.size());
	result += item.collapse_key;
	result += encode_length(item.collapse_count);
    }

    if (mset.internal->stats)
	result += serialise_stats(*(mset.internal->stats));

    return result;
}

Xapian::MSet
unserialise_mset(const char * p, const char * p_end)
{
    Xapian::doccount firstitem;
    decode_length(&p, p_end, firstitem);
    Xapian::doccount matches_lower_bound;
    decode_length(&p, p_end, matches_lower_bound);
    Xapian::doccount matches_estimated;
    decode_length(&p, p_end, matches_estimated);
    Xapian::doccount matches_upper_bound;
    decode_length(&p, p_end, matches_upper_bound);
    Xapian::doccount uncollapsed_lower_bound;
    decode_length(&p, p_end, uncollapsed_lower_bound);
    Xapian::doccount uncollapsed_estimated;
    decode_length(&p, p_end, uncollapsed_estimated);
    Xapian::doccount uncollapsed_upper_bound;
    decode_length(&p, p_end, uncollapsed_upper_bound);
    double max_possible = unserialise_double(&p, p_end);
    double max_attained = unserialise_double(&p, p_end);

    double percent_factor = unserialise_double(&p, p_end);

    vector<Xapian::Internal::MSetItem> items;
    size_t msize;
    decode_length(&p, p_end, msize);
    while (msize-- > 0) {
	double wt = unserialise_double(&p, p_end);
	Xapian::docid did;
	decode_length(&p, p_end, did);
	size_t len;
	decode_length_and_check(&p, p_end, len);
	string sort_key(p, len);
	p += len;
	decode_length_and_check(&p, p_end, len);
	string key(p, len);
	p += len;
	Xapian::doccount collapse_cnt;
	decode_length(&p, p_end, collapse_cnt);
	items.push_back(Xapian::Internal::MSetItem(wt, did, key, collapse_cnt));
	swap(items.back().sort_key, sort_key);
    }

    AutoPtr<Xapian::Weight::Internal> stats;
    if (p != p_end) {
	stats.reset(new Xapian::Weight::Internal());
	unserialise_stats(string(p, p_end - p), *(stats.get()));
    }

    Xapian::MSet mset;
    mset.internal = new Xapian::MSet::Internal(
				       firstitem,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       uncollapsed_upper_bound,
				       uncollapsed_lower_bound,
				       uncollapsed_estimated,
				       max_possible, max_attained,
				       items, percent_factor);
    mset.internal->stats = stats.release();
    return mset;
}

string
serialise_rset(const Xapian::RSet &rset)
{
    string result;
    const set<Xapian::docid> & items = rset.internal->get_items();
    set<Xapian::docid>::const_iterator i;
    Xapian::docid lastdid = 0;
    for (i = items.begin(); i != items.end(); ++i) {
	Xapian::docid did = *i;
	result += encode_length(did - lastdid - 1);
	lastdid = did;
    }
    return result;
}

Xapian::RSet
unserialise_rset(const string &s)
{
    Xapian::RSet rset;

    const char * p = s.data();
    const char * p_end = p + s.size();

    Xapian::docid did = 0;
    while (p != p_end) {
	Xapian::docid inc;
	decode_length(&p, p_end, inc);
	did += inc + 1;
	rset.add_document(did);
    }

    return rset;
}

string
serialise_document(const Xapian::Document &doc)
{
    string result;

    size_t n = doc.values_count();
    result += encode_length(n);
    Xapian::ValueIterator value;
    for (value = doc.values_begin(); value != doc.values_end(); ++value) {
	result += encode_length(value.get_valueno());
	result += encode_length((*value).size());
	result += *value;
	--n;
    }
    Assert(n == 0);

    n = doc.termlist_count();
    result += encode_length(n);
    Xapian::TermIterator term;
    for (term = doc.termlist_begin(); term != doc.termlist_end(); ++term) {
	result += encode_length((*term).size());
	result += *term;
	result += encode_length(term.get_wdf());

	size_t x = term.positionlist_count();
	result += encode_length(x);
	Xapian::PositionIterator pos;
	Xapian::termpos oldpos = 0;
	for (pos = term.positionlist_begin(); pos != term.positionlist_end(); ++pos) {
	    Xapian::termpos diff = *pos - oldpos;
	    string delta = encode_length(diff);
	    result += delta;
	    oldpos = *pos;
	    --x;
	}
	Assert(x == 0);
	--n;
    }
    Assert(n == 0);

    result += doc.get_data();
    return result;
}

Xapian::Document
unserialise_document(const string &s)
{
    Xapian::Document doc;
    const char * p = s.data();
    const char * p_end = p + s.size();

    size_t n_values;
    decode_length(&p, p_end, n_values);
    while (n_values--) {
	Xapian::valueno slot;
	decode_length(&p, p_end, slot);
	size_t len;
	decode_length_and_check(&p, p_end, len);
	doc.add_value(slot, string(p, len));
	p += len;
    }

    size_t n_terms;
    decode_length(&p, p_end, n_terms);
    while (n_terms--) {
	size_t len;
	decode_length_and_check(&p, p_end, len);
	string term(p, len);
	p += len;

	// Set all the wdf using add_term, then pass wdf_inc 0 to add_posting.
	Xapian::termcount wdf;
	decode_length(&p, p_end, wdf);
	doc.add_term(term, wdf);

	size_t n_pos;
	decode_length(&p, p_end, n_pos);
	Xapian::termpos pos = 0;
	while (n_pos--) {
	    Xapian::termpos inc;
	    decode_length(&p, p_end, inc);
	    pos += inc;
	    doc.add_posting(term, pos, 0);
	}
    }

    doc.set_data(string(p, p_end - p));
    return doc;
}
