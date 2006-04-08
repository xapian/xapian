/* flint_termlist.cc: Termlists in a flint database
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include <xapian/error.h>
#include "flint_termlist.h"
#include "flint_utils.h"
#include "utils.h"

#include <algorithm>
using namespace std;

void
FlintTermListTable::set_entries(Xapian::docid did,
			    Xapian::TermIterator t,
			    const Xapian::TermIterator &t_end,
			    flint_doclen_t doclen_,
			    bool store_termfreqs)
{
    DEBUGCALL(DB, void, "FlintTermList::set_entries", did << ", " << t << ", " << t_end << ", " << doclen_ << ", " << store_termfreqs);
    string tag = pack_uint(doclen_);

    string v;
    string prev_term;
    Xapian::doccount size = 0;
    for ( ; t != t_end; ++t) {
	bool stored_wdf = false;
	// If there was a previous term, work out how much we can reuse.
	if (!prev_term.empty()) {
	    string::size_type len = min(prev_term.length(), (*t).length());
	    string::size_type i;
	    for (i = 0; i < len; ++i) {
		if (prev_term[i] != (*t)[i]) break;
	    }
	    // See if we can squeeze the wdf into the spare space in a char.
	    string::size_type x;
	    x = (t.get_wdf() + 1) * (prev_term.length() + 1) + i;
	    if (x < 256) {
		// Cool, we can!
		v += char(x);
		stored_wdf = true;
	    } else {
		v += char(i);
	    }
	    v += char((*t).length() - i);
	    v += (*t).substr(i);
	} else {
	    v += char((*t).length());
	    v += *t;
	}
	prev_term = *t;

	if (!stored_wdf) v += pack_uint(t.get_wdf());
	if (store_termfreqs) v += pack_uint(t.get_termfreq());
	++size;
    }
    tag += pack_uint(size);
    tag += pack_bool(store_termfreqs);
    tag += v;
    add(flint_docid_to_key(did), tag);

    DEBUGLINE(DB, "FlintTermList::set_entries() - new entry is `" + tag + "'");
}

void
FlintTermListTable::delete_termlist(Xapian::docid did)
{
    DEBUGCALL_STATIC(DB, void, "FlintTermList::delete_termlist", did);
    del(flint_docid_to_key(did));
}


FlintTermList::FlintTermList(Xapian::Internal::RefCntPtr<const Xapian::Database::Internal> this_db_,
			       const FlintTable * table_,
			       Xapian::docid did_,
			       Xapian::doccount doccount_)
	: this_db(this_db_), did(did_), table(table_),
	  have_finished(false), current_wdf(0), has_termfreqs(false),
	  current_termfreq(0), doccount(doccount_)
{
    DEBUGCALL(DB, void, "FlintTermList", "[this_db_], " << table_ << ", "
	      << did << ", " << doccount_);

    string key(flint_docid_to_key(did));

    if (!table->get_exact_entry(key, termlist_part))
	throw Xapian::DocNotFoundError("Can't read termlist for document "
				 + om_tostring(did) + ": Not found");

    DEBUGLINE(DB, "FlintTermList::FlintTermList() - data is `" + termlist_part + "'");

    pos = termlist_part.data();
    end = pos + termlist_part.size();

    // Read doclen
    if (!unpack_uint(&pos, end, &doclen)) {
	if (pos != 0) throw Xapian::RangeError("doclen out of range.");
	throw Xapian::DatabaseCorruptError("Unexpected end of data when reading doclen.");
    }

    // Read termlist_size
    if (!unpack_uint(&pos, end, &termlist_size)) {
	if (pos != 0) throw Xapian::RangeError("Size of termlist out of range.");
	throw Xapian::DatabaseCorruptError("Unexpected end of data when reading termlist.");
    }

    // Read has_termfreqs
    if (!unpack_bool(&pos, end, &has_termfreqs)) {
	Assert(pos == 0);
	throw Xapian::DatabaseCorruptError("Unexpected end of data when reading termlist.");
    }
}

Xapian::termcount
FlintTermList::get_approx_size() const
{
    DEBUGCALL(DB, Xapian::termcount, "FlintTermList::get_approx_size", "");
    RETURN(termlist_size);
}

flint_doclen_t
FlintTermList::get_doclength() const
{
    DEBUGCALL(DB, flint_doclen_t, "FlintTermList::get_doclength", "");
    RETURN(doclen);
}


TermList *
FlintTermList::next()
{
    DEBUGCALL(DB, TermList *, "FlintTermList::next", "");
    if (pos == end) {
	have_finished = true;
	RETURN(0);
    }
    bool got_wdf = false;
    // If there was a previous term, how much to reuse.
    if (!current_tname.empty()) {
	string::size_type len = static_cast<unsigned char>(*pos++);
	if (len > current_tname.length()) {
	    // The wdf was squeezed into the same byte.
	    current_wdf = len / (current_tname.length() + 1) - 1;
	    len %= (current_tname.length() + 1);
	    got_wdf = true;
	}
	current_tname.resize(len);
    }
    // What to append (note len must be positive, since just truncating
    // always takes us backwards in the sort order)
    string::size_type len = static_cast<unsigned char>(*pos++);
    current_tname.append(pos, len);
    pos += len;

    if (!got_wdf) {
	// Read wdf
	if (!unpack_uint(&pos, end, &current_wdf)) {
	    if (pos == 0) throw Xapian::DatabaseCorruptError("Unexpected end of data when reading termlist.");
	    throw Xapian::RangeError("Size of wdf out of range, in termlist.");
	}
    }

    // Read termfreq, if stored
    if (has_termfreqs) {
	if (!unpack_uint(&pos, end, &current_termfreq)) {
	    if (pos == 0) throw Xapian::DatabaseCorruptError("Unexpected end of data when reading termlist.");
	    throw Xapian::RangeError("Size of term frequency out of range, in termlist.");
	}
    } else {
	current_termfreq = 0;
    }

    DEBUGLINE(DB, "FlintTermList::next() -" <<
		  " current_tname=" << current_tname <<
		  " current_wdf=" << current_wdf <<
		  " current_termfreq=" << current_termfreq);
    RETURN(0);
}

bool
FlintTermList::at_end() const
{
    DEBUGCALL(DB, bool, "FlintTermList::at_end", "");
    RETURN(have_finished);
}

string
FlintTermList::get_termname() const
{
    DEBUGCALL(DB, string, "FlintTermList::get_termname", "");
    RETURN(current_tname);
}

Xapian::termcount
FlintTermList::get_wdf() const
{
    DEBUGCALL(DB, Xapian::termcount, "FlintTermList::get_wdf", "");
    RETURN(current_wdf);
}

Xapian::doccount
FlintTermList::get_termfreq() const
{
    DEBUGCALL(DB, Xapian::doccount, "FlintTermList::get_termfreq", "");
    if (current_termfreq == 0)
	current_termfreq = this_db->get_termfreq(current_tname);
    RETURN(current_termfreq);
}

OmExpandBits
FlintTermList::get_weighting() const
{
    DEBUGCALL(DB, OmExpandBits, "FlintTermList::get_weighting", "");
    Assert(!have_finished);
    Assert(wt != NULL);

    return wt->get_bits(current_wdf, doclen, get_termfreq(), doccount);
}

Xapian::PositionIterator
FlintTermList::positionlist_begin() const
{
    DEBUGCALL(DB, Xapian::PositionIterator, "FlintTermList::positionlist_begin", "");
    return Xapian::PositionIterator(this_db->open_position_list(did, current_tname));
}
