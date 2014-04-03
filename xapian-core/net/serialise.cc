/** @file serialise.cc
 * @brief functions to convert Xapian objects to strings and back
 */
/* Copyright (C) 2006,2007,2008,2009,2010,2011,2014 Olly Betts
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
#include <xapian/error.h>
#include <xapian/positioniterator.h>
#include <xapian/termiterator.h>
#include <xapian/valueiterator.h>

#include "omassert.h"
#include "api/omenquireinternal.h"
#include "length.h"
#include "serialise.h"
#include "serialise-double.h"
#include "weight/weightinternal.h"

#include <string>
#include <cstring>

using namespace std;

string
serialise_error(const Xapian::Error &e)
{
    // The byte before the type name is the type code.
    string result(1, (e.get_type())[-1]);
    result += encode_length(e.get_context().length());
    result += e.get_context();
    result += encode_length(e.get_msg().length());
    result += e.get_msg();
    // The "error string" goes last so we don't need to store its length.
    const char * err = e.get_error_string();
    if (err) result += err;
    return result;
}

void
unserialise_error(const string &serialised_error, const string &prefix,
		  const string &new_context)
{
    // Use c_str() so last string is nul-terminated.
    const char * p = serialised_error.c_str();
    const char * end = p + serialised_error.size();
    if (p != end) {
	char type = *p++;

	size_t len = decode_length(&p, end, true);
	string context(p, len);
	p += len;

	len = decode_length(&p, end, true);
	string msg(prefix);
	msg.append(p, len);
	p += len;

	const char * error_string = (p == end) ? NULL : p;

	if (!new_context.empty()) {
	    if (!context.empty()) {
		msg += "; context was: ";
		msg += context;
	    }
	    context = new_context;
	}

	switch (type) {
#include "xapian/errordispatch.h"
	}
    }

    throw Xapian::InternalError("Unknown remote exception type", new_context);
}

string
serialise_stats(const Xapian::Weight::Internal &stats)
{
    string result;

    result += encode_length(stats.total_length);
    result += encode_length(stats.collection_size);
    result += encode_length(stats.rset_size);
    result += encode_length(stats.total_term_count);

    result += encode_length(stats.termfreqs.size());
    map<string, TermFreqs>::const_iterator i;
    for (i = stats.termfreqs.begin(); i != stats.termfreqs.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second.termfreq);
	if (stats.rset_size != 0)
	    result += encode_length(i->second.reltermfreq);
	result += encode_length(i->second.collfreq);
    }

    return result;
}

void
unserialise_stats(const string &s, Xapian::Weight::Internal & stat)
{
    const char * p = s.data();
    const char * p_end = p + s.size();

    stat.total_length = decode_length(&p, p_end, false);
    stat.collection_size = decode_length(&p, p_end, false);
    stat.rset_size = decode_length(&p, p_end, false);
    stat.total_term_count = decode_length(&p, p_end, false);

    size_t n = decode_length(&p, p_end, false);
    while (n--) {
	size_t len = decode_length(&p, p_end, true);
	string term(p, len);
	p += len;
	Xapian::doccount termfreq(decode_length(&p, p_end, false));
	Xapian::doccount reltermfreq;
	if (stat.rset_size == 0) {
	    reltermfreq = 0;
	} else {
	    reltermfreq = decode_length(&p, p_end, false);
	}
	Xapian::termcount collfreq(decode_length(&p, p_end, false));
	stat.termfreqs.insert(make_pair(term,
					TermFreqs(termfreq,
						  reltermfreq,
						  collfreq)));
    }
}

string
serialise_mset(const Xapian::MSet &mset)
{
    string result;

    result += encode_length(mset.get_firstitem());
    result += encode_length(mset.get_matches_lower_bound());
    result += encode_length(mset.get_matches_estimated());
    result += encode_length(mset.get_matches_upper_bound());
    result += encode_length(mset.get_uncollapsed_matches_lower_bound());
    result += encode_length(mset.get_uncollapsed_matches_estimated());
    result += encode_length(mset.get_uncollapsed_matches_upper_bound());
    result += serialise_double(mset.get_max_possible());
    result += serialise_double(mset.get_max_attained());

    result += serialise_double(mset.internal->percent_factor);

    result += encode_length(mset.size());
    for (Xapian::MSetIterator i = mset.begin(); i != mset.end(); ++i) {
	result += serialise_double(i.get_weight());
	result += encode_length(*i);
	result += encode_length(i.get_collapse_key().size());
	result += i.get_collapse_key();
	result += encode_length(i.get_collapse_count());
    }

    const map<string, Xapian::MSet::Internal::TermFreqAndWeight> &termfreqandwts
	= mset.internal->termfreqandwts;

    map<string, Xapian::MSet::Internal::TermFreqAndWeight>::const_iterator j;
    for (j = termfreqandwts.begin(); j != termfreqandwts.end(); ++j) {
	result += encode_length(j->first.size());
	result += j->first;
	result += encode_length(j->second.termfreq);
	result += serialise_double(j->second.termweight);
    }

    return result;
}

Xapian::MSet
unserialise_mset(const char * p, const char * p_end)
{
    Xapian::doccount firstitem = decode_length(&p, p_end, false);
    Xapian::doccount matches_lower_bound = decode_length(&p, p_end, false);
    Xapian::doccount matches_estimated = decode_length(&p, p_end, false);
    Xapian::doccount matches_upper_bound = decode_length(&p, p_end, false);
    Xapian::doccount uncollapsed_lower_bound = decode_length(&p, p_end, false);
    Xapian::doccount uncollapsed_estimated = decode_length(&p, p_end, false);
    Xapian::doccount uncollapsed_upper_bound = decode_length(&p, p_end, false);
    double max_possible = unserialise_double(&p, p_end);
    double max_attained = unserialise_double(&p, p_end);

    double percent_factor = unserialise_double(&p, p_end);

    vector<Xapian::Internal::MSetItem> items;
    size_t msize = decode_length(&p, p_end, false);
    while (msize-- > 0) {
	double wt = unserialise_double(&p, p_end);
	Xapian::docid did = decode_length(&p, p_end, false);
	size_t len = decode_length(&p, p_end, true);
	string key(p, len);
	p += len;
	Xapian::doccount collapse_cnt = decode_length(&p, p_end, false);
	items.push_back(Xapian::Internal::MSetItem(wt, did, key, collapse_cnt));
    }

    map<string, Xapian::MSet::Internal::TermFreqAndWeight> terminfo;
    while (p != p_end) {
	Xapian::MSet::Internal::TermFreqAndWeight tfaw;
	size_t len = decode_length(&p, p_end, true);
	string term(p, len);
	p += len;
	tfaw.termfreq = decode_length(&p, p_end, false);
	tfaw.termweight = unserialise_double(&p, p_end);
	terminfo.insert(make_pair(term, tfaw));
    }

    return Xapian::MSet(new Xapian::MSet::Internal(
				       firstitem,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       uncollapsed_upper_bound,
				       uncollapsed_lower_bound,
				       uncollapsed_estimated,
				       max_possible, max_attained,
				       items, terminfo, percent_factor));
}

string
serialise_rset(const Xapian::RSet &rset)
{
    const set<Xapian::docid> & items = rset.internal->get_items();
    string result;
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
	did += decode_length(&p, p_end, false) + 1;
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

    size_t n_values = decode_length(&p, p_end, false);
    while (n_values--) {
	Xapian::valueno slot = decode_length(&p, p_end, false);
	size_t len = decode_length(&p, p_end, true);
	doc.add_value(slot, string(p, len));
	p += len;
    }

    size_t n_terms = decode_length(&p, p_end, false);
    while (n_terms--) {
	size_t len = decode_length(&p, p_end, true);
	string term(p, len);
	p += len;

	// Set all the wdf using add_term, then pass wdf_inc 0 to add_posting.
	Xapian::termcount wdf = decode_length(&p, p_end, false);
	doc.add_term(term, wdf);

	size_t n_pos = decode_length(&p, p_end, false);
	Xapian::termpos pos = 0;
	while (n_pos--) {
	    pos += decode_length(&p, p_end, false);
	    doc.add_posting(term, pos, 0);
	}
    }

    doc.set_data(string(p, p_end - p));
    return doc;
}
