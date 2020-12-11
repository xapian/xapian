/** @file
 * @brief A TermList in a honey database.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2018 Olly Betts
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

#include "honey_termlist.h"

#include "expand/expandweight.h"

using namespace std;

[[noreturn]]
static void
throw_database_corrupt(const char* item, const char* pos)
{
    string message;
    if (pos != NULL) {
	message = "Value overflow unpacking termlist: ";
    } else {
	message = "Out of data unpacking termlist: ";
    }
    message += item;
    throw Xapian::DatabaseCorruptError(message);
}

HoneyTermList::HoneyTermList(const HoneyDatabase* db_, Xapian::docid did_)
    : db(db_), did(did_)
{
    if (!db->termlist_table.get_exact_entry(HoneyTermListTable::make_key(did),
					    data)) {
	// Document with no terms or values, or one which doesn't exist.
	termlist_size = 0;
	doclen = 0;
	pos = end = data.data();
	return;
    }

    pos = data.data();
    end = pos + data.size();

    if (pos == end)
	throw_database_corrupt("No termlist data", pos);

    size_t slot_enc_size = *pos++;

    // If the top bit is clear we have a 7-bit bitmap of slots used.
    if (slot_enc_size & 0x80) {
	slot_enc_size &= 0x7f;
	if (slot_enc_size == 0) {
	    if (!unpack_uint(&pos, end, &slot_enc_size)) {
		throw Xapian::DatabaseCorruptError("Termlist encoding corrupt");
	    }
	}

	// Skip encoded slot data.
	pos += slot_enc_size;
    }

    if (pos == end) {
	// Document with values but no terms.
	termlist_size = 0;
	doclen = 0;
	return;
    }

    if (!unpack_uint(&pos, end, &termlist_size)) {
	throw_database_corrupt("termlist length", pos);
    }
    ++termlist_size;

    if (!unpack_uint(&pos, end, &doclen)) {
	throw_database_corrupt("doclen", pos);
    }
}

Xapian::termcount
HoneyTermList::get_approx_size() const
{
    return termlist_size;
}

void
HoneyTermList::accumulate_stats(Xapian::Internal::ExpandStats& stats) const
{
    Assert(!at_end());
    stats.accumulate(shard_index,
		     current_wdf,
		     doclen,
		     get_termfreq(),
		     db->get_doccount());
}

std::string
HoneyTermList::get_termname() const
{
    Assert(!at_end());
    return current_term;
}

Xapian::termcount
HoneyTermList::get_wdf() const
{
    Assert(!at_end());
    return current_wdf;
}

Xapian::doccount
HoneyTermList::get_termfreq() const
{
    Assert(!at_end());
    if (current_termfreq == 0)
	db->get_freqs(current_term, &current_termfreq, NULL);
    return current_termfreq;
}

TermList*
HoneyTermList::next()
{
    Assert(!at_end());

    if (pos == end) {
	// Set pos to NULL so at_end() returns true.
	pos = NULL;
	return NULL;
    }

    current_wdf = 0;

    if (!current_term.empty()) {
	size_t reuse = static_cast<unsigned char>(*pos++);
	if (reuse > current_term.size()) {
	    current_wdf = reuse / (current_term.size() + 1);
	    reuse = reuse % (current_term.size() + 1);
	}
	current_term.resize(reuse);
    }

    if (current_wdf) {
	--current_wdf;
    } else {
	if (!unpack_uint(&pos, end, &current_wdf)) {
	    throw_database_corrupt("wdf", pos);
	}
    }

    if (pos == end)
	throw_database_corrupt("term", NULL);

    size_t append = static_cast<unsigned char>(*pos++);
    if (size_t(end - pos) < append)
	throw_database_corrupt("term", NULL);

    current_term.append(pos, append);
    pos += append;

    // Indicate that termfreq hasn't been read for the current term.
    current_termfreq = 0;

    return NULL;
}

TermList*
HoneyTermList::skip_to(const std::string& term)
{
    while (!at_end() && current_term < term) {
	HoneyTermList::next();
    }
    return NULL;
}

bool
HoneyTermList::at_end() const
{
    return pos == NULL;
}

Xapian::termcount
HoneyTermList::positionlist_count() const
{
    return db->position_table.positionlist_count(did, current_term);
}

PositionList*
HoneyTermList::positionlist_begin() const
{
    return db->open_position_list(did, current_term);
}
