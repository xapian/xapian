/* quartz_lexicon.cc: Lexicon in a quartz database
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
#include "quartz_lexicon.h"
#include "quartz_utils.h"
#include "omassert.h"
#include "omdebug.h"

void
QuartzLexicon::make_key(string & key, const om_termname & tname)
{
    DEBUGCALL_STATIC(DB, void, "QuartzLexicon::make_key", "string( " << key << "), " << tname);
    if (tname.empty())
	throw OmInvalidArgumentError("QuartzLexicon: Term names must not be null.");
    key = pack_string(tname);
}

void
QuartzLexicon::parse_entry(const std::string & data, om_doccount * termfreq)
{
    DEBUGCALL_STATIC(DB, void, "QuartzLexicon::parse_entry", data << ", " << termfreq);
    const char * pos = data.data();
    const char * end = pos + data.size();

    if (!unpack_uint(&pos, end, termfreq)) {
	if (pos == 0) throw OmDatabaseCorruptError("Unexpected end of data when reading termfreq from lexicon.");
	throw OmRangeError("Size of termfreq out of range, in lexicon.");
    }
}

void
QuartzLexicon::make_entry(std::string & data, om_doccount termfreq)
{
    DEBUGCALL_STATIC(DB, void, "QuartzLexicon::make_entry", data << ", " << termfreq);
    data = pack_uint(termfreq);
}

void
QuartzLexicon::increment_termfreq(QuartzBufferedTable * table,
				  const om_termname & tname)
{
    DEBUGCALL_STATIC(DB, void, "QuartzLexicon::increment_termfreq", table << ", " << tname);
    string key;
    make_key(key, tname);
    string * tag = table->get_or_make_tag(key);

    om_doccount termfreq;

    if (tag->empty()) {
	termfreq = 1;
    } else {
	// Read tag
	parse_entry(*tag, &termfreq);

	// Do the increment
	termfreq += 1;
    }

    // Store modified tag
    make_entry(*tag, termfreq);
}

void
QuartzLexicon::decrement_termfreq(QuartzBufferedTable * table,
				  const om_termname & tname)
{
    DEBUGCALL_STATIC(DB, void, "QuartzLexicon::decrement_termfreq", table << ", " << tname);
    string key;
    make_key(key, tname);
    string * tag = table->get_or_make_tag(key);

    om_doccount termfreq;
    if (tag->empty()) {
	// Have no tag - this shouldn't really happen - in a production
	// build its probably okay to ignore it though.
	Assert(!tag->empty());
	return;
    }

    // Read tag
    parse_entry(*tag, &termfreq);

    // Do the decrement
    termfreq -= 1;

    if (termfreq == 0) {
	// Delete the tag
	table->delete_tag(key);
    } else {
	// Store modified tag
	make_entry(*tag, termfreq);
    }
}

bool
QuartzLexicon::get_entry(const QuartzTable * table,
			 const om_termname & tname,
			 om_doccount * termfreq)
{
    DEBUGCALL_STATIC(DB, bool, "QuartzLexicon::get_entry", table << ", " << tname << ", " << termfreq);
    // This may be called internally.
    string tag;
    string key;
    make_key(key, tname);
    bool found = table->get_exact_entry(key, tag);
    if (!found) RETURN(false);

    parse_entry(tag, termfreq);

    RETURN(true);
}

