/** @file xapian-inspect-honey.cc
 * @brief Inspect the contents of a honey table for development or debugging.
 */
/* Copyright (C) 2007,2008,2009,2010,2011,2012,2017,2018 Olly Betts
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

#include <ios>
#include <iostream>
#include <string>
#include <cstdio> // For sprintf().

#include "honey_cursor.h"
#include "honey_defs.h"
#include "honey_table.h"
#include "honey_version.h"
#include "filetests.h"
#include "stringutils.h"

#include <xapian.h>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "xapian-inspect-honey"
#define PROG_DESC "Inspect a honey table for development or debugging"

#define OPT_HELP 1
#define OPT_VERSION 2

static bool keys = true, tags = true;

static void show_usage() {
    cout << "Usage: " PROG_NAME " [OPTIONS] TABLE\n"
	    "       " PROG_NAME " [OPTIONS] -t TABLE DB\n\n"
"Options:\n"
"  -t, --table=TABLE  which table to inspect\n"
"  --help             display this help and exit\n"
"  --version          output version information and exit" << endl;
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

// Reverse display_nicely() encoding.
static string
unescape(const string& s)
{
    auto bslash = s.find('\\');
    if (bslash == string::npos)
	return s;
    string r(s, 0, bslash);
    for (auto i = s.begin() + bslash; i != s.end(); ++i) {
	char ch = *i;
	if (ch == '\\') {
	    if (++i == s.end())
		goto bad_escaping;
	    ch = *i;
	    switch (ch) {
		case '\\':
		    break;
		case '0':
		    // \0 is not output by display_nicely(), but would
		    // reasonably be expected to work.
		    ch = '\0';
		    break;
		case 'n':
		    ch = '\n';
		    break;
		case 'r':
		    ch = '\r';
		    break;
		case 't':
		    ch = '\t';
		    break;
		case 'x': {
		    if (++i == s.end())
			goto bad_escaping;
		    char ch1 = *i;
		    if (++i == s.end())
			goto bad_escaping;
		    char ch2 = *i;
		    if (!C_isxdigit(ch1) || !C_isxdigit(ch2))
			goto bad_escaping;
		    ch = hex_digit(ch1) << 4 | hex_digit(ch2);
		    break;
		}
		default:
		    goto bad_escaping;
	    }
	}
	r += ch;
    }
    return r;

bad_escaping:
    cout << "Bad escaping in specified key value, assuming literal"
	 << endl;
    return s;
}

static void
show_help()
{
    cout << "Commands:\n"
	    "next   : Next entry (alias 'n' or '')\n"
	    "prev   : Previous entry (alias 'p')\n"
	    "first  : First entry (alias 'f')\n"
	    "last   : Last entry (alias 'l')\n"
	    "goto K : Goto first entry with key >= K (alias 'g')\n"
	    "until K: Display entries until key >= K (alias 'u')\n"
	    "until  : Display entries until end (alias 'u')\n"
	    "open T : Open table T instead (alias 'o') - e.g. open postlist\n"
	    "keys   : Toggle showing keys (default: true) (alias 'k')\n"
	    "tags   : Toggle showing tags (default: true) (alias 't')\n"
	    "help   : Show this (alias 'h' or '?')\n"
	    "quit   : Quit this utility (alias 'q')" << endl;
}

static void
show_entry(HoneyCursor & cursor)
{
    if (cursor.after_end()) {
	cout << "After end" << endl;
	return;
    }
    if (cursor.current_key.empty()) {
	cout << "Before start" << endl;
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
do_until(HoneyCursor & cursor, const string & target)
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

    size_t count = 0;
    while (cursor.next()) {
	++count;
	show_entry(cursor);

	if (target.empty())
	    continue;

	int cmp = target.compare(cursor.current_key);
	if (cmp < 0) {
	    cout << "No exact match, stopping at entry after, "
		    "having advanced by " << count << " entries." << endl;
	    return;
	}
	if (cmp == 0) {
	    cout << "Advanced by " << count << " entries." << endl;
	    return;
	}
    }

    cout << "Reached end, having advanced by " << count << " entries." << endl;
}

int
main(int argc, char **argv)
{
    static const struct option long_opts[] = {
	{"table",	required_argument, 0, 't'},
	{"help",	no_argument, 0, OPT_HELP},
	{"version",	no_argument, 0, OPT_VERSION},
	{NULL,		0, 0, 0}
    };

    string table_name;

    int c;
    while ((c = gnu_getopt_long(argc, argv, "t:", long_opts, 0)) != -1) {
	switch (c) {
	    case 't':
		table_name = optarg;
		break;
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

    // Path to the DB to inspect (possibly with a table name appended).
    string db_path(argv[optind]);
    bool arg_is_directory = dir_exists(db_path);
    if (arg_is_directory && table_name.empty()) {
	cerr << argv[0]
	     << ": You need to specify a table name to inspect with --table."
	     << endl;
	exit(1);
    }
    int single_file_fd = -1;
    if (table_name.empty()) {
	// db_path should be a path to a table, possibly without the extension
	// or with just a trailing '.' (supported mostly for historical
	// reasons).  First normalise away any extension or trailing '.'.
	if (endswith(db_path, "." HONEY_TABLE_EXTENSION)) {
	    db_path.resize(db_path.size() -
			   CONST_STRLEN(HONEY_TABLE_EXTENSION) - 1);
	} else if (endswith(db_path, '.')) {
	    db_path.resize(db_path.size() - 1);
	}
	size_t slash = db_path.find_last_of(DIR_SEPS);
	// If slash is std::string::npos, this assigns the whole of db_path to
	// table_name, which is what we want.
	table_name.assign(db_path, slash + 1, string::npos);
	if (slash != string::npos) {
	    db_path.resize(slash);
	} else {
	    db_path.resize(0);
	}
    } else if (!arg_is_directory) {
	single_file_fd = open(db_path.c_str(), O_RDONLY | O_BINARY);
	if (single_file_fd < 0) {
	    cerr << argv[0] << ": Couldn't open file '" << db_path << "'"
		 << endl;
	    exit(1);
	}
    }

    HoneyVersion* version_file_ptr;
    if (single_file_fd < 0) {
	version_file_ptr = new HoneyVersion(db_path);
    } else {
	version_file_ptr = new HoneyVersion(single_file_fd);
    }
    HoneyVersion& version_file = *version_file_ptr;

    version_file.read();
    Xapian::rev rev = version_file.get_revision();

    show_help();
    cout << endl;

open_different_table:
    try {
	Honey::table_type table_code;
	if (table_name == "docdata") {
	    table_code = Honey::DOCDATA;
	} else if (table_name == "spelling") {
	    table_code = Honey::SPELLING;
	} else if (table_name == "synonym") {
	    table_code = Honey::SYNONYM;
	} else if (table_name == "termlist") {
	    table_code = Honey::TERMLIST;
	} else if (table_name == "position") {
	    table_code = Honey::POSITION;
	} else if (table_name == "postlist") {
	    table_code = Honey::POSTLIST;
	} else {
	    cerr << "Unknown table: '" << table_name << "'" << endl;
	    exit(1);
	}

	HoneyTable* table_ptr;
	if (single_file_fd < 0) {
	    string table_path = db_path;
	    table_path += '/';
	    table_path += table_name;
	    table_path += '.';
	    table_ptr = new HoneyTable("", table_path, true);
	} else {
	    auto offset = version_file.get_offset();
	    table_ptr = new HoneyTable("", single_file_fd, offset, true);
	}
	HoneyTable& table = *table_ptr;

	table.open(0, version_file.get_root(table_code), rev);
	HoneyCursor cursor(&table);
	cursor.rewind();
	if (!cursor.next()) {
	    cout << "No entries!" << endl;
	    exit(0);
	}

	cout << "Table has " << table.get_entry_count() << " entries" << endl;

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
		if (cursor.after_end()) {
		    cout << "At end already." << endl;
		    goto wait_for_input;
		}
		(void)cursor.next();
		continue;
	    } else if (input == "p" || input == "prev") {
		if (!cursor.prev()) {
		    cout << "Before start already." << endl;
		    goto wait_for_input;
		}
		continue;
	    } else if (startswith(input, "u ")) {
		do_until(cursor, unescape(input.substr(2)));
		goto wait_for_input;
	    } else if (startswith(input, "until ")) {
		do_until(cursor, unescape(input.substr(6)));
		goto wait_for_input;
	    } else if (input == "u" || input == "until") {
		do_until(cursor, string());
		goto wait_for_input;
	    } else if (input == "f" || input == "first") {
		cursor.rewind();
		cursor.next();
		continue;
	    } else if (input == "l" || input == "last") {
		cursor.to_end();
		cursor.prev();
		continue;
	    } else if (startswith(input, "g ")) {
		if (!cursor.find_entry_ge(unescape(input.substr(2)))) {
		    cout << "No exact match, going to entry after." << endl;
		}
		continue;
	    } else if (startswith(input, "goto ")) {
		if (!cursor.find_entry_ge(unescape(input.substr(5)))) {
		    cout << "No exact match, going to entry after." << endl;
		}
		continue;
	    } else if (startswith(input, "o ") || startswith(input, "open ")) {
		size_t trim = (input[1] == ' ' ? 2 : 5);
		table_name.assign(input, trim, string::npos);
		if (endswith(table_name, "." HONEY_TABLE_EXTENSION))
		    table_name.resize(table_name.size() -
				      CONST_STRLEN(HONEY_TABLE_EXTENSION) - 1);
		else if (endswith(table_name, '.'))
		    table_name.resize(table_name.size() - 1);
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
