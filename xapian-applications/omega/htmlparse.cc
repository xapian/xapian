/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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

#include <algorithm>
using std::find;
using std::find_if;
#include "htmlparse.h"
#include <stdio.h>
#include <ctype.h>

inline static bool
p_alpha(char c)
{
    return (((unsigned int)c | 32) - 'a') <= ('z' - 'a');
}

inline static bool
p_notdigit(char c)
{
    return ((unsigned int)c - '0') <= ('9' - '0');
}

inline static bool
p_notxdigit(char c)
{
    return !isxdigit(c);
}

inline static bool
p_notalnum(char c)
{
    return !isalnum(c);
}

inline static bool
p_notwhitespace(char c)
{
    return !isspace(c);
}

inline static bool
p_nottag(char c)
{
    return !isalnum(c) && c != '.' && c != '-';
}

inline static bool
p_whitespacegt(char c)
{
    return isspace(c) || c == '>';
}

inline static bool
p_whitespaceeqgt(char c)
{
    return isspace(c) || c == '=' || c == '>';
}

void
HtmlParser::decode_entities(string &s)
{
    // We need a const_iterator version of s.end() - otherwise the
    // find() and find_if() templates don't work...
    string::const_iterator amp = s.begin(), s_end = s.end();
    while ((amp = find(amp, s_end, '&')) != s_end) {
	unsigned int val = 0;
	string::const_iterator end, p = amp + 1;
	if (p != s_end && *p == '#') {
	    p++;
	    if (p != s_end && tolower(*p) == 'x') {
		// hex
		p++;
		end = find_if(p, s_end, p_notxdigit);
		sscanf(s.substr(p - s.begin(), end - p).c_str(), "%x", &val);
	    } else {
		// number
		end = find_if(p, s_end, p_notdigit);
		val = atoi(s.substr(p - s.begin(), end - p).c_str());
	    }
	} else {
	    end = find_if(p, s_end, p_notalnum);
	    string code = s.substr(p - s.begin(), end - p);
	    // FIXME: make this list complete
	    if (code == "amp") val = '&';
	    else if (code == "lt") val = '<';
	    else if (code == "gt") val = '>';
	    else if (code == "nbsp") val = '\xa0';
	    else if (code == "quot") val = '\"';
	}
	if (end < s_end && *end == ';') end++;
	if (val) {
	    s.replace(amp - s.begin(), end - amp, 1u, char(val));
	    s_end = s.end();
	    ++amp;
	} else {
	    amp = end;
	}
    }
}

void
HtmlParser::parse_html(const string &body)
{
    map<string,string> Param;
    string::const_iterator start = body.begin();

    while (1) {
	// Skip through until we find an HTML tag, a comment, or the end of
	// document.  Ignore isolated occurences of `<' which don't start
	// a tag or comment
	string::const_iterator p = start;
	while (1) {
	    p = find(p, body.end(), '<');
	    if (p == body.end()) break;
	    char ch = *(p + 1);
	    if (isalpha(ch) || ch == '/' || ch == '!') break;
	    p++; 
	}


	// process text up to start of tag
	if (p > start) {
	    string text = body.substr(start - body.begin(), p - start);
	    decode_entities(text);
	    process_text(text);
	}

	if (p == body.end()) break;

	start = p + 1;
   
	if (*start == '!') {
	    // comment or SGML declaration
	    if (*(start + 1) == '-' && *(start + 2) == '-') {
		start = find(start + 3, body.end(), '>');
		// unterminated comment swallows rest of document
		// (like NS, but unlike MSIE iirc)
		if (start == body.end()) break;
		
		p = start;
		// look for -->
		while (p != body.end() && (*(p - 1) != '-' || *(p - 2) != '-'))
		    p = find(p + 1, body.end(), '>');

		// If we found --> skip to there, otherwise
		// skip to the first > we found (as Netscape does)
		if (p != body.end()) start = p;
	    } else {
		// just an SGML declaration, perhaps giving the DTD - ignore it
		start = find(start + 1, body.end(), '>');
		if (start == body.end()) break;
	    }
	    start++;
	} else {
	    // opening or closing tag
	    int closing = 0;

	    if (*start == '/') {
		closing = 1;
		start = find_if(start + 1, body.end(), p_notwhitespace);
	    }
	      
	    p = start;
	    start = find_if(start, body.end(), p_nottag);
	    string tag = body.substr(p - body.end(), start - p);
	    // convert tagname to lowercase
	    for (string::iterator i = tag.begin(); i != tag.end(); i++)
		*i = tolower(*i);
	       
	    if (closing) {
		closing_tag(tag);
		   
		/* ignore any bogus parameters on closing tags */
		p = find(start, body.end(), '>');
		if (p == body.end()) break;
		start = p + 1;
	    } else {
		while (start < body.end() && *start != '>') {
		    string name, value;

		    p = find_if(start, body.end(), p_whitespaceeqgt);

		    name = body.substr(start - body.begin(), p - start);
		       
		    p = find_if(p, body.end(), p_notwhitespace);
		      
		    start = p;
		    if (*start == '=') {
			int quote;
		       
			start = find_if(start + 1, body.end(), p_notwhitespace);

			p = body.end();
			   
			quote = *start;
			if (quote == '"' || quote == '\'') {
			    start++;
			    p = find(start, body.end(), quote);
			}
			   
			if (p == body.end()) {
			    // unquoted or no closing quote
			    p = find_if(start, body.end(), p_whitespacegt);
			    
			    value = body.substr(start - body.begin(), p - start);

			    start = find_if(p, body.end(), p_notwhitespace);
			} else {
			    value = body.substr(start - body.begin(), p - start);
			}
		       
			if (name.size()) {
			    // convert parameter name to lowercase
			    string::iterator i;
			    for (i = name.begin(); i != name.end(); i++)
				*i = tolower(*i);
			    // in case of multiple entries, use the first
			    // (as Netscape does)
			    if (Param.find(name) == Param.end())
				Param[name] = value;
		       }
		   }
	       }
	       opening_tag(tag, Param);
	       Param.clear();
	       
	       if (*start == '>') start++;
	   }
	}
    }
}
