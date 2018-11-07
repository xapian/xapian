/** @file chert_dbcheck.cc
 * @brief Check consistency of a chert table.
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2016 Olly Betts
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

#include "chert_dbcheck.h"

#include "bitstream.h"

#include "internaltypes.h"

#include "chert_check.h"
#include "chert_cursor.h"
#include "chert_table.h"
#include "chert_types.h"
#include "pack.h"
#include "backends/valuestats.h"

#include <xapian.h>

#include "autoptr.h"
#include <ostream>
#include <vector>

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
check_chert_table(const char * tablename, const string& dir,
		  chert_revision_number_t * rev_ptr, int opts,
		  vector<Xapian::termcount> & doclens,
		  Xapian::doccount doccount, Xapian::docid db_last_docid,
		  ostream * out)
{
    string filename = dir;
    filename += '/';
    filename += tablename;
    filename += '.';

    try {
	// Check the btree structure.
	ChertTableCheck::check(tablename, filename, rev_ptr, opts, out);
    } catch (const Xapian::DatabaseError & e) {
	if (out)
	    *out << "Failed to check B-tree: " << e.get_description() << endl;
	return 1;
    }

    // Now check the chert structures inside the btree.
    ChertTable table(tablename, filename, true);
    if (rev_ptr && *rev_ptr) {
	if (!table.open(*rev_ptr)) {
	    if (out)
		*out << "Failed to reopen table after it checked OK" << endl;
	    return 1;
	}
    } else {
	table.open();
    }
    AutoPtr<ChertCursor> cursor(table.cursor_get());

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
	Xapian::doccount num_doclens = 0;
	bool have_metainfo_key = false;

	// The first key/tag pair should be the METAINFO - though this may be
	// missing if the table only contains user-metadata.
	if (!cursor->after_end()) {
	    if (cursor->current_key == string("", 1)) {
		have_metainfo_key = true;
		cursor->read_tag();
		// Check format of the METAINFO key.
		Xapian::totallength total_doclen;
		Xapian::docid last_docid;
		Xapian::termcount doclen_lbound;
		Xapian::termcount doclen_ubound;
		Xapian::termcount wdf_ubound;

		const char * data = cursor->current_tag.data();
		const char * end = data + cursor->current_tag.size();
		if (!unpack_uint(&data, end, &last_docid)) {
		    if (out)
			*out << "Tag containing meta information is corrupt (couldn't read last_docid)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &doclen_lbound)) {
		    if (out)
			*out << "Tag containing meta information is corrupt (couldn't read doclen_lbound)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &wdf_ubound)) {
		    if (out)
			*out << "Tag containing meta information is corrupt (couldn't read wdf_ubound)." << endl;
		    ++errors;
		} else if (!unpack_uint(&data, end, &doclen_ubound)) {
		    if (out)
			*out << "Tag containing meta information is corrupt (couldn't read doclen_ubound)." << endl;
		    ++errors;
		} else if (!unpack_uint_last(&data, end, &total_doclen)) {
		    if (out)
			*out << "Tag containing meta information is corrupt (couldn't read total_doclen)." << endl;
		    ++errors;
		} else if (data != end) {
		    if (out)
			*out << "Tag containing meta information is corrupt (junk at end)." << endl;
		    ++errors;
		}
		cursor->next();
	    }
	}

	bool seen_doclen_initial_chunk = false;
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    if (is_user_metadata_key(key)) {
		// User metadata can be anything, so we can't do any particular
		// checks on it other than to check that the tag isn't empty.
		cursor->read_tag();
		if (cursor->current_tag.empty()) {
		    if (out)
			*out << "User metadata item is empty" << endl;
		    ++errors;
		}
		continue;
	    }

	    if (!have_metainfo_key) {
		have_metainfo_key = true;
		if (out)
		    *out << "METAINFO key missing from postlist table" << endl;
		++errors;
	    }

	    if (key.size() >= 2 && key[0] == '\0' && key[1] == '\xe0') {
		// doclen chunk
		const char * pos, * end;
		Xapian::docid did = 1;
		if (key.size() > 2) {
		    // Non-initial chunk.
		    if (!seen_doclen_initial_chunk) {
			if (out)
			    *out << "Doclen initial chunk missing" << endl;
			++errors;
		    }
		    pos = key.data();
		    end = pos + key.size();
		    pos += 2;
		    if (!C_unpack_uint_preserving_sort(&pos, end, &did)) {
			if (out)
			    *out << "Error unpacking docid from doclen key" << endl;
			++errors;
			continue;
		    }
		    if (did <= lastdid) {
			if (out)
			    *out << "First did in this chunk is <= last in "
				    "prev chunk" << endl;
			++errors;
		    }
		}

		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
		if (key.size() == 2) {
		    // Initial chunk.
		    seen_doclen_initial_chunk = true;
		    if (end - pos < 2 || pos[0] || pos[1]) {
			if (out)
			    *out << "Initial doclen chunk has nonzero dummy fields" << endl;
			++errors;
			continue;
		    }
		    pos += 2;
		    if (!unpack_uint(&pos, end, &did)) {
			if (out)
			    *out << "Failed to unpack firstdid for doclen" << endl;
			++errors;
			continue;
		    }
		    ++did;
		}

		bool is_last_chunk;
		if (!unpack_bool(&pos, end, &is_last_chunk)) {
		    if (out)
			*out << "Failed to unpack last chunk flag for doclen" << endl;
		    ++errors;
		    continue;
		}
		// Read what the final document ID in this chunk is.
		if (!unpack_uint(&pos, end, &lastdid)) {
		    if (out)
			*out << "Failed to unpack increase to last" << endl;
		    ++errors;
		    continue;
		}
		lastdid += did;
		bool bad = false;
		while (true) {
		    Xapian::termcount doclen;
		    if (!unpack_uint(&pos, end, &doclen)) {
			if (out)
			    *out << "Failed to unpack doclen" << endl;
			++errors;
			bad = true;
			break;
		    }

		    ++num_doclens;

		    if (did > db_last_docid) {
			if (out)
			    *out << "document id " << did << " in doclen "
				    "stream is larger than get_last_docid() "
				 << db_last_docid << endl;
			++errors;
		    }

		    if (!doclens.empty()) {
			// In chert, a document without terms doesn't get a
			// termlist entry.
			Xapian::termcount termlist_doclen = 0;
			if (did < doclens.size())
			    termlist_doclen = doclens[did];

			if (doclen != termlist_doclen) {
			    if (out)
				*out << "document id " << did << ": length "
				     << doclen << " doesn't match "
				     << termlist_doclen << " in the termlist "
					"table" << endl;
			    ++errors;
			}
		    }

		    if (pos == end) break;

		    Xapian::docid inc;
		    if (!unpack_uint(&pos, end, &inc)) {
			if (out)
			    *out << "Failed to unpack docid increase" << endl;
			++errors;
			bad = true;
			break;
		    }
		    ++inc;
		    did += inc;
		    if (did > lastdid) {
			if (out)
			    *out << "docid " << did << " > last docid "
				 << lastdid << endl;
			++errors;
		    }
		}
		if (bad) {
		    continue;
		}
		if (is_last_chunk) {
		    if (did != lastdid) {
			if (out)
			    *out << "lastdid " << lastdid << " != last did "
				 << did << endl;
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
		    if (out)
			*out << "Bad valuestats key (no slot)" << endl;
		    ++errors;
		    continue;
		}

		cursor->read_tag();
		p = cursor->current_tag.data();
		end = p + cursor->current_tag.size();

		VStats & v = valuestats[slot];
		if (!unpack_uint(&p, end, &v.freq)) {
		    if (out) {
			if (*p == 0) {
			    *out << "Incomplete stats item in value table";
			} else {
			    *out << "Frequency statistic in value table is too large";
			}
			*out << endl;
		    }
		    ++errors;
		    continue;
		}
		if (!unpack_string(&p, end, v.lower_bound)) {
		    if (out) {
			if (*p == 0) {
			    *out << "Incomplete stats item in value table";
			} else {
			    *out << "Lower bound statistic in value table is too large";
			}
			*out << endl;
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
		    if (out)
			*out << "Bad value chunk key (no slot)" << endl;
		    ++errors;
		    continue;
		}
		Xapian::docid did;
		if (!C_unpack_uint_preserving_sort(&p, end, &did)) {
		    if (out)
			*out << "Bad value chunk key (no docid)" << endl;
		    ++errors;
		    continue;
		}
		if (p != end) {
		    if (out)
			*out << "Bad value chunk key (trailing junk)" << endl;
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
			if (out)
			    *out << "Failed to unpack value from chunk" << endl;
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
			if (out)
			    *out << "Value slot " << slot << " has value "
				    "below lower bound: '" << value << "' < '"
				 << v.lower_bound << "'" << endl;
			++errors;
		    } else if (value > v.upper_bound) {
			if (out)
			    *out << "Value slot " << slot << " has value "
				    "above upper bound: '" << value << "' > '"
				 << v.upper_bound << "'" << endl;
			++errors;
		    }

		    if (p == end) break;
		    Xapian::docid delta;
		    if (!unpack_uint(&p, end, &delta)) {
			if (out)
			    *out << "Failed to unpack docid delta from chunk"
				 << endl;
			++errors;
			break;
		    }
		    Xapian::docid new_did = did + delta + 1;
		    if (new_did <= did) {
			if (out)
			    *out << "docid overflowed in value chunk" << endl;
			++errors;
			break;
		    }
		    did = new_did;

		    if (did > db_last_docid) {
			if (out)
			    *out << "document id " << did << " in value chunk "
				    "is larger than get_last_docid() "
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
		if (out)
		    *out << "Error unpacking termname from key" << endl;
		++errors;
		continue;
	    }
	    if (!current_term.empty() && term != current_term) {
		// The term changed unexpectedly.
		if (pos == end) {
		    if (out)
			*out << "No last chunk for term '" << current_term
			     << "'" << endl;
		    current_term.resize(0);
		} else {
		    if (out)
			*out << "Mismatch in follow-on chunk in posting list "
				"for term '" << current_term << "' (got '"
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
		    if (out)
			*out << "First posting list chunk for term '" << term
			     << "' follows previous chunk for the same term"
			     << endl;
		    ++errors;
		}
		current_term = term;
		tf = cf = 0;

		// Unpack extra header from first chunk.
		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
		if (!unpack_uint(&pos, end, &termfreq)) {
		    if (out)
			*out << "Failed to unpack termfreq for term '" << term
			     << "'" << endl;
		    ++errors;
		    continue;
		}
		if (!unpack_uint(&pos, end, &collfreq)) {
		    if (out)
			*out << "Failed to unpack collfreq for term '" << term
			     << "'" << endl;
		    ++errors;
		    continue;
		}
		if (!unpack_uint(&pos, end, &did)) {
		    if (out)
			*out << "Failed to unpack firstdid for term '" << term
			     << "'" << endl;
		    ++errors;
		    continue;
		}
		++did;
	    } else {
		// Continuation chunk.
		if (current_term.empty()) {
		    if (out)
			*out << "First chunk for term '" << current_term
			     << "' is a continuation chunk" << endl;
		    ++errors;
		    current_term = term;
		}
		AssertEq(current_term, term);
		if (!C_unpack_uint_preserving_sort(&pos, end, &did)) {
		    if (out)
			*out << "Failed to unpack did from key" << endl;
		    ++errors;
		    continue;
		}
		if (did <= lastdid) {
		    if (out)
			*out << "First did in this chunk is <= last in "
				"prev chunk" << endl;
		    ++errors;
		}
		cursor->read_tag();
		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();
	    }

	    bool is_last_chunk;
	    if (!unpack_bool(&pos, end, &is_last_chunk)) {
		if (out)
		    *out << "Failed to unpack last chunk flag" << endl;
		++errors;
		continue;
	    }
	    // Read what the final document ID in this chunk is.
	    if (!unpack_uint(&pos, end, &lastdid)) {
		if (out)
		    *out << "Failed to unpack increase to last" << endl;
		++errors;
		continue;
	    }
	    lastdid += did;
	    bool bad = false;
	    while (true) {
		Xapian::termcount wdf;
		if (!unpack_uint(&pos, end, &wdf)) {
		    if (out)
			*out << "Failed to unpack wdf" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		++tf;
		cf += wdf;

		if (pos == end) break;

		Xapian::docid inc;
		if (!unpack_uint(&pos, end, &inc)) {
		    if (out)
			*out << "Failed to unpack docid increase" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		++inc;
		did += inc;
		if (did > lastdid) {
		    if (out)
			*out << "docid " << did << " > last docid " << lastdid
			     << endl;
		    ++errors;
		}
	    }
	    if (bad) {
		continue;
	    }
	    if (is_last_chunk) {
		if (tf != termfreq) {
		    if (out)
			*out << "termfreq " << termfreq << " != # of entries "
			     << tf << endl;
		    ++errors;
		}
		if (cf != collfreq) {
		    if (out)
			*out << "collfreq " << collfreq << " != sum wdf " << cf
			     << endl;
		    ++errors;
		}
		if (did != lastdid) {
		    if (out)
			*out << "lastdid " << lastdid << " != last did " << did
			     << endl;
		    ++errors;
		}
		current_term.resize(0);
	    }
	}
	if (!current_term.empty()) {
	    if (out)
		*out << "Last term '" << current_term << "' has no last chunk"
		     << endl;
	    ++errors;
	}

	if (num_doclens != doccount && doccount != Xapian::doccount(-1)) {
	    if (out)
		*out << "Document length list has " << num_doclens
		     << " entries, should be " << doccount << endl;
	    ++errors;
	}

	map<Xapian::valueno, VStats>::const_iterator i;
	for (i = valuestats.begin(); i != valuestats.end(); ++i) {
	    if (i->second.freq != i->second.freq_real) {
		if (out)
		    *out << "Value stats frequency for slot " << i->first
			 << " is " << i->second.freq << " but recounting "
			    "gives " << i->second.freq_real << endl;
		++errors;
	    }
	}
    } else if (strcmp(tablename, "record") == 0) {
	if (table.get_entry_count() != doccount &&
	    doccount != Xapian::doccount(-1)) {
	    if (out)
		*out << "Document data entry count (" << table.get_entry_count()
		     << ") != get_doccount() (" << doccount << ")" << endl;
	    ++errors;
	}

	// Now check the contents of the record table.  Any data is valid as
	// the tag so we don't check the tags.
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!C_unpack_uint_preserving_sort(&pos, end, &did)) {
		if (out)
		    *out << "Error unpacking docid from key" << endl;
		++errors;
	    } else if (pos != end) {
		if (out)
		    *out << "Extra junk in key" << endl;
		++errors;
	    } else {
		if (did > db_last_docid) {
		    if (out)
			*out << "document id " << did << " in docdata table "
				"is larger than get_last_docid() "
			     << db_last_docid << endl;
		    ++errors;
		}
	    }
	}
    } else if (strcmp(tablename, "termlist") == 0) {
	// Now check the contents of the termlist table.
	Xapian::doccount num_termlists = 0;
	Xapian::doccount num_slotsused_entries = 0;
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!C_unpack_uint_preserving_sort(&pos, end, &did)) {
		if (out)
		    *out << "Error unpacking docid from key" << endl;
		++errors;
		continue;
	    }

	    if (did > db_last_docid) {
		if (out)
		    *out << "document id " << did << " in termlist table "
			    "is larger than get_last_docid() "
			 << db_last_docid << endl;
		++errors;
	    }

	    if (end - pos == 1 && *pos == '\0') {
		// Value slots used entry.
		++num_slotsused_entries;
		cursor->read_tag();

		pos = cursor->current_tag.data();
		end = pos + cursor->current_tag.size();

		if (pos == end) {
		    if (out)
			*out << "Empty value slots used tag" << endl;
		    ++errors;
		    continue;
		}

		Xapian::valueno prev_slot;
		if (!unpack_uint(&pos, end, &prev_slot)) {
		    if (out)
			*out << "Value slot encoding corrupt" << endl;
		    ++errors;
		    continue;
		}

		while (pos != end) {
		    Xapian::valueno slot;
		    if (!unpack_uint(&pos, end, &slot)) {
			if (out)
			    *out << "Value slot encoding corrupt" << endl;
			++errors;
			break;
		    }
		    slot += prev_slot + 1;
		    if (slot <= prev_slot) {
			if (out)
			    *out << "Value slot number overflowed ("
				 << prev_slot << " -> " << slot << ")" << endl;
			++errors;
		    }
		    prev_slot = slot;
		}
		continue;
	    }

	    if (pos != end) {
		if (out)
		    *out << "Extra junk in key" << endl;
		++errors;
		continue;
	    }

	    ++num_termlists;
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
		if (out) {
		    if (pos != 0) {
			*out << "doclen out of range";
		    } else {
			*out << "Unexpected end of data when reading doclen";
		    }
		    *out << endl;
		}
		++errors;
		continue;
	    }

	    // Read termlist_size
	    if (!unpack_uint(&pos, end, &termlist_size)) {
		if (out) {
		    if (pos != 0) {
			*out << "termlist_size out of range";
		    } else {
			*out << "Unexpected end of data when reading "
				"termlist_size";
		    }
		    *out << endl;
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
			if (out) {
			    if (pos == 0) {
				*out << "Unexpected end of data when reading "
					"termlist current_wdf";
			    } else {
				*out << "Size of wdf out of range in termlist";
			    }
			    *out << endl;
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
		if (out)
		    *out << "termlist_size != # of entries in termlist" << endl;
		++errors;
	    }
	    if (doclen != actual_doclen) {
		if (out)
		    *out << "doclen != sum(wdf)" << endl;
		++errors;
	    }

	    // + 1 so that did is a valid subscript.
	    if (doclens.size() <= did) doclens.resize(did + 1);
	    doclens[did] = actual_doclen;
	}

	if (num_termlists != doccount && doccount != Xapian::doccount(-1)) {
	    if (out)
		*out << "Number of termlists (" << num_termlists
		     << ") != get_doccount() (" << doccount << ")" << endl;
	    ++errors;
	}

	// chert doesn't store a valueslots used entry if there are no terms,
	// so we can only check there aren't more such entries than documents.
	if (num_slotsused_entries > doccount &&
	    doccount != Xapian::doccount(-1)) {
	    if (out)
		*out << "More slots-used entries (" << num_slotsused_entries
		     << ") then documents (" << doccount << ")" << endl;
	    ++errors;
	}
    } else if (strcmp(tablename, "position") == 0) {
	// Now check the contents of the position table.
	for ( ; !cursor->after_end(); cursor->next()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!C_unpack_uint_preserving_sort(&pos, end, &did)) {
		if (out)
		    *out << "Error unpacking docid from key" << endl;
		++errors;
		continue;
	    }

	    if (did > db_last_docid) {
		if (out)
		    *out << "document id " << did << " in position table "
			    "is larger than get_last_docid() "
			 << db_last_docid << endl;
		++errors;
	    } else if (!doclens.empty()) {
		// In chert, a document without terms doesn't get a
		// termlist entry, so we can't tell the difference
		// easily.
		if (did >= doclens.size() || doclens[did] == 0) {
		    if (out)
			*out << "Position list entry for document " << did
			     << " which doesn't exist or has no terms" << endl;
		    ++errors;
		}
	    }

	    if (pos == end) {
		if (out)
		    *out << "No termname in key" << endl;
		++errors;
		continue;
	    }

	    cursor->read_tag();

	    const string & data = cursor->current_tag;
	    pos = data.data();
	    end = pos + data.size();

	    Xapian::termpos pos_last;
	    if (!unpack_uint(&pos, end, &pos_last)) {
		if (out)
		    *out << tablename << " table: Position list data corrupt"
			 << endl;
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
		rd.decode_interpolative(0, pos_size - 1, pos_first, pos_last);
		Xapian::termpos p = rd.decode_interpolative_next();
		bool ok = true;
		while (p != pos_last) {
		    Xapian::termpos pos_prev = p;
		    p = rd.decode_interpolative_next();
		    if (p <= pos_prev) {
			if (out)
			    *out << tablename << " table: Positions not "
				    "strictly monotonically increasing" << endl;
			++errors;
			ok = false;
			break;
		    }
		}
		if (ok && !rd.check_all_gone()) {
		    if (out)
			*out << tablename << " table: Junk after position data"
			     << endl;
		    ++errors ;
		}
	    }
	}
    } else {
	if (out)
	    *out << tablename << " table: Don't know how to check structure\n"
		 << endl;
	return errors;
    }

    if (out) {
	if (!errors)
	    *out << tablename << " table structure checked OK\n";
	else
	    *out << tablename << " table errors found: " << errors << "\n";
	*out << endl;
    }

    return errors;
}
