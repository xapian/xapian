/* delve.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "om/omerror.h"
#include "quartz_table.h"
#include "quartz_types.h"
#include <iostream>
#include <vector>
#include <string>
#include "ctype.h"

static std::string hex_encode(const std::string & input) {
    const char * table = "0123456789abcdef";
    std::string result;
    std::string::const_iterator i = input.begin();
    for (i = input.begin();
	 i != input.end();
	 i++) {
	unsigned char val = *i;
	if (isprint(val) && !isspace(val) && val != '\\') {
	    result += val;
	} else {
	    result += "\\x";
	    result += table[val/16];
	    result += table[val%16];
	}
    }

    return result;
}

int
main(int argc, char *argv[])
{
    const char *progname = argv[0];
    argv++;
    argc--;
 
    std::vector<std::string> tables;
    quartz_revision_number_t revnum = 0;
    bool use_revno = false;
    om_termname startterm;
    om_termname endterm;

    bool syntax_error = false;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "-r") == 0) {
	    revnum = atoi(argv[1]);
	    use_revno = true;
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "-s") == 0) {
	    startterm = argv[1];
	    use_revno = true;
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "-e") == 0) {
	    endterm = argv[1];
	    use_revno = true;
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }

    while (argc && *argv[0] != '-') {
	tables.push_back(*argv);
	argc--;
	argv++;
    }

    if (syntax_error || argc != 0) {
	std::cout << "Syntax:\t" << progname << " <options> <table>...\n"
		"\t-r <revno>            Specify revision number to open\n"
		"\t-s <start>            Start at term start\n";
		"\t-e <end>              End at term end\n";
	exit(1);
    }

    std::vector<std::string>::const_iterator i;
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

	    std::cout << "Table `" << *i << "' at revision " << openrev;
	    if (openrev != latestrev)
		std::cout << " (Newest revision is " << latestrev << ")";
	    std::cout << std::endl;

	    std::cout << "table contains " << entrycount <<
		    (entrycount == 1 ? " entry" : " entries") << std::endl;
	    
	    AutoPtr<QuartzCursor> cursor(table.cursor_get());

	    QuartzDbKey key;
	    key.value = std::string("\0", 1);
	    //key.value = std::string("\1", 1);
	    cursor->find_entry(key);
	    if (cursor->current_key.value == key.value) {
		std::cerr << "Calling prev" << std::endl;
		cursor->prev();
	    }

	    if (cursor->current_key.value.size() != 0) {
		std::cerr << "Couldn't move to beginning of table "
			"(at key,tag=" <<
			hex_encode(cursor->current_key.value) << "," <<
			hex_encode(cursor->current_tag.value) << ")" <<
			std::endl;
		exit(1);
	    }
	    
	    while (!cursor->after_end()) {
		std::cout << hex_encode(cursor->current_key.value) << " -> "
			  << hex_encode(cursor->current_tag.value) << "\n";
		cursor->next();
	    }
	}
	catch (const OmError &e) {
	    std::cout << "Error: " << e.get_msg() << std::endl;
	}
    }
}
