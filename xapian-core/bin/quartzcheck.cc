/* quartzcheck.cc: use Btree::check to check consistency of a quartz database
 * or btree.  Also check the structures inside the tables.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005 Olly Betts
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
#include <iostream>

#include "autoptr.h"
#include "btreecheck.h"
#include "bcursor.h"
#include "quartz_types.h"
#include "quartz_utils.h"

#include <xapian.h>

using namespace std;

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

static size_t check_table(const char *table, int opts);

static vector<Xapian::termcount> doclens;

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 3 || strcmp(argv[1], "--help") == 0) {
	cout << "usage: " << argv[0]
	     << " <path to btree and prefix>|<quartz directory> [[t][f][b][v][+]]\n"
	        "The btree(s) is/are always checked - control the output verbosity with:\n"
		" t = short tree printing\n"
		" f = full tree printing\n"
		" b = show bitmap\n"
		" v = show stats about B-tree (default)\n"
		" + = same as tbv\n"
		" e.g. " << argv[0]
	     << " /var/lib/xapian/data/default"
		"      " << argv[0]
	     << " /var/lib/xapian/data/default/postlist_ fbv"
	     << endl;
	exit(1);
    }

    int opts = 0;
    const char * opt_string = argv[2];
    if (!opt_string) opt_string = "v";
    for (const char *p = opt_string; *p; ++p) {
	switch (*p) {
	    case 't': opts |= OPT_SHORT_TREE; break;
	    case 'f': opts |= OPT_FULL_TREE; break;
	    case 'b': opts |= OPT_SHOW_BITMAP; break;
	    case 'v': opts |= OPT_SHOW_STATS; break;
	    case '+':
		opts |= OPT_SHORT_TREE | OPT_SHOW_BITMAP | OPT_SHOW_STATS;
		break;
	    case '?':
		cerr << "use t,f,b,v or + in the option string\n";
		exit(0);
	    default:
		cerr << "option " << opt_string << " unknown\n";
		exit(1);
	}
    }

    try {
	size_t errors = 0;
	struct stat sb;
	if (stat(argv[1], &sb) == 0 && S_ISDIR(sb.st_mode)) {
	    try {
		Xapian::Database db(argv[1]);
		doclens.reserve(db.get_lastdocid());
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
	    }
	    // Assume it's a quartz directory and try to check all the btrees
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position", "value"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(argv[1]);
		table += '/';
		table += *t;
		table += '_';
		cout << *t << ":\n";
		errors += check_table(table.c_str(), opts);
	    }
	} else {
	    errors = check_table(argv[1], opts);
	}
	if (errors > 0) {
	    cout << "Total errors found: " << errors << endl;
	    exit(1);
	}
	cout << "No errors found" << endl;
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}

static size_t
check_table(const char *filename, int opts)
{
    // Check the btree structure.
    BtreeCheck::check(filename, opts);

    // Now check the quartz structures inside the btree.
    Btree table(filename, true);
    table.open();
    AutoPtr<Bcursor> cursor(table.cursor_get());

    size_t errors = 0;

    cursor->find_entry("");
    cursor->next(); // Skip the empty entry.

    const char *p = strrchr(filename, '/');
    if (!p) p = strrchr(filename, '\\');
    if (p) ++p; else p = filename;
    string tablename;
    while (unsigned char ch = *p++) {
	if (ch == '_' && *p == '\0') break;
	tablename += tolower(ch);
    }

    if (tablename == "postlist") {
	// Now check the structure of each postlist in the table.
	string current_term;
	Xapian::docid lastdid = 0;
	Xapian::termcount termfreq = 0, collfreq = 0;
	Xapian::termcount tf = 0, cf = 0;
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

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
	    if (current_term.empty()) {
		current_term = term;
		tf = cf = 0;
		if (pos != end) {
		    cout << "Extra bytes after key for first chunk of "
			"posting list for term `" << term << "'" << endl;
		    ++errors;
		    continue;
		}
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
		} else {
		    ++did;
		}
	    } else {
		if (term != current_term) {
		    if (pos == end) {
			cout << "No last chunk for term `" << term << "'"
			     << endl;
		    } else {
			cout << "Mismatch in follow-on chunk in posting "
			    "list for term `" << current_term << "' (got `"
			    << term << "')" << endl;
		    }
		    ++errors;
		    current_term = term;
		}
		if (pos != end) {
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
	    ++lastdid;
	    lastdid += did;
	    bool bad = false;
	    while (true) {
		Xapian::termcount wdf, doclen;
		if (!unpack_uint(&pos, end, &wdf)) {
		    cout << "Failed to unpack wdf" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		if (!unpack_uint(&pos, end, &doclen)) {
		    cout << "Failed to unpack doc length" << endl;
		    ++errors;
		    bad = true;
		    break;
		}
		++tf;
		cf += wdf;

		if (!doclens.empty()) {
		    if (did >= doclens.size()) {
			cout << "document id " << did << " is larger than any in the termlist table!" << endl;
		    } else if (doclens[did] != doclen) {
			cout << "doclen " << doclen << " doesn't match " << doclens[did] << " in the termlist table" << endl;
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
	    if (bad) continue;
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
		current_term = "";
	    }

	    cursor->next();
	}
	if (!current_term.empty()) {
	    cout << "Last term `" << current_term << "' has no last chunk"
		 << endl;
	    ++errors;
	}
    } else if (tablename == "record") {
	// Now check the contents of the record table.  Apart from the
	// METAINFO key, any data is valid as the tag so we don't check
	// those.
	if (!cursor->after_end()) {
	    if (cursor->current_key != string("", 1)) {
		cout << "METAINFO key missing from record table" << endl;
		return errors + 1;
	    } else {
		cursor->read_tag();
		// Check format of the METAINFO key.
		Xapian::docid did;
		quartz_totlen_t totlen;
		const char * data = cursor->current_tag.data();
		const char * end = data + cursor->current_tag.size();
		if (!unpack_uint(&data, end, &did)) {
		    cout << "Record containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		if (!unpack_uint_last(&data, end, &totlen)) {
		    cout << "Record containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		if (data != end) {
		    cout << "Record containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		cursor->next();
	    }
	}
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_last(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->next();
	}
    } else if (tablename == "termlist") {
	// Now check the contents of the termlist table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_last(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

	    Xapian::termcount doclen, termlist_size;
	    bool has_termfreqs;

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

	    // Read has_termfreqs
	    if (!unpack_bool(&pos, end, &has_termfreqs)) {
		cout << "Unexpected end of data when reading termlist" << endl;
		++errors;
		continue;
	    }
	    if (has_termfreqs) {
		cout << "has_termfreqs is true, but Xapian never sets it!" << endl;
		++errors;
		continue;
	    }

	    Xapian::termcount actual_doclen = 0, actual_termlist_size = 0;
	    string current_tname;

	    bool bad = false;
	    while (pos != end) {
		Xapian::doccount current_wdf;
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
			    cout << "Unexpected end of data when reading termlist" << endl;
			} else {
			    cout << "Size of wdf out of range, in termlist" << endl;
			}
			++errors;
			bad = true;
			break;
		    }
		}

		// Don't bother with the (has_termfreqs == true) case since
		// we never generate that.

		++actual_termlist_size;
		actual_doclen += current_wdf;
	    }
	    if (bad) continue;

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

	    cursor->next();
	}
    } else if (tablename == "value") {
	// Now check the contents of the value table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_last(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

	    bool bad = false;

	    bool first = true;
	    Xapian::valueno last_value_no = 0;
	    while (pos && pos != end) {
		Xapian::valueno this_value_no;
		string this_value;

		if (!unpack_uint(&pos, end, &this_value_no)) {
		    if (pos == 0)
			cout << "Incomplete item in value table" << endl;
		    else
			cout << "Value number in value table is too large" << endl;
		    ++errors;
		    bad = true;
		    break;
		}

		if (!unpack_string(&pos, end, this_value)) {
		    if (pos == 0)
			cout << "Incomplete item in value table" << endl;
		    else
			cout << "Item in value table is too large" << endl;
		    ++errors;
		    bad = true;
		    break;
		}

		if (first) {
		    first = false;
		} else if (this_value_no <= last_value_no) {
		    cout << "Values not in sorted order - valueno " << last_value_no << " comes before valueno " << this_value_no << endl;
		    ++errors;
		}
		last_value_no = this_value_no;
	    }

	    cursor->next();
	}
    } else if (tablename == "position") {
	// Now check the contents of the position table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    }
	    if (pos == end) {
		cout << "No termname in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

	    Xapian::termcount number_of_entries;

	    // Read list length.
	    if (!unpack_uint(&pos, end, &number_of_entries)) {
		if (pos != 0) {
		    cout << "number_of_entries out of range" << endl;
		} else {
		    cout << "Unexpected end of data when reading number_of_entries" << endl;
		}
		++errors;
		continue;
	    }

	    Xapian::termcount actual_number_of_entries = 0;

	    bool bad = false;
	    while (pos != end) {
		Xapian::termpos pos_increment;
		if (!unpack_uint(&pos, end, &pos_increment)) {
		    if (pos != 0) {
			cout << "value out of range" << endl;
		    } else {
			cout << "Unexpected end of data when reading position increment" << endl;
		    }
		    ++errors;
		    bad = true;
		    break;
		}

		++actual_number_of_entries;
	    }
	    if (bad) continue;

	    if (number_of_entries != actual_number_of_entries) {
		cout << "number_of_entries != # of entries in positionlist" << endl;
		++errors;
	    }

	    cursor->next();
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
