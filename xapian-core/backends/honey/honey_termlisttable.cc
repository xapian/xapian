/** @file glass_termlisttable.cc
 * @brief Subclass of GlassTable which holds termlists.
 */
/* Copyright (C) 2007,2008,2009,2010 Olly Betts
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

#include "glass_termlisttable.h"

#include <xapian/document.h>
#include <xapian/error.h>
#include <xapian/termiterator.h>

#include "debuglog.h"
#include "omassert.h"
#include "pack.h"
#include "stringutils.h"

#include <string>

using namespace std;

void
GlassTermListTable::set_termlist(Xapian::docid did,
				 const Xapian::Document & doc,
				 Xapian::termcount doclen)
{
    LOGCALL_VOID(DB, "GlassTermListTable::set_termlist", did | doc | doclen);

    string tag;
    pack_uint(tag, doclen);

    Xapian::doccount termlist_size = doc.termlist_count();
    if (termlist_size == 0) {
	// doclen is sum(wdf) so should be zero if there are no terms.
	Assert(doclen == 0);
	Assert(doc.termlist_begin() == doc.termlist_end());
	add(make_key(did), string());
	return;
    }

    Xapian::TermIterator t = doc.termlist_begin();
    if (t != doc.termlist_end()) {
	pack_uint(tag, termlist_size);
	string prev_term = *t;

	tag += prev_term.size();
	tag += prev_term;
	pack_uint(tag, t.get_wdf());
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
		pack_uint(tag, wdf);
	    }

	    prev_term = *t;
	    --termlist_size;
	}
    }
    Assert(termlist_size == 0);
    add(make_key(did), tag);
}
