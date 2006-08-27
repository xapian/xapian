/* indextext.cc: split text into terms
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006 Olly Betts
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

#include "indextext.h"

#include "symboltab.h"

#include <ctype.h>

using namespace std;

inline static bool
p_plusminus(unsigned int c)
{
    return c == '+' || c == '-';
}

Xapian::termpos
index_text(const string &s, Xapian::Document &doc, Xapian::Stem &stemmer,
	   Xapian::termcount wdfinc, const string &prefix,
	   Xapian::termpos pos)
{
    string rprefix = prefix;
    // If we're using a multi-character prefix, make sure to add a colon when
    // generating raw (R) terms as otherwise XFOO + Rterm will collide with
    // XFOOR + term
    if (rprefix.size() > 1 && rprefix[rprefix.size() - 1] != ':')
	rprefix += ':';
    rprefix += 'R';

    AccentNormalisingItor j(s.begin());
    const AccentNormalisingItor s_end(s.end());
    while (true) {
	AccentNormalisingItor first = j;
	while (first != s_end && !isalnum(*first)) ++first;
	if (first == s_end) break;
	AccentNormalisingItor last;
	string term;
	if (isupper(*first)) {
	    j = first;
	    term = *j;
	    while (++j != s_end && *j == '.' && ++j != s_end && isupper(*j)) {
		term += *j;
	    } 
	    if (term.length() < 2 || (j != s_end && isalnum(*j))) {
		term = "";
	    }
	    last = j;
	}
	if (term.empty()) {
	    j = first;
	    while (isalnum(*j)) {
		term += *j;
		++j;
		if (j == s_end) break;
		if (*j == '&') {
		    AccentNormalisingItor next = j;
		    ++next;
		    if (next == s_end || !isalnum(*next)) break;
		    term += '&';
		    j = next;
		}
	    }
	    last = j;
	    if (j != s_end && (*j == '#' || p_plusminus(*j))) {
		string::size_type len = term.length();
		if (*j == '#') {
		    term += '#';
		    do { ++j; } while (j != s_end && *j == '#');
		} else {
		    while (j != s_end && p_plusminus(*j)) {
			term += *j;
			++j;
		    }
		}
		if (term.size() - len > 3 || (j != s_end && isalnum(*j))) {
		    term.resize(len);
		} else {
		    last = j;
		}
	    }
	}
	if (term.length() <= MAX_PROB_TERM_LENGTH) {
	    lowercase_term(term);
	    if (isupper(*first)) {
		if (pos != static_cast<Xapian::termpos>(-1)
			// Not in GCC 2.95.2 numeric_limits<Xapian::termpos>::max()
		   ) {
		    doc.add_posting(rprefix + term, pos, wdfinc);
		} else {
		    doc.add_term(rprefix + term, wdfinc);
		}
	    }

	    term = stemmer.stem_word(term);
	    if (pos != static_cast<Xapian::termpos>(-1)
		    // Not in GCC 2.95.2 numeric_limits<Xapian::termpos>::max()
	       ) {
		doc.add_posting(prefix + term, pos++, wdfinc);
	    } else {
		doc.add_term(prefix + term, wdfinc);
	    }
	}
    }
    return pos;
}
