/* quartzcheck.cc: use Btree::check to check consistency of a quartz database
 * or btree
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

using namespace std;

#include "autoptr.h"
#include "btreecheck.h"
#include "quartz_table.h"
#include "quartz_types.h"
#include "quartz_utils.h"

static void check_table(const char *table, int opts);

int
main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
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
	struct stat sb;
	if (stat(argv[1], &sb) == 0 && S_ISDIR(sb.st_mode)) {
	    // Assume it's a quartz directory and try to check all the btrees
	    const char * tables[] = {
		"record", "postlist", "termlist", "position", "value"
	    };
	    for (const char **t = tables;
		 t != tables + sizeof(tables)/sizeof(tables[0]); ++t) {
		string table(argv[1]);
		table += '/';
		table += *t;
		table += '_';
		cout << *t << ":\n";
		check_table(table.c_str(), opts);
	    }
	} else {
	    check_table(argv[1], opts);
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_msg() << endl;
	exit(1);
    }
}

static void check_table(const char *filename, int opts) {
    // Check the btree structure.
    BtreeCheck::check(filename, opts);

    // Now check the quartz structures inside the btree.
    Btree table(filename, true);
    table.open();
    AutoPtr<QuartzCursor> cursor(table.cursor_get());

    cursor->find_entry("");
    cursor->next(); // Skip the empty entry.
    switch (filename[strlen(filename) - 6]) { // 5th from last char is unique!
	case 'e': // rEcord_
	    break;
	case 't': { // posTlist_
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
		}
		if (current_term.empty()) {
		    current_term = term;
		    tf = cf = 0;
		    if (pos != end) {
			cout << "Extra bytes after key for first chunk of "
			    "posting list for term `" << term << "'" << endl;
		    }
		    // Unpack extra header from first chunk.
		    pos = cursor->current_tag.data();
		    end = pos + cursor->current_tag.size();
		    if (!unpack_uint(&pos, end, &termfreq)) {
			cout << "Failed to unpack termfreq for term `" << term
			     << "'" << endl;
		    }
		    if (!unpack_uint(&pos, end, &collfreq)) {
			cout << "Failed to unpack collfreq for term `" << term
			     << "'" << endl;
		    }
		    if (!unpack_uint(&pos, end, &did)) {
			cout << "Failed to unpack firstdid for term `" << term
			     << "'" << endl;
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
			current_term = term;
		    }
		    if (pos != end) {
			if (!unpack_uint_preserving_sort(&pos, end, &did)) {
			    cout << "Failed to unpack did from key" << endl;
			}
			if (did <= lastdid) {
			    cout << "First did in this chunk is <= last in "
				"prev chunk" << endl;
			}
		    }
		    pos = cursor->current_tag.data();
		    end = pos + cursor->current_tag.size();
		}

		bool is_last_chunk;
		if (!unpack_bool(&pos, end, &is_last_chunk)) {
		    cout << "Failed to unpack last chunk flag" << endl;
		}
		// Read what the final document ID in this chunk is.
		if (!unpack_uint(&pos, end, &lastdid)) {
		    cout << "Failed to unpack increase to last" << endl;
		}
		++lastdid;
		lastdid += did;
		while (true) {
		    Xapian::termcount wdf, doclen;
		    if (!unpack_uint(&pos, end, &wdf)) {
			cout << "Failed to unpack wdf" << endl;
		    }
		    if (!unpack_uint(&pos, end, &doclen)) {
			cout << "Failed to unpack doc length" << endl;
		    }
		    ++tf;
		    cf += wdf;
		    if (pos == end) break;
		    Xapian::docid inc;
		    if (!unpack_uint(&pos, end, &inc)) {
			cout << "Failed to unpack docid increase" << endl;
		    }
		    ++inc;
		    did += inc;
		    if (did > lastdid) {
			cout << "docid " << did << " > last docid " << lastdid
			     << endl;
		    }
		}
		if (is_last_chunk) {
		    if (tf != termfreq) {
			cout << "termfreq " << termfreq << " != # of entries "
			     << tf << endl;
		    }
		    if (cf != collfreq) {
			cout << "collfreq " << collfreq << " != sum wdf " << cf
			     << endl;
		    }
		    if (did != lastdid) {
			cout << "lastdid " << lastdid << " != last did " << did
			     << endl;
		    }
		    current_term = "";
		}

		cursor->next();
	    }
	    if (!current_term.empty()) {
		cout << "Last term `" << current_term << "' has no last chunk"
		     << endl;
	    }
	    cout << "Postlist structure checked OK" << endl;
	    break;
	}
	case 'm': // terMlist_
	    break;
	case 'i': // posItion_
	    break;
	case 'v': // Value_
	    break;
    }
}
