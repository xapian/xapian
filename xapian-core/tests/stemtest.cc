/* stemtest.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <stdio.h>
#include <ctype.h>
#include <getopt.h>

#include <string>
#include <iostream>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

#include "om/omstem.h"

static void
stemfile(const OmStem &stemmer, FILE * f)
{
    while (true) {
	int ch = getc(f);
	if (ch == EOF) return;
	if (isspace(ch)) {
	    putchar(ch);
	    continue;
	}

	std::string word;
	while (true) {
	    word += tolower(ch);

	    ch = getc(f);
	    if (ch == EOF) break;
	    if (isspace(ch)) {
		ungetc(ch, f);
		break;
	    }
	}

	cout << stemmer.stem_word(word);
    }
}

int main(int argc, char **argv)
{
    std::string lang = "english";

    struct option opts[] = {
	{"language",	required_argument, 0, 'l'},
	{NULL,		0, 0, 0}
    };

    bool syntax_error = false;

    int c;
    while ((c = getopt_long(argc, argv, "l", opts, NULL)) != EOF) {
	if (c == 'l') {
	    lang = argv[optind];
	} else {
	    syntax_error = true;
	}
    }

    try {
	OmStem stemmer(lang);
	while (argv[optind]) {
	    FILE * f = fopen(argv[optind], "r");
	    if (f == NULL) {
		cerr << "File " << argv[optind] << " not found\n";
		exit(1);
	    }
	    stemfile(stemmer, f);
	    ++optind;
	}
    } catch (const OmError &e) {
	cout << e.get_msg() << endl;
	return 1;
    }

    return 0;
}
