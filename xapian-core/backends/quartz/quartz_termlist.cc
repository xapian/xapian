/* quartz_termlist.cc: Termlists in quartz databases
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "om/omerror.h"
#include "quartz_termlist.h"
#include "quartz_lexicon.h"
#include "quartz_utils.h"
#include "utils.h"

void
QuartzTermList::read_size()
{
    if (!unpack_uint(&pos, end, &termlist_size)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	else throw OmRangeError("Size of termlist out of range.");
    }
}

void
QuartzTermList::write_size(std::string & data,
			   om_termcount size)
{
    data += pack_uint(size);
}

void
QuartzTermList::read_doclen()
{
    if (!unpack_uint(&pos, end, &doclen)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	else throw OmRangeError("Size of termlist out of range.");
    }
}

void
QuartzTermList::write_doclen(std::string & data,
			     quartz_doclen_t doclen)
{
    data += pack_uint(doclen);
}

void
QuartzTermList::read_has_termfreqs()
{
    if (!unpack_bool(&pos, end, &has_termfreqs)) {
	Assert(pos == 0);
	throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
    }
}

void
QuartzTermList::write_has_termfreqs(std::string & data,
				    bool store_termfreqs)
{
    data += pack_bool(store_termfreqs);
}

void
QuartzTermList::read_item()
{
    // Read termname
    if (!unpack_string(&pos, end, current_tname)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	else throw OmRangeError("Size of termname out of range, in termlist.");
    }

    // Read wdf
    if (!unpack_uint(&pos, end, &current_wdf)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	else throw OmRangeError("Size of wdf out of range, in termlist.");
    }
    
    // Read termfreq, if stored
    if (has_termfreqs) {
	if (!unpack_uint(&pos, end, &current_termfreq)) {
	    if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termlist.");
	    else throw OmRangeError("Size of term frequency out of range, in termlist.");
	}
    } else {
	current_termfreq = 0;
    }
}

void
QuartzTermList::write_item(std::string & data,
			   om_termname tname,
			   om_termcount wdf,
			   bool store_termfreq,
			   om_doccount termfreq)
{
    data += pack_string(tname);
    data += pack_uint(wdf);
    if (store_termfreq) {
	data += pack_uint(termfreq);
    }
}


void
QuartzTermList::set_entries(QuartzBufferedTable * table,
			    om_docid did,
			    const OmDocumentContents::document_terms & terms,
			    quartz_doclen_t doclen,
			    bool store_termfreqs)
{
    QuartzDbTag * tag = table->get_or_make_tag(quartz_docid_to_key(did));

    tag->value = "";
    write_doclen(tag->value, doclen);
    write_size(tag->value, terms.size());
    write_has_termfreqs(tag->value, store_termfreqs);

    OmDocumentContents::document_terms::const_iterator i;
    for (i = terms.begin(); i != terms.end(); i++) {
	write_item(tag->value,
		   i->second.tname,
		   i->second.wdf,
		   store_termfreqs,
		   i->second.termfreq);
    }
    DEBUGLINE(DB, "QuartzTermList::set_entries() - new entry is `" + tag->value + "'");
}

void
QuartzTermList::delete_termlist(QuartzBufferedTable * table,
				om_docid did)
{
    table->delete_tag(quartz_docid_to_key(did));
}


QuartzTermList::QuartzTermList(RefCntPtr<const Database> this_db_,
			       const QuartzTable * table_,
			       const QuartzTable * lexicon_table_,
			       om_docid did)
	: this_db(this_db_),
	  table(table_),
	  lexicon_table(lexicon_table_),
	  have_finished(false),
	  current_wdf(0),
	  has_termfreqs(false),
	  current_termfreq(0)
{
    QuartzDbKey key(quartz_docid_to_key(did));

    bool found = table->get_exact_entry(key, termlist_part);
    if (!found)
	throw OmDocNotFoundError("Can't read termlist for document "
				 + om_tostring(did) + ": Not found");

    DEBUGLINE(DB, "QuartzTermList::QuartzTermList() - data is `" + termlist_part.value + "'");

    pos = termlist_part.value.data();
    end = pos + termlist_part.value.size();

    read_doclen();
    read_size();
    read_has_termfreqs();
}

om_termcount
QuartzTermList::get_approx_size() const
{
    return termlist_size;
}

quartz_doclen_t
QuartzTermList::get_doclength() const
{
    return doclen;
}



TermList *
QuartzTermList::next()
{
    if (pos == end) {
	have_finished = true;
    } else {
	read_item();
	DEBUGLINE(DB, "QuartzTermList::next() - " <<
		  "current_tname=" << current_tname <<
		  "current_wdf=" << current_wdf <<
		  "current_termfreq=" << current_termfreq);
    }
    return 0;
}

bool
QuartzTermList::at_end() const
{
    return have_finished;
}

const om_termname
QuartzTermList::get_termname() const
{
    return current_tname;
}

om_termcount
QuartzTermList::get_wdf() const
{
    return current_wdf;
}

om_doccount
QuartzTermList::get_termfreq() const
{
    if (current_termfreq == 0) {
	// FIXME: sort out (thread) locking - the database needs to be locked somehow during this call.
	current_termfreq = 0; // If not found, this value will be unchanged.
	QuartzLexicon::get_entry(lexicon_table,
				 current_tname,
				 0,
				 &current_termfreq);
    }

    return current_termfreq;
}

OmExpandBits
QuartzTermList::get_weighting() const
{
    throw OmUnimplementedError("QuartzTermList::get_weighting() unimplemented");
}

