/** @file flint_termlisttable.cc
 * @brief Subclass of FlintTable which holds termlists.
 */
/* Copyright (C) 2007,2008 Olly Betts
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
#include <xapian/termiterator.h>

#include "flint_termlisttable.h"
#include "flint_utils.h"
#include "debuglog.h"
#include "omassert.h"
#include "str.h"
#include "stringutils.h"

#include <string>

using namespace std;

void
FlintTermListTable::set_termlist(Xapian::docid did,
				 const Xapian::Document & doc,
				 flint_doclen_t doclen)
{
    LOGCALL_VOID(DB, "FlintTermListTable::set_termlist", did | doc | doclen);

    Xapian::doccount termlist_size = doc.termlist_count();
    if (termlist_size == 0) {
	// doclen is sum(wdf) so should be zero if there are no terms.
	Assert(doclen == 0);
	Assert(doc.termlist_begin() == doc.termlist_end());
	add(flint_docid_to_key(did), string());
	return;
    }

    string tag = F_pack_uint(doclen);

    Xapian::TermIterator t = doc.termlist_begin();
    if (t != doc.termlist_end()) {
	tag += F_pack_uint(termlist_size);
	string prev_term = *t;

	// Previous database versions encoded a boolean here, which was
	// always false (and F_pack_bool() encodes false as a '0').  We can
	// just omit this and successfully read old and new termlists
	// except in the case where the next byte is a '0' - in this case
	// we need keep the '0' so that the decoder can just skip any '0'
	// it sees in this position (this shouldn't be a common case - 48
	// character terms aren't very common, and the first term
	// alphabetically is likely to be shorter than average).
	if (prev_term.size() == '0') tag += '0';

	tag += char(prev_term.size());
	tag += prev_term;
	tag += F_pack_uint(t.get_wdf());
	--termlist_size;

	while (++t != doc.termlist_end()) {
	    const string & term = *t;
	    // If there's a shared prefix with the previous term, we don't
	    // store it explicitly, but just store the length of the shared
	    // prefix.  In general, this is a big win.
	    size_t reuse = common_prefix_length(prev_term, term);

	    // reuse must be <= prev_term.size(), and we know that value while
	    // decoding.  So if the wdf is small enough that we can multiply it
	    // by (prev_term.size() + 1), add reuse and fit the result in a
	    // byte, then we can pack reuse and the wdf into a single byte and
	    // save ourselves a byte.  We actually need to add one to the wdf
	    // before multiplying so that a wdf of 0 can be detected by the
	    // decoder.
	    size_t packed = 0;
	    Xapian::termcount wdf = t.get_wdf();
	    // If wdf >= 128, then we aren't going to be able to pack it in so
	    // don't even try to avoid the calculation overflowing and making
	    // us think we can.
	    if (wdf < 127)
		packed = (wdf + 1) * (prev_term.size() + 1) + reuse;

	    if (packed && packed < 256) {
		// We can pack the wdf into the same byte.
		tag += char(packed);
		tag += char(term.size() - reuse);
		tag.append(term.data() + reuse, term.size() - reuse);
	    } else {
		tag += char(reuse);
		tag += char(term.size() - reuse);
		tag.append(term.data() + reuse, term.size() - reuse);
		// FIXME: pack wdf after reuse next time we rejig the format
		// incompatibly.
		tag += F_pack_uint(wdf);
	    }

	    prev_term = *t;
	    --termlist_size;
	}
    }
    Assert(termlist_size == 0);
    add(flint_docid_to_key(did), tag);
}

flint_doclen_t
FlintTermListTable::get_doclength(Xapian::docid did) const
{
    LOGCALL(DB, flint_doclen_t, "FlintTermListTable::get_doclength", did);

    string tag;
    if (!get_exact_entry(flint_docid_to_key(did), tag))
	throw Xapian::DocNotFoundError("No termlist found for document " +
				       str(did));

    if (tag.empty()) RETURN(0);

    const char * pos = tag.data();
    const char * end = pos + tag.size();

    flint_doclen_t doclen;
    if (!F_unpack_uint(&pos, end, &doclen)) {
	const char *msg;
	if (pos == 0) {
	    msg = "Too little data for doclen in termlist";
	} else {
	    msg = "Overflowed value for doclen in termlist";
	}
	throw Xapian::DatabaseCorruptError(msg);
    }

    RETURN(doclen);
}
