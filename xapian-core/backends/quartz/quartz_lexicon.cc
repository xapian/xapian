/* quartz_lexicon.cc: Lexicon in a quartz database
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

#include "quartz_lexicon.h"
#include "quartz_utils.h"
#include "omassert.h"

void
QuartzLexicon::make_key(QuartzDbKey & key,
			const om_termname & tname)
{
    if (tname.size() == 0)
	throw OmInvalidArgumentError("QuartzLexicon: Term names must not be null.");
    key.value = pack_string(tname);
}

om_termid
QuartzLexicon::allocate_termid(QuartzBufferedTable * table)
{
    QuartzDbKey key;
    key.value = pack_string("");
    QuartzDbTag * tag = table->get_or_make_tag(key);

    om_termid tid;
    if (tag->value.size() == 0) {
	tid = 1;
    } else {
	std::string::const_iterator pos = tag->value.begin();
	std::string::const_iterator end = pos + tag->value.size();

	if (!unpack_uint(&pos, end, &tid)) {
	    if(pos == 0)
		throw OmDatabaseCorruptError("Unexpected end of data when reading next termid from lexicon.");
	    else throw OmRangeError("Size of next termid out of range, in lexicon.");
	}
    }
    tag->value = pack_uint(tid + 1);

    return tid;
}

void
QuartzLexicon::parse_entry(const std::string & data,
			   om_termid * tid,
			   om_doccount * termfreq)
{
    std::string::const_iterator pos = data.begin();
    std::string::const_iterator end = pos + data.size();

    if (!unpack_uint(&pos, end, tid)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termid from lexicon.");
	else throw OmRangeError("Size of termid out of range, in lexicon.");
    }
    if (!unpack_uint(&pos, end, termfreq)) {
	if(pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termfreq from lexicon.");
	else throw OmRangeError("Size of termfreq out of range, in lexicon.");
    }
}

void
QuartzLexicon::make_entry(std::string & data,
			  om_termid tid,
			  om_doccount termfreq)
{
    data = pack_uint(tid);
    data += pack_uint(termfreq);
}

void
QuartzLexicon::increment_termfreq(QuartzBufferedTable * table,
				  const om_termname & tname,
				  om_termid * tidptr)
{
    QuartzDbKey key;
    make_key(key, tname);
    QuartzDbTag * tag = table->get_or_make_tag(key);

    om_termid tid;
    om_doccount termfreq;

    if (tag->value.size() == 0) {
	// New tag - allocate a new termid.
	tid = allocate_termid(table);
	termfreq = 1;
    } else {
	// Read tag
	parse_entry(tag->value, &tid, &termfreq);

	// Do the increment
	termfreq += 1;
    }

    // Store modified tag
    make_entry(tag->value, tid, termfreq);

    if (tidptr != 0) *tidptr = tid;
}

void
QuartzLexicon::decrement_termfreq(QuartzBufferedTable * table,
				  const om_termname & tname)
{
    QuartzDbKey key;
    make_key(key, tname);
    QuartzDbTag * tag = table->get_or_make_tag(key);

    om_termid tid;
    om_doccount termfreq;
    if (tag->value.size() == 0) {
	// Have no tag - this shouldn't really happen - in a production
	// build its probably okay to ignore it though.
	Assert(tag->value.size() != 0);
	return;
    }

    // Read tag
    parse_entry(tag->value, &tid, &termfreq);

    // Do the decrement
    termfreq -= 1;

    if (termfreq == 0) {
	// Delete the tag
	table->delete_tag(key);
    } else {
	// Store modified tag
	make_entry(tag->value, tid, termfreq);
    }
}

bool
QuartzLexicon::get_entry(const QuartzTable * table,
			 const om_termname & tname,
			 om_termid * tid,
			 om_doccount * termfreq)
{
    // This may be called internally.
    QuartzDbTag tag;
    QuartzDbKey key;
    make_key(key, tname);
    bool found = table->get_exact_entry(key, tag);
    if (!found) return false;

    parse_entry(tag.value, tid, termfreq);

    return true;
}

