/* stemtest.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

// Quick test for the stemming algorithms: bit of a hacked rush job this...

#include <stdio.h>
#include <ctype.h>  /* for isupper, islower, toupper, tolower */

#include <string>

#include "om/omstem.h"

#define IS_LETTER(ch)  (islower(ch) || (ch) == '^')

void stemfile(const OmStem &stemmer, FILE * f)
{
    while(true) {
	int ch = getc(f);
	if (ch == EOF) return;
	if (IS_LETTER(ch)) {
	    string word;
	    while(true) {
		/* force lower case: */
		if (isupper(ch)) {
		    ch = tolower(ch);
		}

		word += ch;

		ch = getc(f);
		if (!IS_LETTER(ch)) { ungetc(ch, f); break; }
	    }

	    cout << stemmer.stem_word(word);
	} else putchar(ch);
    }
}

int main(int argc, char **argv)
{
    string lang = "english";
    if(argc > 1) {
	bool noarg = false;
	if(!strcmp(argv[1], "--dutch")) {
	    lang = "dutch";
	} else if(!strcmp(argv[1], "--english")) {
	    lang = "english";
	} else if(!strcmp(argv[1], "--french")) {
	    lang = "french";
	} else if(!strcmp(argv[1], "--german")) {
	    lang = "german";
	} else if(!strcmp(argv[1], "--italian")) {
	    lang = "italian";
	} else if(!strcmp(argv[1], "--portuguese")) {
	    lang = "portuguese";
	} else if(!strcmp(argv[1], "--spanish")) {
	    lang = "spanish";
	} else {
	    noarg = true;
	}
	if(!noarg) {
	    argc--;
	    argv++;
	}
    }
    OmStem stemmer(lang);

    for (int i = 1; i < argc; i++)
    {
	FILE * f = fopen(argv[i], "r");
	if (f == 0) {
	    fprintf(stderr, "File %s not found\n", argv[i]);
	    exit(1);
	}
	stemfile(stemmer, f);
    }

    return 0;
}
