/** @file xapian-check-brass.cc
 * @brief Check consistency of a brass table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include "xapian-check-brass.h"

#include "bitstream.h"

#include "internaltypes.h"

#include "brass_check.h"
#include "brass_cursor.h"
#include "brass_table.h"
#include "brass_types.h"
#include "pack.h"
#include "valuestats.h"

#include <xapian.h>

#include "autoptr.h"
#include <iostream>

using namespace std;

static inline bool
is_user_metadata_key(const string & key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xc0';
}

struct VStats : public ValueStats {
    Xapian::doccount freq_real;

    VStats() : ValueStats(), freq_real(0) {}
};

size_t
check_brass_table(const char * tablename, string filename,
		  brass_revision_number_t * rev_ptr, int opts,
		  vector<Xapian::termcount> & doclens,
		  Xapian::docid db_last_docid)
{
    filename += '.';

    // Check the btree structure.
    BrassTableCheck::check(tablename, filename, rev_ptr, opts);

    // Now check the brass structures inside the btree.
    BrassTable table(tablename, filename, true);
    if (rev_ptr) {
	table.open(*rev_ptr);
    } else {
	table.open();
    }
    AutoPtr<BrassCursor> cursor(table.cursor_get());

    size_t errors = 0;

    cursor->find_entry(string());
    cursor->next(); // Skip the empty entry.

    if (strcmp(tablename, "postlist") == 0) {
	// Now check the structure of each postlist in the table.
	map<Xapian::valueno, VStats> valuestats;
	string current_term;
	Xapian::docid lastdid = 0;
	Xapian::termcount termfreq = 0, collfreq = 0;
	Xapian::termcount tf = 0, cf = 0;
	bool have_metainfo_key = false;

	// The first key/tag pair should be the METAINFO - though this may be
	// missing if the table only contains user-metadata.
	if (!cursor->after_end()) {
	    if (cursor->current_key == string("", 1)) {
		have_metainfo_key = true;
		cursor->read_tag();
		// Check format of the METAINFO key.
		totlen_t total_doclen;
		Xapian::docid last_docid;
		Xapian::termcount doclen_lbound;
		Xapian::termcount doclen_ubound;
		Xapian::termcount wdf_ubound;

		const char * data = cursor->current_tag.data();
		const char * end = data + cursor->current_tag.size();
		if (!unpack_uint(&data, end, &last_docid)) {
		    cout << "Tag containing meta information is corrupt (couldn't read last_docid)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &doclen_lbound)) {
		    cout << "Tag containing meta information is corrupt (couldn't read doclen_lbound)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &wdf_ubound)) {
		    cout << "Tag containing meta information is corrupt (couldn't read wdf_ubound)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &doclen_ubound)) {
		    cout << "Tag containing meta information is corrupt (couldn't read doclen_ubound)." << endl;
		    ++errors;
		} else if (!unpack_uint_last(&data, end, &total_doclen)) {
		    cout << "Tag containing meta information is corrupt (couldn't read total_doclen)." << endl;
		    ++errors;
		} else if (data != end) {
		    cout << "Tag containing meta information is corrupt (junk at end)." << endl;
		    ++errors;
		}
		cursor->next();
	    }
	}

	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    if (is_user_metadata_key(key)) {
		// User metadata can be anything, so we can't do any particular
		// checks on it other than to check that the tag isn't empty.
		cursor->read_tag();
		if (cursor->current_tag.empty()) {
		    cout << "User metadata item is empty" << endl;
		    ++errors;
		}
		continue;
	    }

	    if (!have_metainfo_key) {
		have_metainfo_key = true;
		cout << "METAINFO key missing from postlist table" << endl;
		++errors;
	    }

	    if (key.size() >= 2 && key[0] == '\0' && key[1] == '\xe0') {
		// doclen chunk
		const char * pos, * end;
		Xapian::docid did = 1;
		if (key.size() > 2) {
		    // Non-initial chunk.
		    pos = key.data();
		    end = pos + key.size();
		    pos += 2;
		    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
			cout << "Error unpacking docid from doclen key" << endl;
			++errors;
			continue;
		    }
		}

		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
		if (key.size() == 2) {
		    // Initial chunk.
		    if (end - pos < 2 || pos[0] || pos[1]) {
			cout << "Initial doclen chunk has nonzero dummy fields" << endl;
			++errors;
			continue;
		    }
		    pos += 2;
		    if (!unpack_uint(&pos, end, &did)) {
			cout << "Failed to unpack firstdid for doclen" << endl;
			++errors;
			continue;
		    }
		    ++did;
		    if (did <= lastdid) {
			cout << "First did in this chunk is <= last in "
			    "prev chunk" << endl;
			++errors;
		    }
		}

		bool is_last_chunk;
		if (!unpack_bool(&pos, end, &is_last_chunk)) {
		    cout << "Failed to unpack last chunk flag for doclen" << endl;
		    ++errors;
		    continue;
		}
		// Read what the final document ID in this chunk is.
		if (!unpack_uint(&pos, end, &lastdid)) {
		    cout << "Failed to unpack increase to last" << endl;
		    ++errors;
		    continue;
		}
		lastdid += did;
		bool bad = false;
		while (true) {
		    Xapian::termcount doclen;
		    if (!unpack_uint(&pos, end, &doclen)) {
			cout << "Failed to unpack doclen" << endl;
			++errors;
			bad = true;
			break;
		    }

		    if (did > db_last_docid) {
			cout << "document id " << did << " in doclen stream "
			     << "is larger than get_last_docid() "
			     << db_last_docid << endl;
			++errors;
		    }

		    if (!doclens.empty()) {
			// In brass, a document without terms doesn't get a
			// termlist entry.
			Xapian::termcount termlist_doclen = 0;
			if (did < doclens.size())
			    termlist_doclen = doclens[did];

			if (doclen != termlist_doclen) {
			    cout << "document id " << did << ": length "
				 << doclen << " doesn't match "
				 << termlist_doclen << " in the termlist table"
				 << endl;
			    ++errors;
			}
		    }

		    if (pos == end) break;

		    Xapian::docid inc;
		    if (!unpack_uint(&pos, end, &inc)) {
			cout << "Failed to unpack docid increase" << endl;
			++errors;
			bad = true;
			break;
		    }
		    ++inc;
		    did += inc;
		    if (did > lastdid) {
			cout << "docid " << did << " > last docid " << lastdid
			     << endl;
			++errors;
		    }
		}
		if (bad) {
		    continue;
		}
		if (is_last_chunk) {
		    if (did != lastdid) {
			cout << "lastdid " << lastdid << " != last did " << did
			     << endl;
			++errors;
		    }
		}

		continue;
	    }

	    if (key.size() >= 2 && key[0] == '\0' && key[1] == '\xd0') {
		// Value stats.
		const char * p = key.data();
		const char * end = p + key.length();
		p += 2;
		Xapian::valueno slot;
		if (!unpack_uint_last(&p, end, &slot)) {
		    cout << "Bad valuestats key (no slot)" << endl;
		    ++errors;
		    continue;
		}

		cursor->read_tag();
		p = cursor->current_tag.data();
		end = p + cursor->current_tag.size();

		VStats & v = valuestats[slot];
		if (!unpack_uint(&p, end, &v.freq)) {
		    if (*p == 0) {
			cout << "Incomplete stats item in value table" << endl;
		    } else {
			cout << "Frequency statistic in value table is too large" << endl;
		    }
		    ++errors;
		    continue;
		}
		if (!unpack_string(&p, end, v.lower_bound)) {
		    if (*p == 0) {
			cout << "Incomplete stats item in value table" << endl;
		    } else {
			cout << "Lower bound statistic in value table is too large" << endl;
		    }
		    ++errors;
		    continue;
		}
		size_t len = end - p;
		if (len == 0) {
		    v.upper_bound = v.lower_bound;
		} else {
		    v.upper_bound.assign(p, len);
		}

		continue;
	    }

	    if (key.size() >= 2 && key[0] == '\0' && key[1] == '\xd8') {
		// Value stream chunk.
		const char * p = key.data();
		const char * end = p + key.length();
		p += 2;
		Xapian::valueno slot;
		if (!unpack_uint(&p, end, &slot)) {
		    cout << "Bad value chunk key (no slot)" << endl;
		    ++errors;
		    continue;
		}
		Xapian::docid did;
		if (!unpack_uint_preserving_sort(&p, end, &did)) {
		    cout << "Bad value chunk key (no docid)" << endl;
		    ++errors;
		    continue;
		}
		if (p != end) {
		    cout << "Bad value chunk key (trailing junk)" << endl;
		    ++errors;
		    continue;
		}

		VStats & v = valuestats[slot];

		cursor->read_tag();
		p = cursor->current_tag.data();
		end = p + cursor->current_tag.size();

		while (true) {
		    string value;
		    if (!unpack_string(&p, end, value)) {
			cout << "Failed to unpack value from chunk" << endl;
			++errors;
			break;
		    }

		    ++v.freq_real;

		    // FIXME: Cross-check that docid did has value slot (and
		    // vice versa - that there's a value here if the slot entry
		    // says so).

		    // FIXME: Check if the bounds are tight?  Or is that better
		    // as a separate tool which can also update the bounds?
		    if (value < v.lower_bound) {
			cout << "Value slot " << slot << " has value below "
				"lower bound: '" << value << "' < '"
			     << v.lower_bound << "'" << endl;
			++errors;
		    } else if (value > v.upper_bound) {
			cout << "Value slot " << slot << " has value above "
				"upper bound: '" << value << "' > '"
			     << v.upper_bound << "'" << endl;
			++errors;
		    }

		    if (p == end) break;
		    Xapian::docid delta;
		    if (!unpack_uint(&p, end, &delta)) {
			cout << "Failed to unpack docid delta from chunk" << endl;
			++errors;
			break;
		    }
		    Xapian::docid new_did = did + delta + 1;
		    if (new_did <= did) {
			cout << "docid overflowed in value chunk" << endl;
			++errors;
			break;
		    }
		    did = new_did;

		    if (did > db_last_docid) {
			cout << "document id " << did << " in value chunk "
			     << "is larger than get_last_docid() "
			     << db_last_docid << endl;
			++errors;
		    }
		}
		continue;
	    }

	    const char * pos, * end;

	    // Get term from key.
	    pos = key.data();
	    end = pos + key.size();

	    string term;
	    Xapian::docid did;
	    if (!unpack_string_preserving_sort(&pos, end, term)) {
		cout << "Error unpacking termname from key" << endl;
		++errors;
		continue;
	    }
	    if (!current_term.empty() && term != current_term) {
		// The term changed unexpectedly.
		if (pos == end) {
		    cout << "No last chunk for term `" << current_term
			 << "'" << endl;
		    current_term.resize(0);
		} else {
		    cout << "Mismatch in follow-on chunk in posting "
			"list for term `" << current_term << "' (got `"
			<< term << "')" << endl;
		    current_term = term;
		    tf = cf = 0;
		    lastdid = 0;
		}
		++errors;
	    }
	    if (pos == end) {
		// First chunk.
		if (term == current_term) {
		    // This probably isn't possible.
		    cout << "First posting list chunk for term `"
			 << term << "' follows previous chunk for the same "
			 "term" << endl;
		    ++errors;
		}
		current_term = term;
		tf = cf = 0;

		// Unpack extra header from first chunk.
		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
		if (!unpack_uint(&pos, end, &termfreq)) {
		    cout << "Failed to unpack termfreq for term `" << term
			 << "'" << endl;
		    ++errors;
		    continue;
		}
		if (!unpack_uint(&pos, end, &collfreq)) {
		    cout << "Failed to unpack collfreq for term `" << term
			 << "'" << endl;
		    ++errors;
		    continue;
		}
		if (!unpack_uint(&pos, end, &did)) {
		    cout << "Failed to unpack firstdid for term `" << term
			 << "'" << endl;
		    ++errors;
		    continue;
		}
		++did;
	    } else {
		// Continuation chunk.
		if (current_term.empty()) {
		    cout << "First chunk for term `" << current_term << "' "
			 "is a continuation chunk" << endl;
		    ++errors;
		    current_term = term;
		}
		AssertEq(current_term, term);
		if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		    cout << "Failed to unpack did from key" << endl;
		    ++errors;
		    continue;
		}
		if (did <= lastdid) {
		    cout << "First did in this chunk is <= last in "
			"prev chunk" << endl;
		    ++errors;
		}
		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
	    }

	    bool is_last_chunk;
	    if (!unpack_bool(&pos, end, &is_last_chunk)) {
		cout << "Failed to unpack last chunk flag" << endl;
		++errors;
		continue;
	    }
	    // Read what the final document ID in this chunk is.
	    if (!unpack_uint(&pos, end, &lastdid)) {
		cout << "Failed to unpack increase to last" << endl;
		++errors;
		continue;
	    }
	    lastdid += did;
	    bool bad = false;
	    while (true) {
		Xapian::termcount wdf;
		if (!unpack_uint(&pos, end, &wdf)) {
		    cout << "Failed to unpack wdf" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		++tf;
		cf += wdf;

		if (pos == end) break;

		Xapian::docid inc;
		if (!unpack_uint(&pos, end, &inc)) {
		    cout << "Failed to unpack docid increase" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		++inc;
		did += inc;
		if (did > lastdid) {
		    cout << "docid " << did << " > last docid " << lastdid
			 << endl;
		    ++errors;
		}
	    }
	    if (bad) {
		continue;
	    }
	    if (is_last_chunk) {
		if (tf != termfreq) {
		    cout << "termfreq " << termfreq << " != # of entries "
			 << tf << endl;
		    ++errors;
		}
		if (cf != collfreq) {
		    cout << "collfreq " << collfreq << " != sum wdf " << cf
			 << endl;
		    ++errors;
		}
		if (did != lastdid) {
		    cout << "lastdid " << lastdid << " != last did " << did
			 << endl;
		    ++errors;
		}
		current_term.resize(0);
	    }
	}
	if (!current_term.empty()) {
	    cout << "Last term `" << current_term << "' has no last chunk"
		 << endl;
	    ++errors;
	}

	map<Xapian::valueno, VStats>::const_iterator i;
	for (i = valuestats.begin(); i != valuestats.end(); ++i) {
	    if (i->second.freq != i->second.freq_real) {
		cout << "Value stats frequency for slot " << i->first << " is "
		     << i->second.freq << " but recounting gives "
		     << i->second.freq_real << endl;
		++errors;
	    }
	}
    } else if (strcmp(tablename, "record") == 0) {
	// Now check the contents of the record table.  Any data is valid as
	// the tag so we don't check the tags.
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		++errors;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		++errors;
	    }
	}
    } else if (strcmp(tablename, "termlist") == 0) {
	// Now check the contents of the termlist table.
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		++errors;
		continue;
	    }

	    if (end - pos == 1 && *pos == '\0') {
		// Value slots used entry.
		cursor->read_tag();

		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();

		if (pos == end) {
		    cout << "Empty value slots used tag" << endl;
		    ++errors;
		    continue;
		}

		Xapian::valueno prev_slot;
		if (!unpack_uint(&pos, end, &prev_slot)) {
		    cout << "Value slot encoding corrupt" << endl;
		    ++errors;
		    continue;
		}

		while (pos != end) {
		    Xapian::valueno slot;
		    if (!unpack_uint(&pos, end, &slot)) {
			cout << "Value slot encoding corrupt" << endl;
			++errors;
			break;
		    }
		    slot += prev_slot + 1;
		    if (slot <= prev_slot) {
			cout << "Value slot number overflowed (" << prev_slot << " -> " << slot << ")" << endl;
			++errors;
		    }
		    prev_slot = slot;
		}
		continue;
	    }

	    if (pos != end) {
		cout << "Extra junk in key" << endl;
		++errors;
		continue;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

	    if (pos == end) {
		// Empty termlist.
		continue;
	    }

	    Xapian::termcount doclen, termlist_size;

	    // Read doclen
	    if (!unpack_uint(&pos, end, &doclen)) {
		if (pos != 0) {
		    cout << "doclen out of range" << endl;
		} else {
		    cout << "Unexpected end of data when reading doclen" << endl;
		}
		++errors;
		continue;
	    }

	    // Read termlist_size
	    if (!unpack_uint(&pos, end, &termlist_size)) {
		if (pos != 0) {
		    cout << "termlist_size out of range" << endl;
		} else {
		    cout << "Unexpected end of data when reading termlist_size" << endl;
		}
		++errors;
		continue;
	    }

	    Xapian::termcount actual_doclen = 0, actual_termlist_size = 0;
	    string current_tname;

	    bool bad = false;
	    while (pos != end) {
		Xapian::doccount current_wdf = 0;
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
			if (pos == 0) {
			    cout << "Unexpected end of data when reading termlist current_wdf" << endl;
			} else {
			    cout << "Size of wdf out of range, in termlist" << endl;
			}
			++errors;
			bad = true;
			break;
		    }
		}

		++actual_termlist_size;
		actual_doclen += current_wdf;
	    }
	    if (bad) {
		continue;
	    }

	    if (termlist_size != actual_termlist_size) {
		cout << "termlist_size != # of entries in termlist" << endl;
		++errors;
	    }
	    if (doclen != actual_doclen) {
		cout << "doclen != sum(wdf)" << endl;
		++errors;
	    }

	    // + 1 so that did is a valid subscript.
	    if (doclens.size() <= did) doclens.resize(did + 1);
	    doclens[did] = actual_doclen;
	}
    } else if (strcmp(tablename, "position") == 0) {
	// Now check the contents of the position table.
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		++errors;
		continue;
	    }
	    if (pos == end) {
		cout << "No termname in key" << endl;
		++errors;
		continue;
	    }

	    if (!doclens.empty()) {
		// In glass, a document without terms doesn't get a
		// termlist entry, so we can't tell the difference
		// easily.
		if (did >= doclens.size() || doclens[did] == 0) {
		    cout << "Position list entry for document " << did
			 << " which doesn't exist or has no terms" << endl;
		    ++errors;
		    continue;
		}
	    }

	    cursor->read_tag();

	    const string & data = cursor->current_tag;
	    pos = data.data();
	    end = pos + data.size();

	    Xapian::termpos pos_last;
	    if (!unpack_uint(&pos, end, &pos_last)) {
		cout << tablename << " table: Position list data corrupt" << endl;
		++errors;
		continue;
	    }
	    if (pos == end) {
		// Special case for single entry position list.
	    } else {
		// Skip the header we just read.
		BitReader rd(data, pos - data.data());
		Xapian::termpos pos_first = rd.decode(pos_last);
		Xapian::termpos pos_size = rd.decode(pos_last - pos_first) + 2;
		vector<Xapian::termpos> positions;
		positions.resize(pos_size);
		positions[0] = pos_first;
		positions.back() = pos_last;
		rd.decode_interpolative(positions, 0, pos_size - 1);
		vector<Xapian::termpos>::const_iterator current_pos = positions.begin();
		Xapian::termpos lastpos = *current_pos++;
		while (current_pos != positions.end()) {
		    Xapian::termpos termpos = *current_pos++;
		    if (termpos <= lastpos) {
			cout << tablename << " table: Positions not strictly monotonically increasing" << endl;
			++errors;
			break;
		    }
		    lastpos = termpos;
		}
	    }
	}
    } else {
	cout << tablename << " table: Don't know how to check structure\n" << endl;
	return errors;
    }

    if (!errors)
	cout << tablename << " table structure checked OK\n" << endl;
    else
	cout << tablename << " table errors found: " << errors << "\n" << endl;

    return errors;
}
