/* quartzdump.cc - dump a quartz table for debugging purposes
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2006 Olly Betts
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
#include <xapian/error.h>
#include "btree.h"
#include "bcursor.h"
#include "quartz_types.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctype.h>

#include "autoptr.h"
#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "quartzdump"
#define PROG_DESC "Dump a quartz table for debugging purposes"

#define OPT_HELP 1
#define OPT_VERSION 2

static void show_usage() {
    cout << "Usage: "PROG_NAME" [OPTIONS] TABLE...\n\n"
"  -r, --revision=REVNO   Revision number to open (default: highest)\n"
"  -s, --start-key=START  Start at key START\n"
"  -e, --end-key=END      End at key END\n"
"  --help                 display this help and exit\n"
"  --version              output version information and exit" << endl;
}

static string hex_encode(const string & input) {
    const char * table = "0123456789abcdef";
    string result;
    for (string::const_iterator i = input.begin(); i != input.end(); ++i) {
	unsigned char val = *i;
	if (isprint(val)) {
	    if (val == ' ' || val == '\\') result += '\\';
	    result += val;
	} else {
	    result += "\\x";
	    result += table[val >> 4];
	    result += table[val & 0x0f];
	}
    }

    return result;
}

int
main(int argc, char *argv[])
{
    vector<string> tables;
    quartz_revision_number_t revnum = 0;
    bool use_revno = false;
    string startkey;
    string endkey;
    bool use_endkey = false;

    bool syntax_error = false;

    const struct option long_opts[] = {
	{"revision",	required_argument, 0, 'r'},
	{"start-key",	required_argument, 0, 's'},
	{"end-key",	required_argument, 0, 'e'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    int c;
    while ((c = gnu_getopt_long(argc, argv, "r:s:e:", long_opts, 0)) != EOF) {
        switch (c) {
            case 'r':
		revnum = atoi(optarg);
		use_revno = true;
                break;
            case 's':
		startkey = optarg;
                break;
	    case 'e':
		endkey = optarg;
		use_endkey = true;
		break;
	    case OPT_HELP:
		cout << PROG_NAME" - "PROG_DESC"\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME" - "PACKAGE_STRING << endl;
		exit(0);
            default:
                syntax_error = true;
		break;
        }
    }

    while (argv[optind]) {
	tables.push_back(argv[optind++]);
    }

    if (syntax_error || tables.empty()) {
	show_usage();
	exit(1);
    }

    vector<string>::const_iterator i;
    for (i = tables.begin(); i != tables.end(); i++) {
	try {
	    Btree table(*i, true);
	    if (use_revno) {
		table.open(revnum);
	    } else {
		table.open();
	    }

	    quartz_revision_number_t openrev = table.get_open_revision_number();
	    quartz_revision_number_t latestrev = table.get_latest_revision_number();

	    quartz_tablesize_t entrycount = table.get_entry_count();

	    cout << "Table `" << *i << "' at revision " << openrev;
	    if (openrev != latestrev)
		cout << " (Newest revision is " << latestrev << ")";
	    cout << endl;

	    cout << "table contains " << entrycount <<
		    (entrycount == 1 ? " entry" : " entries") << endl;

	    AutoPtr<Bcursor> cursor(table.cursor_get());

	    string key = startkey;
	    cursor->find_entry(key);

	    if (startkey.empty() || cursor->current_key < startkey) {
		cursor->next();
	    }

	    while (!cursor->after_end()) {
		if (use_endkey && cursor->current_key > endkey) break;
		cursor->read_tag();
		cout << hex_encode(cursor->current_key) << " -> "
		     << hex_encode(cursor->current_tag) << "\n";
		cursor->next();
	    }
	} catch (const Xapian::Error &e) {
	    cerr << "Error: " << e.get_msg() << endl;
	    return 1;
	}
    }
}
