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
#include "api/rsetinternal.h"
#include "length.h"
#include "serialise.h"
#include "serialise-double.h"
#include "weight/weightinternal.h"

#include <memory>
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
serialise_rset(const Xapian::RSet &rset)
{
    string result;
    if (rset.internal.get()) {
	Xapian::docid lastdid = 0;
	for (Xapian::docid did : rset.internal->docs) {
	    result += encode_length(did - lastdid - 1);
	    lastdid = did;
	}
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
    AssertEq(n, 0);

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
