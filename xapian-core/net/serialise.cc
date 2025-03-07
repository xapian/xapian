/** @file
 * @brief functions to convert Xapian objects to strings and back
 */
/* Copyright (C) 2006-2024 Olly Betts
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
#include "pack.h"
#include "serialise.h"
#include "serialise-double.h"
#include "stringutils.h"
#include "weight/weightinternal.h"

#include <string>
#include <string_view>

using namespace std;

string
serialise_stats(const Xapian::Weight::Internal &stats)
{
    string result;

    pack_uint(result, stats.total_length);
    pack_uint(result, stats.collection_size);
    pack_uint(result, stats.rset_size);
    pack_uint(result, stats.db_doclength_lower_bound);
    pack_uint(result, stats.db_doclength_upper_bound -
		      stats.db_doclength_lower_bound);
    pack_uint(result, stats.db_unique_terms_lower_bound);
    pack_uint(result, stats.db_unique_terms_upper_bound -
		      stats.db_unique_terms_lower_bound);
    pack_bool(result, stats.have_max_part);

    pack_uint(result, stats.termfreqs.size());
    string_view prev_term;
    for (auto&& i : stats.termfreqs) {
	const string& term = i.first;
	// We reduce the size of the encoding by reusing any prefix which is in
	// common with the previous term.  This is much more compact if there
	// are a lot of terms, especially if they share a prefix such as an
	// OP_WILDCARD which expands to a lot of terms.
	size_t reuse = common_prefix_length(prev_term, term, 255);
	size_t append = term.size() - reuse;
	if (reuse == prev_term.size() && usual(term.size() <= 255)) {
	    // Reuse the whole of the previous term.  In this case we store the
	    // new length for the reuse count, which is longer than a valid
	    // reuse count.  This saves a byte (or two if the new term is
	    // >= 128 bytes long).
	    AssertRel(term.size(), >, reuse);
	    result += char(term.size());
	} else {
	    result += char(reuse);
	    pack_uint(result, append);
	}
	result.append(term.data() + reuse, append);
	pack_uint(result, i.second.termfreq);
	if (stats.rset_size != 0)
	    pack_uint(result, i.second.reltermfreq);
	pack_uint(result, i.second.collfreq);
	if (stats.have_max_part)
	    result += serialise_double(i.second.max_part);
	prev_term = term;
    }

    return result;
}

void
unserialise_stats(const char* p, const char* p_end,
		  Xapian::Weight::Internal& stat)
{
    size_t n;
    if (!unpack_uint(&p, p_end, &stat.total_length) ||
	!unpack_uint(&p, p_end, &stat.collection_size) ||
	!unpack_uint(&p, p_end, &stat.rset_size) ||
	!unpack_uint(&p, p_end, &stat.db_doclength_lower_bound) ||
	!unpack_uint(&p, p_end, &stat.db_doclength_upper_bound) ||
	!unpack_uint(&p, p_end, &stat.db_unique_terms_lower_bound) ||
	!unpack_uint(&p, p_end, &stat.db_unique_terms_upper_bound) ||
	!unpack_bool(&p, p_end, &stat.have_max_part) ||
	!unpack_uint(&p, p_end, &n)) {
	unpack_throw_serialisation_error(p);
    }
    stat.db_doclength_upper_bound += stat.db_doclength_lower_bound;
    stat.db_unique_terms_upper_bound += stat.db_unique_terms_lower_bound;

    string term;
    for ( ; n; --n) {
	Xapian::doccount termfreq;
	Xapian::doccount reltermfreq = 0;
	Xapian::termcount collfreq;
	if (p == p_end) {
	    unpack_throw_serialisation_error(p);
	}
	size_t reuse = static_cast<unsigned char>(*p++);
	size_t append;
	if (reuse <= term.size()) {
	    term.resize(reuse);
	    if (!unpack_uint(&p, p_end, &append) ||
		size_t(p_end - p) < append) {
		unpack_throw_serialisation_error(p);
	    }
	} else {
	    append = reuse - term.size();
	}
	term.append(p, append);
	p += append;
	if (!unpack_uint(&p, p_end, &termfreq) ||
	    (stat.rset_size != 0 && !unpack_uint(&p, p_end, &reltermfreq)) ||
	    !unpack_uint(&p, p_end, &collfreq)) {
	    unpack_throw_serialisation_error(p);
	}
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
    if (rset.internal) {
	Xapian::docid lastdid = 0;
	for (Xapian::docid did : rset.internal->docs) {
	    pack_uint(result, did - lastdid - 1);
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
	if (!unpack_uint(&p, p_end, &inc)) {
	    unpack_throw_serialisation_error(p);
	}
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
    pack_uint(result, n);
    Xapian::ValueIterator value;
    for (value = doc.values_begin(); value != doc.values_end(); ++value) {
	pack_uint(result, value.get_valueno());
	pack_string(result, *value);
	--n;
    }
    Assert(n == 0);

    n = doc.termlist_count();
    pack_uint(result, n);
    Xapian::TermIterator term;
    for (term = doc.termlist_begin(); term != doc.termlist_end(); ++term) {
	pack_string(result, *term);
	pack_uint(result, term.get_wdf());

	size_t x = term.positionlist_count();
	pack_uint(result, x);
	Xapian::PositionIterator pos;
	Xapian::termpos oldpos = 0;
	for (pos = term.positionlist_begin(); pos != term.positionlist_end(); ++pos) {
	    pack_uint(result, *pos - oldpos);
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
unserialise_document(string_view s)
{
    Xapian::Document doc;
    const char * p = s.data();
    const char * p_end = p + s.size();

    size_t n_values;
    if (!unpack_uint(&p, p_end, &n_values)) {
	unpack_throw_serialisation_error(p);
    }
    string value;
    for ( ; n_values; --n_values) {
	Xapian::valueno slot;
	if (!unpack_uint(&p, p_end, &slot) ||
	    !unpack_string(&p, p_end, value)) {
	    unpack_throw_serialisation_error(p);
	}
	doc.add_value(slot, value);
    }

    size_t n_terms;
    if (!unpack_uint(&p, p_end, &n_terms)) {
	unpack_throw_serialisation_error(p);
    }
    string term;
    for ( ; n_terms; --n_terms) {
	Xapian::termcount wdf;
	if (!unpack_string(&p, p_end, term) ||
	    !unpack_uint(&p, p_end, &wdf)) {
	    unpack_throw_serialisation_error(p);
	}
	// Set all the wdf using add_term, then pass wdf_inc 0 to add_posting.
	doc.add_term(term, wdf);

	size_t n_pos;
	if (!unpack_uint(&p, p_end, &n_pos)) {
	    unpack_throw_serialisation_error(p);
	}
	Xapian::termpos pos = 0;
	for ( ; n_pos; --n_pos) {
	    Xapian::termpos inc;
	    if (!unpack_uint(&p, p_end, &inc)) {
		unpack_throw_serialisation_error(p);
	    }
	    pos += inc;
	    doc.add_posting(term, pos, 0);
	}
    }

    doc.set_data(string_view(p, p_end - p));
    return doc;
}
