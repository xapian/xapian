/* htmlparse.cc: simple HTML parser for omega indexer
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Ananova Ltd
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

#include <algorithm>
using std::find;
using std::find_if;
#include "htmlparse.h"
#include <stdio.h>
#include <ctype.h>

map<string, unsigned int> HtmlParser::named_ents;

inline static bool
p_alpha(char c)
{
    return isalpha(c);
}

inline static bool
p_notdigit(char c)
{
    return !isdigit(c);
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

HtmlParser::HtmlParser()
{
    static struct ent { const char *n; unsigned int v; } ents[] = {
	{ "quot", 34 },
	{ "amp", 38 },
	{ "lt", 60 },
	{ "gt", 62 },
	{ "AElig", 198 },
	{ "Aacute", 193 },
	{ "Acirc", 194 },
	{ "Agrave", 192 },
	{ "Aring", 197 },
	{ "Atilde", 195 },
	{ "Auml", 196 },
	{ "Ccedil", 199 },
	{ "ETH", 208 },
	{ "Eacute", 201 },
	{ "Ecirc", 202 },
	{ "Egrave", 200 },
	{ "Euml", 203 },
	{ "Iacute", 205 },
	{ "Icirc", 206 },
	{ "Igrave", 204 },
	{ "Iuml", 207 },
	{ "Ntilde", 209 },
	{ "Oacute", 211 },
	{ "Ocirc", 212 },
	{ "Ograve", 210 },
	{ "Oslash", 216 },
	{ "Otilde", 213 },
	{ "Ouml", 214 },
	{ "THORN", 222 },
	{ "Uacute", 218 },
	{ "Ucirc", 219 },
	{ "Ugrave", 217 },
	{ "Uuml", 220 },
	{ "Yacute", 221 },
	{ "aacute", 225 },
	{ "acirc", 226 },
	{ "acute", 180 },
	{ "aelig", 230 },
	{ "agrave", 224 },
	{ "aring", 229 },
	{ "atilde", 227 },
	{ "auml", 228 },
	{ "brvbar", 166 },
	{ "ccedil", 231 },
	{ "cedil", 184 },
	{ "cent", 162 },
	{ "copy", 169 },
	{ "curren", 164 },
	{ "deg", 176 },
	{ "divide", 247 },
	{ "eacute", 233 },
	{ "ecirc", 234 },
	{ "egrave", 232 },
	{ "eth", 240 },
	{ "euml", 235 },
	{ "frac12", 189 },
	{ "frac14", 188 },
	{ "frac34", 190 },
	{ "iacute", 237 },
	{ "icirc", 238 },
	{ "iexcl", 161 },
	{ "igrave", 236 },
	{ "iquest", 191 },
	{ "iuml", 239 },
	{ "laquo", 171 },
	{ "macr", 175 },
	{ "micro", 181 },
	{ "middot", 183 },
	{ "nbsp", 160 },
	{ "not", 172 },
	{ "ntilde", 241 },
	{ "oacute", 243 },
	{ "ocirc", 244 },
	{ "ograve", 242 },
	{ "ordf", 170 },
	{ "ordm", 186 },
	{ "oslash", 248 },
	{ "otilde", 245 },
	{ "ouml", 246 },
	{ "para", 182 },
	{ "plusmn", 177 },
	{ "pound", 163 },
	{ "raquo", 187 },
	{ "reg", 174 },
	{ "sect", 167 },
	{ "shy", 173 },
	{ "sup1", 185 },
	{ "sup2", 178 },
	{ "sup3", 179 },
	{ "szlig", 223 },
	{ "thorn", 254 },
	{ "times", 215 },
	{ "uacute", 250 },
	{ "ucirc", 251 },
	{ "ugrave", 249 },
	{ "uml", 168 },
	{ "uuml", 252 },
	{ "yacute", 253 },
	{ "yen", 165 },
	{ "yuml", 255 },
// iso8859-1 only for now	{ "OElig", 338 },
// ditto			{ "oelig", 339 },
	{ NULL, 0 }
    };
    if (named_ents.empty()) {
	struct ent *i = ents;
	while (i->n) {
	    named_ents[string(i->n)] = i->v;
	    ++i;
	}
    }
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
	    map<string, unsigned int>::const_iterator i;
	    i = named_ents.find(code);
	    if (i != named_ents.end()) val = i->second;
	}
	if (end < s_end && *end == ';') end++;
	if (val) {
	    string::size_type amp_pos = amp - s.begin();
	    s.replace(amp_pos, end - amp, 1u, char(val));
	    s_end = s.end();
	    // We've modified the string, so the iterators are no longer
	    // valid...
	    amp = s.begin() + amp_pos + 1;
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
	    // tag, closing tag, comment (or SGML declaration), or PHP
	    if (isalpha(ch) || ch == '/' || ch == '!' || ch == '?') break;
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
   
	if (start == body.end()) break;

	if (*start == '!') {
	    if (++start == body.end()) break;
	    if (++start == body.end()) break;
	    // comment or SGML declaration
	    if (*(start - 1) == '-' && *start == '-') {
		start = find(start + 1, body.end(), '>');
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
		start = find(start - 1, body.end(), '>');
		if (start == body.end()) break;
	    }
	    ++start;
	} else if (*start == '?') {
	    if (++start == body.end()) break;
	    // PHP - swallow until ?> or EOF
	    start = find(start + 1, body.end(), '>');

	    // look for ?>
	    while (start != body.end() && *(start - 1) != '?')
		start = find(start + 1, body.end(), '>');

	    // unterminated PHP swallows rest of document (rather arbitrarily
	    // but it avoids polluting the database when things go wrong)
	    if (start != body.end()) ++start;
	} else {
	    // opening or closing tag
	    int closing = 0;

	    if (*start == '/') {
		closing = 1;
		start = find_if(start + 1, body.end(), p_notwhitespace);
	    }
	      
	    p = start;
	    start = find_if(start, body.end(), p_nottag);
	    string tag = body.substr(p - body.begin(), start - p);
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
		    if (start != body.end() && *start == '=') {
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

		if (start != body.end() && *start == '>') ++start;
	    }
	}
    }
}
