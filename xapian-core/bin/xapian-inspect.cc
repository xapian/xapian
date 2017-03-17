/** @file xapian-inspect.cc
 * @brief Inspect the contents of a glass table for development or debugging.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012 Olly Betts
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

#include <iomanip>
#include <iostream>
#include <string>
#include <cstdio> // For sprintf().

#include "glass_cursor.h"
#include "glass_table.h"
#include "glass_version.h"
#include "filetests.h"
#include "stringutils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-inspect"
#define PROG_DESC "Inspect the contents of a glass table for development or debugging"

#define OPT_HELP 1
#define OPT_VERSION 2

static bool keys = true, tags = true;

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] TABLE\n\n"
"Options:\n"
"  --help           display this help and exit\n"
"  --version        output version information and exit" << endl;
}

static void
display_nicely(const string & data) {
    string::const_iterator i;
    for (i = data.begin(); i != data.end(); ++i) {
	unsigned char ch = *i;
	if (ch < 32 || ch >= 127) {
	    switch (ch) {
		case '\n': cout << "\\n"; break;
		case '\r': cout << "\\r"; break;
		case '\t': cout << "\\t"; break;
		default: {
		    char buf[20];
		    sprintf(buf, "\\x%02x", int(ch));
		    cout << buf;
		}
	    }
	} else if (ch == '\\') {
	    cout << "\\\\";
	} else {
	    cout << ch;
	}
    }
}

static void
show_help()
{
    cout << "Commands:\n"
	    "next   : Next entry (alias 'n' or '')\n"
	    "prev   : Previous entry (alias 'p')\n"
	    "goto K : Goto entry with key K (alias 'g')\n"
	    "until K: Display entries until key K (alias 'u')\n"
	    "open T : Open table T instead (alias 'o') - e.g. open postlist\n"
	    "keys   : Toggle showing keys (default: true) (alias 'k')\n"
	    "tags   : Toggle showing tags (default: true) (alias 't')\n"
	    "help   : Show this (alias 'h' or '?')\n"
	    "quit   : Quit this utility (alias 'q')" << endl;
}

static void
show_entry(GlassCursor & cursor)
{
    if (cursor.after_end()) {
	cout << "After end" << endl;
	return;
    }
    if (keys) {
	cout << "Key: ";
	display_nicely(cursor.current_key);
	cout << endl;
    }
    if (tags) {
	cout << "Tag: ";
	cursor.read_tag();
	display_nicely(cursor.current_tag);
	cout << endl;
    }
}

static void
do_until(GlassCursor & cursor, const string & target)
{
    if (cursor.after_end()) {
	cout << "At end already." << endl;
	return;
    }

    if (!target.empty()) {
	int cmp = target.compare(cursor.current_key);
	if (cmp <= 0) {
	    if (cmp)
		cout << "Already after specified key." << endl;
	    else
		cout << "Already at specified key." << endl;
	    return;
	}
    }

    while (cursor.next()) {
	int cmp = 1;
	if (!target.empty()) {
	    cmp = target.compare(cursor.current_key);
	    if (cmp < 0) {
		cout << "No exact match, stopping at entry before." << endl;
		cursor.find_entry_lt(cursor.current_key);
		return;
	    }
	}
	show_entry(cursor);
	if (cmp == 0) {
	    return;
	}
    }

    cout << "Reached end." << endl;
}

int
main(int argc, char **argv)
{
    const struct option long_opts[] = {
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    int c;
    while ((c = gnu_getopt_long(argc, argv, "", long_opts, 0)) != -1) {
	switch (c) {
	    case OPT_HELP:
		cout << PROG_NAME " - " PROG_DESC "\n\n";
		show_usage();
		exit(0);
	    case OPT_VERSION:
		cout << PROG_NAME " - " PACKAGE_STRING << endl;
		exit(0);
	    default:
		show_usage();
		exit(1);
	}
    }

    if (argc - optind != 1) {
	show_usage();
	exit(1);
    }

    // Path to the table to inspect.
    string table_name(argv[optind]);
    bool arg_is_directory = dir_exists(table_name);
    if (endswith(table_name, ".DB"))
	table_name.resize(table_name.size() - 2);
    else if (!endswith(table_name, '.'))
	table_name += '.';
    if (arg_is_directory && !file_exists(table_name + "DB")) {
	cerr << argv[0] << ": You need to specify a single Btree table, not a database directory." << endl;
	exit(1);
    }

    string db_dir;
    size_t slash = table_name.rfind('/');
    if (slash != string::npos) {
	db_dir.assign(table_name, 0, slash);
    } else {
	db_dir = ".";
    }
    GlassVersion version_file(db_dir);
    version_file.read();
    glass_revision_number_t rev = version_file.get_revision();

    show_help();
    cout << endl;

open_different_table:
    try {
	Glass::table_type table_code;
	if (endswith(table_name, "docdata.")) {
	    table_code = Glass::DOCDATA;
	} else if (endswith(table_name, "spelling.")) {
	    table_code = Glass::SPELLING;
	} else if (endswith(table_name, "synonym.")) {
	    table_code = Glass::SYNONYM;
	} else if (endswith(table_name, "termlist.")) {
	    table_code = Glass::TERMLIST;
	} else if (endswith(table_name, "position.")) {
	    table_code = Glass::POSITION;
	} else if (endswith(table_name, "postlist.")) {
	    table_code = Glass::POSTLIST;
	} else {
	    cout << "Unknown table." << endl;
	    exit(1);
	}

	GlassTable table("", table_name, true);
	table.open(0, version_file.get_root(table_code), rev);
	if (table.empty()) {
	    cout << "No entries!" << endl;
	    exit(0);
	}

	GlassCursor cursor(&table);
	cursor.find_entry(string());
	cursor.next();

	while (!cin.eof()) {
	    show_entry(cursor);
wait_for_input:
	    cout << "? " << flush;

	    string input;
	    getline(cin, input);
	    if (cin.eof()) break;

	    if (endswith(input, '\r'))
		input.resize(input.size() - 1);

	    if (input.empty() || input == "n" || input == "next") {
		if (cursor.after_end() || !cursor.next()) {
		    cout << "At end already." << endl;
		    goto wait_for_input;
		}
		continue;
	    } else if (input == "p" || input == "prev") {
		// If the cursor has fallen off the end, point it back at
		// the last entry.
		if (cursor.after_end()) cursor.find_entry(cursor.current_key);
		cursor.find_entry_lt(cursor.current_key);
		if (cursor.current_key.empty()) {
		    cout << "At start already." << endl;
		    goto wait_for_input;
		}
		continue;
	    } else if (startswith(input, "u ")) {
		do_until(cursor, input.substr(2));
		goto wait_for_input;
	    } else if (startswith(input, "until ")) {
		do_until(cursor, input.substr(6));
		goto wait_for_input;
	    } else if (input == "u" || input == "until") {
		do_until(cursor, string());
		goto wait_for_input;
	    } else if (startswith(input, "g ")) {
		if (!cursor.find_entry(input.substr(2))) {
		    cout << "No exact match, going to entry before." << endl;
		}
		continue;
	    } else if (startswith(input, "goto ")) {
		if (!cursor.find_entry(input.substr(5))) {
		    cout << "No exact match, going to entry before." << endl;
		}
		continue;
	    } else if (startswith(input, "o ")) {
		table_name = db_dir;
		if (!table_name.empty()) table_name += '/';
		table_name += input.substr(2);
		if (endswith(table_name, ".DB"))
		    table_name.resize(table_name.size() - 2);
		else if (!endswith(table_name, '.'))
		    table_name += '.';
		goto open_different_table;
	    } else if (startswith(input, "open ")) {
		table_name = db_dir;
		if (!table_name.empty()) table_name += '/';
		table_name += input.substr(5);
		if (endswith(table_name, ".DB"))
		    table_name.resize(table_name.size() - 2);
		else if (!endswith(table_name, '.'))
		    table_name += '.';
		goto open_different_table;
	    } else if (input == "t" || input == "tags") {
		tags = !tags;
		cout << "Showing tags: " << boolalpha << tags << endl;
	    } else if (input == "k" || input == "keys") {
		keys = !keys;
		cout << "Showing keys: " << boolalpha << keys << endl;
	    } else if (input == "q" || input == "quit") {
		break;
	    } else if (input == "h" || input == "help" || input == "?") {
		show_help();
		goto wait_for_input;
	    } else {
		cout << "Unknown command." << endl;
		goto wait_for_input;
	    }
	}
    } catch (const Xapian::Error &error) {
	cerr << argv[0] << ": " << error.get_description() << endl;
	exit(1);
    }
}
