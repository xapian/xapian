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

#include "stemmer.h"

#define LETTER(ch)  (islower(ch) || (ch) == '^')

void stemfile(Stemmer *stemmer, FILE * f)
{   while(true)
    {   int ch = getc(f);
	if (ch == EOF) return;
	if (LETTER(ch))
	{
	    string word;
	    while(true)
	    {
		/* force lower case: */
		if isupper(ch) ch = tolower(ch);

		word += ch;

		ch = getc(f);
		if (!LETTER(ch)) { ungetc(ch, f); break; }
	    }

	    cout << stemmer->stem_word(word);
	}
	else putchar(ch);
    }
}


int main(int argc, char **argv)
{
    stemmer_language lang = STEMLANG_ENGLISH;
    if(argc > 1) {
	bool noarg = false;
	if(!strcmp(argv[1], "--dutch")) {
	    lang = STEMLANG_DUTCH;
	} else if(!strcmp(argv[1], "--english")) {
	    lang = STEMLANG_ENGLISH;
	} else if(!strcmp(argv[1], "--french")) {
	    lang = STEMLANG_FRENCH;
	} else if(!strcmp(argv[1], "--german")) {
	    lang = STEMLANG_GERMAN;
	} else if(!strcmp(argv[1], "--italian")) {
	    lang = STEMLANG_ITALIAN;
	} else if(!strcmp(argv[1], "--portuguese")) {
	    lang = STEMLANG_PORTUGUESE;
	} else if(!strcmp(argv[1], "--spanish")) {
	    lang = STEMLANG_SPANISH;
	} else {
	    noarg = true;
	}
	if(!noarg) {
	    argc--;
	    argv++;
	}
    }
    Stemmer * stemmer = StemmerBuilder::create(lang);

    cout << "stemtest version 1: stemming in " << stemmer->get_lang() << endl;

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
