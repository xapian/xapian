/* @file serialise.cc
 * @brief functions to convert Xapian objects to strings and back
 */
/* Copyright (C) 2006,2007 Olly Betts
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

#include "omenquireinternal.h"
#include "serialise.h"
#include "serialise-double.h"
#include "stats.h"
#include "utils.h"

#include <string>

using namespace std;

string
encode_length(size_t len)
{
    string result;
    if (len < 255) {
	result += static_cast<unsigned char>(len);
    } else {
	result += '\xff';
	len -= 255;
	while (true) {
	    unsigned char byte = static_cast<unsigned char>(len & 0x7f);
	    len >>= 7;
	    if (!len) {
		result += (byte | static_cast<unsigned char>(0x80));
		break;
	    }
	    result += byte;
	}
    }
    return result;
}

size_t
decode_length(const char ** p, const char *end)
{
    if (*p == end) {
	throw Xapian::NetworkError("Bad encoded length: no data");
    }

    size_t len = static_cast<unsigned char>(*(*p)++);
    if (len != 0xff) return len;
    len = 0;
    unsigned char ch;
    int shift = 0;
    do {
	if (*p == end || shift > 28)
	    throw Xapian::NetworkError("Bad encoded length: insufficient data");
	ch = *(*p)++;
	len |= size_t(ch & 0x7f) << shift;
	shift += 7;
    } while ((ch & 0x80) == 0);
    return len + 255;
}

string
serialise_error(const Xapian::Error &e)
{
    string result;
    result += encode_length(e.get_type().length());
    result += e.get_type();
    result += encode_length(e.get_context().length());
    result += e.get_context();
    // The "message" goes last so we don't need to store its length.
    result += e.get_msg();
    return result;
}

void
unserialise_error(const string &error_string, const string &prefix,
		  const string &new_context)
{
    const char * p = error_string.data();
    const char * end = p + error_string.size();
    size_t len;
    len = decode_length(&p, end);
    if (len == 7 && memcmp(p, "UNKNOWN", 7) == 0) {
	throw Xapian::InternalError("UNKNOWN");
    }
    string type(p, len);
    p += len;

    len = decode_length(&p, end);
    string context(p, len);
    p += len;

    string msg(prefix);
    msg.append(p, error_string.data() + error_string.size() - p);

    if (!context.empty() && !new_context.empty()) {
	msg += "; context was: ";
	msg += context;
	context = new_context;
    }

#include <xapian/errordispatch.h>

    msg = "Unknown remote exception type " + type + ": " + msg;
    throw Xapian::InternalError(msg, context);
}

string serialise_stats(const Stats &stats)
{
    string result;

    result += encode_length(stats.collection_size);
    result += encode_length(stats.rset_size);
    result += serialise_double(stats.average_length);

    map<string, Xapian::doccount>::const_iterator i;

    result += encode_length(stats.termfreq.size());
    for (i = stats.termfreq.begin(); i != stats.termfreq.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second);
    }

    for (i = stats.reltermfreq.begin(); i != stats.reltermfreq.end(); ++i) {
	result += encode_length(i->first.size());
	result += i->first;
	result += encode_length(i->second);
    }

    return result;
}

Stats
unserialise_stats(const string &s)
{
    const char * p = s.c_str();
    const char * p_end = p + s.size();

    Stats stat;

    stat.collection_size = decode_length(&p, p_end);
    stat.rset_size = decode_length(&p, p_end);
    stat.average_length = unserialise_double(&p, p_end);

    size_t n = decode_length(&p, p_end);
    while (n--) {
	size_t len = decode_length(&p, p_end);
	string term(p, len);
	p += len;
	stat.termfreq.insert(make_pair(term, decode_length(&p, p_end)));
    }

    while (p != p_end) {
	size_t len = decode_length(&p, p_end);
	string term(p, len);
	p += len;
	stat.reltermfreq.insert(make_pair(term, decode_length(&p, p_end)));
    }

    return stat;
}

string
serialise_mset(const Xapian::MSet &mset)
{
    string result;

    result += encode_length(mset.get_firstitem());
    result += encode_length(mset.get_matches_lower_bound());
    result += encode_length(mset.get_matches_estimated());
    result += encode_length(mset.get_matches_upper_bound());
    result += serialise_double(mset.get_max_possible());
    result += serialise_double(mset.get_max_attained());
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
unserialise_mset(const string &s)
{
    const char * p = s.data();
    const char * p_end = p + s.size();

    Xapian::doccount firstitem = decode_length(&p, p_end);
    Xapian::doccount matches_lower_bound = decode_length(&p, p_end);
    Xapian::doccount matches_estimated = decode_length(&p, p_end);
    Xapian::doccount matches_upper_bound = decode_length(&p, p_end);
    Xapian::weight max_possible = unserialise_double(&p, p_end);
    Xapian::weight max_attained = unserialise_double(&p, p_end);
    vector<Xapian::Internal::MSetItem> items;
    size_t msize = decode_length(&p, p_end);
    while (msize-- > 0) {
	Xapian::weight wt = unserialise_double(&p, p_end);
	Xapian::docid did = decode_length(&p, p_end);
	size_t len = decode_length(&p, p_end);
	string key(p, len);
	p += len;
	items.push_back(Xapian::Internal::MSetItem(wt, did, key,
						   decode_length(&p, p_end)));
    }

    map<string, Xapian::MSet::Internal::TermFreqAndWeight> terminfo;
    while (p != p_end) {
	Xapian::MSet::Internal::TermFreqAndWeight tfaw;
	size_t len = decode_length(&p, p_end);
	string term(p, len);
	p += len;
	tfaw.termfreq = decode_length(&p, p_end);
	tfaw.termweight = unserialise_double(&p, p_end);
	terminfo.insert(make_pair(term, tfaw));
    }

    return Xapian::MSet(new Xapian::MSet::Internal(
				       firstitem,
				       matches_upper_bound,
				       matches_lower_bound,
				       matches_estimated,
				       max_possible, max_attained,
				       items, terminfo, 0));
}

string
serialise_rset(const Xapian::RSet &rset)
{
    const set<Xapian::docid> & items = rset.internal->get_items();
    string result;
    set<Xapian::docid>::const_iterator i;
    for (i = items.begin(); i != items.end(); ++i) {
	result += encode_length(*i);
    }
    return result;
}

Xapian::RSet
unserialise_rset(const string &s)
{
    Xapian::RSet rset;

    const char * p = s.data();
    const char * p_end = p + s.size();

    while (p != p_end) {
	rset.add_document(decode_length(&p, p_end));
    }

    return rset;
}

string
serialise_document(const Xapian::Document &doc)
{
    string result;

    size_t n = doc.values_count();
    result += encode_length(doc.values_count());
    Xapian::ValueIterator value;
    for (value = doc.values_begin(); value != doc.values_end(); ++value) {
	result += encode_length(value.get_valueno());
	result += encode_length((*value).size());
	result += *value;
	--n;
    }
    Assert(n == 0);

    result += encode_length(doc.termlist_count());
    Xapian::TermIterator term;
    n = doc.termlist_count();
    for (term = doc.termlist_begin(); term != doc.termlist_end(); ++term) {
	result += encode_length((*term).size());
	result += *term;
	result += encode_length(term.get_wdf());

	result += encode_length(term.positionlist_count());
	Xapian::PositionIterator pos;
	Xapian::termpos oldpos = 0;
	size_t x = term.positionlist_count();
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

    size_t n_values = decode_length(&p, p_end);
    while (n_values--) {
	Xapian::valueno valno = decode_length(&p, p_end);
	size_t len = decode_length(&p, p_end);
	doc.add_value(valno, string(p, len));
	p += len;
    }

    size_t n_terms = decode_length(&p, p_end);
    while (n_terms--) {
	size_t len = decode_length(&p, p_end);
	string term(p, len);
	p += len;

	// Set all the wdf using add_term, then pass wdf_inc 0 to add_posting.
	Xapian::termcount wdf = decode_length(&p, p_end);
	doc.add_term(term, wdf);

	size_t n_pos = decode_length(&p, p_end);
	Xapian::termpos pos = 0;
	while (n_pos--) {
	    pos += decode_length(&p, p_end);
	    doc.add_posting(term, pos, 0);
	}
    }

    doc.set_data(string(p, p_end - p));
    return doc;
}
