/** @file
 * @brief Subclass of HoneyTable which holds termlists.
 */
/* Copyright (C) 2007,2008,2009,2010,2018 Olly Betts
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

#include "honey_termlisttable.h"

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
HoneyTermListTable::set_termlist(Xapian::docid did,
				 const Xapian::Document& doc,
				 Xapian::termcount doclen)
{
    LOGCALL_VOID(DB, "HoneyTermListTable::set_termlist", did | doc | doclen);

    Xapian::doccount termlist_size = doc.termlist_count();

    string tag;
    // FIXME: Need to encode value slots used here.  For now, just encode no
    // slots as used.
    tag += '\0';

    if (usual(termlist_size != 0)) {
	pack_uint(tag, termlist_size - 1);
	pack_uint(tag, doclen);
	Xapian::TermIterator t = doc.termlist_begin();
	string prev_term = *t;

	tag += prev_term.size();
	pack_uint(tag, t.get_wdf());
	tag += prev_term;

	while (++t != doc.termlist_end()) {
	    const string& term = *t;
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
	    Xapian::termcount wdf = t.get_wdf();
	    // If wdf >= 127, then we aren't going to be able to pack it in so
	    // don't even try to avoid any risk of the calculation overflowing
	    // and making us think we can.
	    size_t packed;
	    if (wdf < 127 &&
		(packed = (wdf + 1) * (prev_term.size() + 1) + reuse) < 256) {
		// We can pack the wdf into the same byte.
		tag += char(packed);
	    } else {
		tag += char(reuse);
		pack_uint(tag, wdf);
	    }
	    tag += char(term.size() - reuse);
	    tag.append(term.data() + reuse, term.size() - reuse);

	    prev_term = *t;
	}
    } else {
	Assert(doclen == 0);
	Assert(doc.termlist_begin() == doc.termlist_end());
    }

    if (rare(tag.size() == 1 && tag[0] == '\0')) {
	// No slots used or terms.
	del(make_key(did));
    } else {
	add(make_key(did), tag);
    }
}
