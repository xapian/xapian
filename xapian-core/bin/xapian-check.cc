/* xapian-check.cc: Check consistency of a chert or flint database or btree.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007,2008 Olly Betts
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
#include <iostream>

#include "autoptr.h"

#include "bitstream.h"

#include "chert_check.h"
#include "chert_cursor.h"
#include "chert_table.h"
#include "chert_types.h"
#include "chert_utils.h"

#include "stringutils.h"
#include "utils.h"

#include "xapian-check-flint.h"

#include <xapian.h>

using namespace std;

#define PROG_NAME "xapian-check"
#define PROG_DESC "Check the consistency of a chert or flint database or table"

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

static void show_usage() {
    cout << "Usage: "PROG_NAME" <chert or flint directory>|<path to btree and prefix> [[t][f][b][v][+]]\n\n"
"The btree(s) is/are always checked - control the output verbosity with:\n"
" t = short tree printing\n"
" f = full tree printing\n"
" b = show bitmap\n"
" v = show stats about B-tree (default)\n"
" + = same as tbv\n"
" e.g. "PROG_NAME" /var/lib/xapian/data/default\n"
"      "PROG_NAME" /var/lib/xapian/data/default/postlist fbv" << endl;
}

static size_t check_chert_table(const char * tablename, string table, int opts,
				std::vector<Xapian::termcount> & doclens);

static inline bool
is_user_metadata_key(const string & key)
{
    return key.size() > 1 && key[0] == '\0' && key[1] == '\xc0';
}

int
main(int argc, char **argv)
{
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1], "--help") == 0) {
	    cout << PROG_NAME" - "PROG_DESC"\n\n";
	    show_usage();
	    exit(0);
	}
	if (strcmp(argv[1], "--version") == 0) {
	    cout << PROG_NAME" - "PACKAGE_STRING << endl;
	    exit(0);
	}
    }
    if (argc < 2 || argc > 3) {
	show_usage();
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
	    default:
		cerr << "option " << opt_string << " unknown\n";
		cerr << "use t,f,b,v and/or + in the option string\n";
		exit(1);
	}
    }

    try {
	vector<Xapian::termcount> doclens;
	size_t errors = 0;
	struct stat sb;
	string dir(argv[1]);
	if (stat((dir + "/iamflint").c_str(), &sb) == 0) {
	    // Check a whole flint database directory.
	    try {
		Xapian::Database db = Xapian::Flint::open(dir);
		doclens.reserve(db.get_lastdocid());
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    // This is a flint directory so try to check all the btrees.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position", "value",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(dir);
		table += '/';
		table += *t;
		cout << *t << ":\n";
		if (strcmp(*t, "position") == 0 ||
		    strcmp(*t, "value") == 0 ||
		    strcmp(*t, "spelling") == 0 ||
		    strcmp(*t, "synonym") == 0) {
		    // These are created lazily, so may not exist.
		    if (!file_exists(table + ".DB")) {
			cout << "Lazily created, and not yet used.\n" << endl;
			continue;
		    }
		}
		errors += check_flint_table(*t, table, opts, doclens);
	    }
	} else if (stat((dir + "/iamchert").c_str(), &sb) == 0) {
	    // Check a whole chert database directory.
	    try {
		Xapian::Database db = Xapian::Chert::open(dir);
		doclens.reserve(db.get_lastdocid());
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    // This is a chert directory so try to check all the btrees.
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position", "value",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(dir);
		table += '/';
		table += *t;
		cout << *t << ":\n";
		if (strcmp(*t, "position") == 0 ||
		    strcmp(*t, "value") == 0 ||
		    strcmp(*t, "spelling") == 0 ||
		    strcmp(*t, "synonym") == 0) {
		    // These are created lazily, so may not exist.
		    if (!file_exists(table + ".DB")) {
			cout << "Lazily created, and not yet used.\n" << endl;
			continue;
		    }
		}
		errors += check_chert_table(*t, table, opts, doclens);
	    }
	} else {
	    if (stat((dir + "/record_DB").c_str(), &sb) == 0) {
		// Quartz is no longer supported as of Xapian 1.1.0.
		cerr << argv[0] << ": '" << dir << "' is a quartz database.\n"
			"Support for quartz was dropped in Xapian 1.1.0" << endl;
		exit(1);
	    }
	    // Just check a single Btree.  If it ends with "." or ".DB"
	    // already, trim that so the user can do xapian-check on
	    // "foo", "foo.", or "foo.DB".
	    string filename = dir;
	    if (endswith(filename, '.'))
		filename.resize(filename.size() - 1);
	    else if (endswith(filename, ".DB"))
		filename.resize(filename.size() - 3);

	    size_t p = filename.find_last_of('/');
#if defined __WIN32__ || defined __EMX__
	    if (p == string::npos) p = 0;
	    p = filename.find_last_of('\\', p);
#endif
	    if (p == string::npos) p = 0; else ++p;

	    bool flint = !file_exists(filename.substr(0, p) + "iamchert");

	    string tablename;
	    while (p != filename.size()) {
		tablename += tolower(static_cast<unsigned char>(filename[p++]));
	    }

	    if (flint) {
		errors = check_flint_table(tablename.c_str(), filename, opts,
				 	   doclens);
	    } else {
		errors = check_chert_table(tablename.c_str(), filename, opts,
				 	   doclens);
	    }
	}
	if (errors > 0) {
	    cout << "Total errors found: " << errors << endl;
	    exit(1);
	}
	cout << "No errors found" << endl;
    } catch (const char *error) {
	cerr << argv[0] << ": " << error << endl;
	exit(1);
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    } catch (...) {
	cerr << argv[0] << ": Unknown exception" << endl;
	exit(1);
    }
}

static size_t
check_chert_table(const char * tablename, string filename, int opts,
		  vector<Xapian::termcount> & doclens)
{
    filename += '.';

    // Check the btree structure.
    ChertTableCheck::check(tablename, filename, opts);

    // Now check the chert structures inside the btree.
    ChertTable table(tablename, filename, true);
    table.open();
    AutoPtr<ChertCursor> cursor(table.cursor_get());

    size_t errors = 0;

    cursor->find_entry("");
    cursor->next(); // Skip the empty entry.

    if (strcmp(tablename, "postlist") == 0) {
	// Now check the structure of each postlist in the table.
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
		Xapian::docid did;
		chert_totlen_t totlen;
		const char * data = cursor->current_tag.data();
		const char * end = data + cursor->current_tag.size();
		if (!unpack_uint(&data, end, &did)) {
		    cout << "Tag containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		if (!unpack_uint_last(&data, end, &totlen)) {
		    cout << "Tag containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		if (data != end) {
		    cout << "Tag containing meta information is corrupt." << endl;
		    return errors + 1;
		}
		cursor->next();
	    }
	}

	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    if (is_user_metadata_key(key)) {
		// User metadata can be anything, so we can't do any particular
		// checks on it.
		cursor->next();
		continue;
	    }

	    if (!have_metainfo_key) {
		cout << "METAINFO key missing from postlist table" << endl;
		return errors + 1;
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
			cursor->next();
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
			cursor->next();
			continue;
		    }
		    pos += 2;
		    if (!unpack_uint(&pos, end, &did)) {
			cout << "Failed to unpack firstdid for doclen" << endl;
			++errors;
			cursor->next();
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
		    cursor->next();
		    continue;
		}
		// Read what the final document ID in this chunk is.
		if (!unpack_uint(&pos, end, &lastdid)) {
		    cout << "Failed to unpack increase to last" << endl;
		    ++errors;
		    cursor->next();
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
		if (bad) {
		    cursor->next();
		    continue;
		}
		if (is_last_chunk) {
		    if (did != lastdid) {
			cout << "lastdid " << lastdid << " != last did " << did
			     << endl;
			++errors;
		    }
		}

		cursor->next();
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
		cursor->next();
		continue;
	    }
	    if (current_term.empty()) {
		current_term = term;
		tf = cf = 0;
		if (pos != end) {
		    cout << "Extra bytes after key for first chunk of "
			"posting list for term `" << term << "'" << endl;
		    ++errors;
		    cursor->next();
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
		    cursor->next();
		    continue;
		}
		if (!unpack_uint(&pos, end, &collfreq)) {
		    cout << "Failed to unpack collfreq for term `" << term
			 << "'" << endl;
		    ++errors;
		    cursor->next();
		    continue;
		}
		if (!unpack_uint(&pos, end, &did)) {
		    cout << "Failed to unpack firstdid for term `" << term
			 << "'" << endl;
		    ++errors;
		    cursor->next();
		    continue;
		}
		++did;
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
			cursor->next();
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
		cursor->next();
		continue;
	    }
	    // Read what the final document ID in this chunk is.
	    if (!unpack_uint(&pos, end, &lastdid)) {
		cout << "Failed to unpack increase to last" << endl;
		++errors;
		cursor->next();
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
		cursor->next();
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
		current_term = "";
	    }

	    cursor->next();
	}
	if (!current_term.empty()) {
	    cout << "Last term `" << current_term << "' has no last chunk"
		 << endl;
	    ++errors;
	}
    } else if (strcmp(tablename, "record") == 0) {
	// Now check the contents of the record table.  Any data is valid as
	// the tag so we don't check the tags.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->next();
	}
    } else if (strcmp(tablename, "termlist") == 0) {
	// Now check the contents of the termlist table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

	    if (pos == end) {
		// Empty termlist.
		cursor->next();
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
		cursor->next();
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
		cursor->next();
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
		cursor->next();
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

	    cursor->next();
	}
    } else if (strcmp(tablename, "value") == 0) {
	// Now check the contents of the value table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    } else if (pos != end) {
		cout << "Extra junk in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    pos = cursor->current_tag.data();
	    end = pos + cursor->current_tag.size();

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
		    break;
		}

		if (!unpack_string(&pos, end, this_value)) {
		    if (pos == 0)
			cout << "Incomplete item in value table" << endl;
		    else
			cout << "Item in value table is too large" << endl;
		    ++errors;
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
    } else if (strcmp(tablename, "position") == 0) {
	// Now check the contents of the position table.
	while (!cursor->after_end()) {
	    string & key = cursor->current_key;

	    // Get docid from key.
	    const char * pos = key.data();
	    const char * end = pos + key.size();

	    Xapian::docid did;
	    if (!unpack_uint_preserving_sort(&pos, end, &did)) {
		cout << "Error unpacking docid from key" << endl;
		return errors + 1;
	    }
	    if (pos == end) {
		cout << "No termname in key" << endl;
		return errors + 1;
	    }

	    cursor->read_tag();

	    const string & data = cursor->current_tag;
	    pos = data.data();
	    end = pos + data.size();

	    Xapian::termpos pos_last;
	    if (!unpack_uint(&pos, end, &pos_last)) {
		cout << tablename << " table: Position list data corrupt" << endl;
		++errors;
		cursor->next();
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
