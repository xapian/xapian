/* quartz_termlist.cc: Termlists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include "om/omerror.h"
#include "quartz_termlist.h"
#include "quartz_lexicon.h"
#include "quartz_utils.h"
#include "utils.h"

void
QuartzTermList::set_entries(QuartzBufferedTable * table,
			    om_docid did,
			    OmTermIterator t,
			    const OmTermIterator &t_end,
			    quartz_doclen_t doclen_,
			    bool store_termfreqs)
{
    DEBUGCALL_STATIC(DB, void, "QuartzTermList::set_entries", table << ", " << did << ", " << t << ", " << t_end << ", " << doclen_ << ", " << store_termfreqs);
    QuartzDbTag * tag = table->get_or_make_tag(quartz_docid_to_key(did));

    tag->value = "";
    unsigned int size = 0;
    for ( ; t != t_end; ++t) {
	tag->value += pack_string(*t);
	tag->value += pack_uint(t.get_wdf());
	if (store_termfreqs) tag->value += pack_uint(t.get_termfreq());
	++size;
    }
    string v = pack_uint(doclen_);
    v += pack_uint(size);
    v += pack_bool(store_termfreqs);
    tag->value = v + tag->value;

    DEBUGLINE(DB, "QuartzTermList::set_entries() - new entry is `" + tag->value + "'");
}

void
QuartzTermList::delete_termlist(QuartzBufferedTable * table, om_docid did)
{
    DEBUGCALL_STATIC(DB, void, "QuartzTermList::delete_termlist", table << ", " << did);
    table->delete_tag(quartz_docid_to_key(did));
}


QuartzTermList::QuartzTermList(RefCntPtr<const Database> this_db_,
			       const QuartzTable * table_,
			       const QuartzTable * lexicon_table_,
			       om_docid did,
			       om_doccount doccount_)
	: this_db(this_db_),
	  table(table_),
	  lexicon_table(lexicon_table_),
	  have_finished(false),
	  current_wdf(0),
	  has_termfreqs(false),
	  current_termfreq(0),
	  doccount(doccount_)
{
    DEBUGCALL(DB, void, "QuartzTermList", "[this_db_], " << table_ << ", " << lexicon_table_ << ", " << did << ", " << doccount_);
    QuartzDbKey key(quartz_docid_to_key(did));

    if (!table->get_exact_entry(key, termlist_part))
	throw OmDocNotFoundError("Can't read termlist for document "
				 + om_tostring(did) + ": Not found");

    DEBUGLINE(DB, "QuartzTermList::QuartzTermList() - data is `" + termlist_part.value + "'");

    pos = termlist_part.value.data();
    end = pos + termlist_part.value.size();

    // Read doclen
    if (!unpack_uint(&pos, end, &doclen)) {
	if (pos != 0) throw OmRangeError("doclen out of range.");
	throw OmDatabaseCorruptError("Unexpected end of data when reading doclen.");
    }

    // Read termlist_size
    if (!unpack_uint(&pos, end, &termlist_size)) {
	if (pos != 0) throw OmRangeError("Size of termlist out of range.");
	throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
    }

    // Read has_termfreqs
    if (!unpack_bool(&pos, end, &has_termfreqs)) {
	Assert(pos == 0);
	throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
    }
}

om_termcount
QuartzTermList::get_approx_size() const
{
    DEBUGCALL(DB, om_termcount, "QuartzTermList::get_approx_size", "");
    RETURN(termlist_size);
}

quartz_doclen_t
QuartzTermList::get_doclength() const
{
    DEBUGCALL(DB, quartz_doclen_t, "QuartzTermList::get_doclength", "");
    RETURN(doclen);
}


TermList *
QuartzTermList::next()
{
    DEBUGCALL(DB, TermList *, "QuartzTermList::next", "");
    if (pos == end) {
	have_finished = true;
	RETURN(0);
    }

    // Read termname
    if (!unpack_string(&pos, end, current_tname)) {
	if (pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	throw OmRangeError("Size of termname out of range, in termlist.");
    }

    // Read wdf
    if (!unpack_uint(&pos, end, &current_wdf)) {
	if (pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	throw OmRangeError("Size of wdf out of range, in termlist.");
    }
    
    // Read termfreq, if stored
    if (has_termfreqs) {
	if (!unpack_uint(&pos, end, &current_termfreq)) {
	    if (pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	    throw OmRangeError("Size of term frequency out of range, in termlist.");
	}
    } else {
	current_termfreq = 0;
    }
 
    DEBUGLINE(DB, "QuartzTermList::next() - " <<
		  "current_tname=" << current_tname <<
		  "current_wdf=" << current_wdf <<
		  "current_termfreq=" << current_termfreq);
    RETURN(0);
}

bool
QuartzTermList::at_end() const
{
    DEBUGCALL(DB, bool, "QuartzTermList::at_end", "");
    RETURN(have_finished);
}

om_termname
QuartzTermList::get_termname() const
{
    DEBUGCALL(DB, om_termname, "QuartzTermList::get_termname", "");
    RETURN(current_tname);
}

om_termcount
QuartzTermList::get_wdf() const
{
    DEBUGCALL(DB, om_termcount, "QuartzTermList::get_wdf", "");
    RETURN(current_wdf);
}

om_doccount
QuartzTermList::get_termfreq() const
{
    DEBUGCALL(DB, om_doccount, "QuartzTermList::get_termfreq", "");
    if (current_termfreq == 0) {
	// If not found, value of current_termfreq will be unchanged from 0.
	QuartzLexicon::get_entry(lexicon_table,
				 current_tname,
				 &current_termfreq);
    }

    RETURN(current_termfreq);
}

OmExpandBits
QuartzTermList::get_weighting() const
{
    DEBUGCALL(DB, OmExpandBits, "QuartzTermList::get_weighting", "");
    Assert(!have_finished);
    Assert(wt != NULL);

    return wt->get_bits(current_wdf, doclen, get_termfreq(), doccount);
}
