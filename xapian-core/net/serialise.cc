/* @file serialise.cc
 * @brief functions to convert Xapian objects to strings and back
 */
/* Copyright (C) 2006 Olly Betts
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
#include "stats.h"
#include "utils.h"

#include <float.h>
#include <math.h>

#include <algorithm>
#include <string>

using namespace std;

string
encode_length(size_t len)
{
    string result;
    if (len < 255) {
	result += static_cast<char>(len);
    } else {
	result += '\xff';
	len -= 255;
	while (true) {
	    char byte = static_cast<char>(len & 0x7f);
	    len >>= 7;
	    if (!len) {
		result += (byte | static_cast<char>(0x80));
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

// The serialisation we use for doubles is inspired by a comp.lang.c post
// by Jens Moeller:
//
// http://groups.google.com/group/comp.lang.c/browse_thread/thread/6558d4653f6dea8b/75a529ec03148c98
//
// The clever part is that the mantissa is encoded as a base-256 number which
// means there's no rounding error provided both ends have FLT_RADIX as some
// power of two.
//
// FLT_RADIX == 2 seems to be ubiquitous on modern UNIX platforms, while
// some older platforms used FLT_RADIX == 16 (IBM machines for example).
// FLT_RADIX == 10 seems to be very rare (the only instance Google finds
// is for a cross-compiler to some TI calculators).

#if FLT_RADIX == 2
# define MAX_MANTISSA_BYTES ((DBL_MANT_DIG + 7 + 7) / 8)
# define MAX_EXP ((DBL_MAX_EXP + 1) / 8)
# define MAX_MANTISSA (1 << (DBL_MAX_EXP & 7))
#elif FLT_RADIX == 16
# define MAX_MANTISSA_BYTES ((DBL_MANT_DIG + 1 + 1) / 2)
# define MAX_EXP ((DBL_MAX_EXP + 1) / 2)
# define MAX_MANTISSA (1 << ((DBL_MAX_EXP & 1) * 4))
#elif
# error FLT_RADIX is a value not currently handled (not 2 or 16)
// # define MAX_MANTISSA_BYTES (sizeof(double) + 1)
#endif

static int base256ify_double(double &v) {
    int exp;
    v = frexp(v, &exp);
    // v is now in the range [0.5, 1.0)
    --exp;
    v = ldexp(v, (exp & 7) + 1);
    // v is now in the range [1.0, 256.0)
    exp >>= 3;
    return exp;
}

std::string serialise_double(double v)
{
    /* First byte:
     *  bit 7 Negative flag
     *  bit 4..6 Mantissa length - 1
     *  bit 0..3 --- 0-13 -> Exponent + 7
     *            \- 14 -> Exponent given by next byte
     *             - 15 -> Exponent given by next 2 bytes
     *
     * Then optional medium (1 byte) or large exponent (2 bytes, lsb first)
     *
     * Then mantissa (0 iff value is 0)
     */

    bool negative = (v < 0.0);

    if (negative) v = -v;

    int exp = base256ify_double(v);

    string result;

    if (exp <= 6 && exp >= -7) {
	unsigned char b = (unsigned char)(exp + 7);
	if (negative) b |= '\x80';
	result += char(b);
    } else {
	if (exp >= -128 && exp < 127) {
	    result += negative ? '\x8e' : '\x0e';
	    result += char(exp + 128);
	} else {
	    if (exp < -32768 || exp > 32767) {
		throw Xapian::InternalError("Insane exponent in floating point number");
	    }
	    result += negative ? '\x8f' : '\x0f';
	    result += char(unsigned(exp + 32768) & 0xff);
	    result += char(unsigned(exp + 32768) >> 8);
	}
    }

    int maxbytes = min(MAX_MANTISSA_BYTES, 8);

    size_t n = result.size();
    do {
	unsigned char byte = static_cast<unsigned char>(v);
	result += (char)byte;
	v -= double(byte);
	v *= 256.0;
    } while (v != 0.0 && --maxbytes);

    n = result.size() - n;
    if (n > 1) {
	Assert(n <= 8);
	result[0] = (unsigned char)result[0] | ((n - 1) << 4);
    }

    return result;
}

double unserialise_double(const char ** p, const char *end)
{
    if (end - *p < 2) {
	throw Xapian::NetworkError("Bad encoded double: insufficient data");
    }
    unsigned char first = *(*p)++;
    if (first == 0 && *(*p) == 0) {
	++*p;
	return 0.0;
    }

    bool negative = first & 0x80;
    size_t mantissa_len = ((first >> 4) & 0x07) + 1;

    int exp = first & 0x0f;
    if (exp >= 14) {
	int bigexp = static_cast<unsigned char>(*(*p)++);
	if (exp == 15) {
	    if (*p == end) {
		throw Xapian::NetworkError("Bad encoded double: short large exponent");
	    }
	    exp = bigexp | (static_cast<unsigned char>(*(*p)++) << 8);
	    exp -= 32768;
	} else {
	    exp = bigexp - 128;
	}
    } else {
	exp -= 7;
    }

    if (size_t(end - *p) < mantissa_len) {
	throw Xapian::NetworkError("Bad encoded double: short mantissa");
    }

    double v = 0.0;

    static double dbl_max_mantissa = DBL_MAX;
    static int dbl_max_exp = base256ify_double(dbl_max_mantissa);
    *p += mantissa_len;
    if (exp > dbl_max_exp ||
	(exp == dbl_max_exp && double(**p) > dbl_max_mantissa)) {
	// The mantissa check should be precise provided that FLT_RADIX
	// is a power of 2.
	v = HUGE_VAL;
    } else {
	const char *q = *p;
	while (mantissa_len--) {
	    v *= 0.00390625; // 1/256
	    v += double(static_cast<unsigned char>(*--q));
	}

	if (exp) v = ldexp(v, exp * 8);

#if 0
	if (v == 0.0) {
	    // FIXME: handle underflow
	}
#endif
    }

    if (negative) v = -v;

    return v;
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

#define XAPIAN_DEFINE_ERROR_BASECLASS(CLASS, BASECLASS)
#define XAPIAN_DEFINE_ERROR_CLASS(CLASS, BASECLASS) \
    if (type == #CLASS) throw Xapian::CLASS(msg, context)
#include <xapian/errortypes.h>

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
