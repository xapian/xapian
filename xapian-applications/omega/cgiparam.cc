/* cgiparam.cc: Parse CGI parameters
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 James Aylett
 * Copyright 2001 Ananova Ltd
 * Copyright 2002,2003,2009 Olly Betts
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

#include "cgiparam.h"

#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>

using namespace std;

multimap<string, string> cgi_params;

static void
add_param(string name, string val)
{
    size_t i = name.length();
    if (i > 2 && name[i - 2] == '.') {
	// An image button called B gives B.x and B.y parameters with the
	// coordinates of the click.  We throw away the ".y" one and trim
	// ".x" from the other.
	if (name[i - 1] == 'y') return;
	if (name[i - 1] == 'x') {
	    name.resize(i - 2);
	    // For an image button, the value of the CGI parameter is the
	    // coordinate of the click within the image - this is meaningless
	    // to us, so instead we turn "[ 2 ].x=NNN" into "[ 2 ]=2 ]", then
	    // below that gets turned into "[=2 ]".  The trailing non-numeric
	    // characters are ignored by atoi().
	    i = name.find(' ');
	    if (i != string::npos)
		val = name.substr(i + 1);
	    else {
		i = name.find_first_not_of("0123456789");
		if (i == string::npos) {
		    // For image buttons with entirely numeric names, make the
		    // value the name, and the name "#" - e.g. "2.x=NNN" becomes
		    // "#=2".
		    val = name;
		    name = '#';
		} else {
		    // Otherwise we just copy the name into the value, so
		    // ">.x=NNN" becomes ">=>".
		    val = name;
		}
	    }
	}
    }
    // Truncate at first space - convert `[ page two ]=2'
    // into `[=2'
    i = name.find(' ');
    if (i != string::npos) name.resize(i);
    cgi_params.insert(multimap<string, string>::value_type(name, val));
}

void
decode_argv(char **argv)
{
    cgi_params.clear();
    while (*argv) {
	char *p = strchr(*argv, '=');
	if (p) {
	    add_param(string(*argv, p), p + 1);
	} else {
	    add_param(*argv, "");
	}
	++argv;
    }
}

void
decode_test()
{
    cgi_params.clear();
    while (!feof(stdin)) {
	string name, val;
	bool had_equals = false;
	while (1) {
	    int ch = getchar();
	    if (ch == EOF || ch == '\n') {
		if (name.empty()) return; // end on blank line
		add_param(name, val);
		break;
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    }
}

void
decode_post()
{
    char *content_length;
    unsigned int cl = INT_MAX;
    
    content_length = getenv("CONTENT_LENGTH");
    /* Netscape Fasttrack server for NT doesn't give CONTENT_LENGTH */
    if (content_length) cl = atoi(content_length);

    cgi_params.clear();
    while (cl && (!feof(stdin))) {
	string name, val;
	bool had_equals = false;
	while (1) {
	    int ch = EOF;
	    if (cl) {
		ch = getchar();
		cl--;
	    }
	    if (ch == EOF || ch == '&') {
		if (!name.empty()) add_param(name, val);
		break;
	    }
	    char orig_ch = ch;
	    if (ch == '+')
		ch = ' ';
	    else if (ch == '%') {
		if (cl >= 2) {
		    cl -= 2;
		    int c = getchar();
		    ch = (c & 0xf) + ((c & 64) ? 9 : 0);
		    if (c != EOF) c = getchar();
		    ch = ch << 4;
		    ch |= (c & 0xf) + ((c & 64) ? 9 : 0);
	        }
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (orig_ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    }
}

void
decode_get()
{
    const char *q_str = getenv("QUERY_STRING");
    if (!q_str) q_str = ""; // Hmm, sounds like a broken web server

    cgi_params.clear();
    char ch;
    do {
	string name, val;
	bool had_equals = false;
	while (1) {
	    ch = *q_str++;
	    if (ch == '\0' || ch == '&') {
		if (name.empty()) return; // end on blank line
		add_param(name, val);
		break;
	    }
	    char orig_ch = ch;
	    if (ch == '+')
		ch = ' ';
	    else if (ch == '%') {
		int c = *q_str++;
		ch = (c & 0xf) + ((c & 64) ? 9 : 0);
		if (c) c = *q_str++;
		ch = ch << 4;
		ch |= (c & 0xf) + ((c & 64) ? 9 : 0);
		if (!c) return; // unfinished % code
	    }
	    if (had_equals) {
		val += char(ch);
	    } else if (orig_ch == '=') {
		had_equals = true;
	    } else {
		name += char(ch);
	    }
	}
    } while (ch);
}
