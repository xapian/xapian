/* quartzdump.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
#include "om/omerror.h"
#include "quartz_table.h"
#include "quartz_types.h"
#include <iostream>
#include <vector>
#include <string>
#include <ctype.h>

#include "getopt.h"

using namespace std;

static string hex_encode(const string & input) {
    const char * table = "0123456789abcdef";
    string result;
    string::const_iterator i;
    for (i = input.begin(); i != input.end(); i++) {
	unsigned char val = *i;
	if (isprint(val) && !isspace(val) && val != '\\') {
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
    const char *progname = argv[0];
 
    vector<string> tables;
    quartz_revision_number_t revnum = 0;
    bool use_revno = false;
    om_termname startterm;
    om_termname endterm;
    bool use_endterm = false;

    bool syntax_error = false;
    int c;
    while ((c = getopt(argc, argv, "r:s:e:")) != EOF) {
        switch (c) {
            case 'r':
		revnum = atoi(optarg);
		use_revno = true;
                break;
            case 's':
		startterm = optarg;
                break;
	    case 'e':
		endterm = optarg;
		use_endterm = true;
		break;
            default:
                syntax_error = true;
		break;
        }
    }

    while (argv[optind]) {
	tables.push_back(argv[optind++]);
    }

    if (syntax_error || tables.empty()) {
	cout << "Syntax: " << progname << " [<options>] <table>...\n"
		"\t-r <revno>            Specify revision number to open\n"
		"\t-s <start>            Start at term start\n"
		"\t-e <end>              End at term end" << endl;
	exit(1);
    }

    vector<string>::const_iterator i;
    for (i = tables.begin(); i != tables.end(); i++) {
	try {
	    QuartzDiskTable table(*i, true, 0);
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
	    
	    AutoPtr<QuartzCursor> cursor(table.cursor_get());

	    string key;
	    key = startterm;
	    cursor->find_entry(key);

	    if (startterm.empty() || cursor->current_key < startterm) {
		cerr << "Calling next" << endl;
		cursor->next();
	    }
	    
	    while (!cursor->after_end() &&
		   (!use_endterm || cursor->current_key <= endterm)) {
		cout << hex_encode(cursor->current_key) << " -> "
		     << hex_encode(cursor->current_tag) << "\n";
		cursor->next();
	    }
	}
	catch (const OmError &e) {
	    cout << "Error: " << e.get_msg() << endl;
	}
    }
    return 0;
}
