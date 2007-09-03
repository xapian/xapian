/* xapian-check.cc: use Btree::check to check consistency of a flint database
 * or btree.  Also check the structures inside the tables.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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
#include "flint_check.h"
#include "flint_cursor.h"
#include "flint_table.h"
#include "flint_types.h"
#include "flint_utils.h"
#include "stringutils.h"
#include "utils.h"

#include <xapian.h>

using namespace std;

#define PROG_NAME "xapian-check"
#define PROG_DESC "Check the consistency of a flint database or table"

// FIXME: We don't currently cross-check wdf between postlist and termlist.
// It's hard to see how to efficiently.  We do cross-check doclens, but that
// "only" requires (4 * last_docid()) bytes.

static void show_usage() {
    cout << "Usage: "PROG_NAME" <flint directory>|<path to btree and prefix> [[t][f][b][v][+]]\n\n"
"The btree(s) is/are always checked - control the output verbosity with:\n"
" t = short tree printing\n"
" f = full tree printing\n"
" b = show bitmap\n"
" v = show stats about B-tree (default)\n"
" + = same as tbv\n"
" e.g. "PROG_NAME" /var/lib/xapian/data/default\n"
"      "PROG_NAME" /var/lib/xapian/data/default/postlist fbv" << endl;
}

static size_t check_table(string table, int opts);

static vector<Xapian::termcount> doclens;

static const unsigned char flstab[256] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

// Highly optimised fls() implementation.
inline int my_fls(unsigned mask)
{
    int result = 0;
    if (mask >= 0x10000u) {
	mask >>= 16;
	result = 16;
    }
    if (mask >= 0x100u) {
	mask >>= 8;
	result += 8;
    }
    return result + flstab[mask];
}

class BitReader {
    private:
	string buf;
	size_t idx;
	int n_bits;
	unsigned int acc;
    public:
	BitReader(const string &buf_) : buf(buf_), idx(0), n_bits(0), acc(0) { }
	Xapian::termpos decode(Xapian::termpos outof) {
	    size_t bits = my_fls(outof - 1);
	    const size_t spare = (1 << bits) - outof;
	    const size_t mid_start = (outof - spare) / 2;
	    Xapian::termpos p;
	    if (spare) {
		p = read_bits(bits - 1);
		if (p < mid_start) {
		    if (read_bits(1)) p += mid_start + spare;
		}
	    } else {
		p = read_bits(bits);
	    }
	    Assert(p < outof);
	    return p;
	}
	unsigned int read_bits(int count) {
	    unsigned int result;
	    if (count > 25) {
		// If we need more than 25 bits, read in two goes to ensure
		// that we don't overflow acc.  This is a little more
		// conservative than it needs to be, but such large values will
		// inevitably be rare (because you can't fit very many of them
		// into 2^32!)
		Assert(count <= 32);
		result = read_bits(16);
		return result | (read_bits(count - 16) << 16);
	    }
	    while (n_bits < count) {
		Assert(idx < buf.size());
		acc |= static_cast<unsigned char>(buf[idx++]) << n_bits;
		n_bits += 8;
	    }
	    result = acc & ((1u << count) - 1);
	    acc >>= count;
	    n_bits -= count;
	    return result;
	}
	// Check all the data has been read.  Because it'll be zero padded
	// to fill a byte, the best we can actually do is check that
	// there's less than a byte left and that all remaining bits are
	// zero.
	bool check_all_gone() const {
	    return (idx == buf.size() && n_bits < 7 && acc == 0);
	}
	void decode_interpolative(vector<Xapian::termpos> & pos, int j, int k);
};

void
BitReader::decode_interpolative(vector<Xapian::termpos> & pos, int j, int k)
{
    while (j + 1 < k) {
	const size_t mid = (j + k) / 2;
	// Decode one out of (pos[k] - pos[j] + 1) values
	// (less some at either end because we must be able to fit
	// all the intervening pos in)
	const size_t outof = pos[k] - pos[j] + j - k + 1;
	pos[mid] = decode(outof) + (pos[j] + mid - j);
	decode_interpolative(pos, j, mid);
	j = mid;
    }
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
	size_t errors = 0;
	struct stat sb;
	string meta_file(argv[1]);
	meta_file += "/iamflint";
	if (stat(meta_file.c_str(), &sb) == 0) {
	    // Check a whole flint database directory.
	    try {
		Xapian::Database db = Xapian::Flint::open(argv[1]);
		doclens.reserve(db.get_lastdocid());
	    } catch (const Xapian::Error & e) {
		// Ignore so we can check a database too broken to open.
		cout << "Database couldn't be opened for reading: "
		     << e.get_description()
		     << "\nContinuing check anyway" << endl;
		++errors;
	    }
	    // Assume it's a flint directory and try to check all the btrees
	    // Note: it's important to check termlist before postlist so
	    // that we can cross-check the document lengths.
	    const char * tables[] = {
		"record", "termlist", "postlist", "position", "value",
		"spelling", "synonym"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(argv[1]);
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
		errors += check_table(table, opts);
	    }
	} else {
	    // Just check a single Btree.  If it ends with "." or ".DB"
	    // already, trim that so the user can do xapian-check on
	    // "foo", "foo.", or "foo.DB".
	    string table_name = argv[1];
	    if (endswith(table_name, '.'))
		table_name.resize(table_name.size() - 1);
	    else if (endswith(table_name, ".DB"))
		table_name.resize(table_name.size() - 3);

	    errors = check_table(table_name, opts);
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
check_table(string filename, int opts)
{
    size_t p = filename.find_last_of('/');
#ifdef __WIN32__
    if (p == string::npos) p = 0;
    p = filename.find_last_of('\\', p);
#endif
    if (p == string::npos) p = 0; else ++p;
    string tablename;
    while (p != filename.size()) {
	tablename += tolower(static_cast<unsigned char>(filename[p++]));
    }

    filename += '.';

    // Check the btree structure.
    BtreeCheck::check(filename, opts);

    // Now check the flint structures inside the btree.
    FlintTable table(filename, true);
    table.open();
    AutoPtr<FlintCursor> cursor(table.cursor_get());

    size_t errors = 0;

    cursor->find_entry("");
    cursor->next(); // Skip the empty entry.

    if (tablename == "postlist") {
	// Now check the structure of each postlist in the table.
	string current_term;
	Xapian::docid lastdid = 0;
	Xapian::termcount termfreq = 0, collfreq = 0;
	Xapian::termcount tf = 0, cf = 0;

	// The first key/tag pair should be the METAINFO.
	if (!cursor->after_end()) {
	    if (cursor->current_key != string("", 1)) {
		cout << "METAINFO key missing from postlist table" << endl;
		return errors + 1;
	    }
	    cursor->read_tag();
	    // Check format of the METAINFO key.
	    Xapian::docid did;
	    flint_totlen_t totlen;
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
    } else if (tablename == "termlist") {
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
    } else if (tablename == "position") {
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
		continue;
	    }
	    if (pos == end) {
		// Special case for single entry position list.
	    } else {
		BitReader rd(data);
		// Skip the header we just read.
		(void)rd.read_bits(8 * (pos - data.data()));
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
		    Xapian::termpos pos = *current_pos++;
		    if (pos <= lastpos) {
			cout << tablename << " table: Positions not strictly monotonically increasing" << endl;
			++errors;
			continue;
		    }
		    lastpos = pos;
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
