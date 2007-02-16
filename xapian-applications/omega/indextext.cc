/* indextext.cc: tokenise text to produce terms
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004,2005,2006,2007 Olly Betts
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
#include "tclUniData.h"
#include "utf8itor.h"

#include <ctype.h>

using namespace std;

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

    char buf[4];

    Utf8Iterator j(s);
    const Utf8Iterator s_end;
    while (true) {
	Utf8Iterator first = j;
	while (first != s_end && !is_wordchar(*first)) ++first;
	if (first == s_end) break;
	Utf8Iterator last;
	string term;
	if (*first < 128 && isupper(*first)) {
	    j = first;
	    term.append(buf, to_utf8(*j, buf));
	    while (++j != s_end && *j == '.' && ++j != s_end && *j < 128 && isupper(*j)) {
		term.append(buf, to_utf8(*j, buf));
	    }
	    if (term.length() < 2 || (j != s_end && is_wordchar(*j))) {
		term = "";
	    }
	    last = j;
	}
	if (term.empty()) {
	    j = first;
	    while (is_wordchar(*j)) {
		term.append(buf, to_utf8(*j, buf));
		++j;
		if (j == s_end) break;
		if (*j == '&' || *j == '\'') {
		    Utf8Iterator next = j;
		    ++next;
		    if (next == s_end || !is_wordchar(*next)) break;
		    term += *j;
		    j = next;
		}
	    }
	    last = j;
	    if (j != s_end && (*j == '+' || *j == '-' || *j == '#')) {
		string::size_type len = term.length();
		if (*j == '#') {
		    term += '#';
		    do { ++j; } while (j != s_end && *j == '#');
		} else {
		    while (j != s_end && (*j == '+' || *j == '-')) {
			term.append(buf, to_utf8(*j, buf));
			++j;
		    }
		}
		if (term.size() - len > 3 || (j != s_end && is_wordchar(*j))) {
		    term.resize(len);
		} else {
		    last = j;
		}
	    }
	}
	j = last;

	if (term.length() <= MAX_PROB_TERM_LENGTH) {
	    term = U_downcase_term(term);
	    if (isupper(static_cast<unsigned char>(*first))) {
		if (pos != static_cast<Xapian::termpos>(-1)
			// Not in GCC 2.95.2 numeric_limits<Xapian::termpos>::max()
		   ) {
		    doc.add_posting(rprefix + term, pos, wdfinc);
		} else {
		    doc.add_term(rprefix + term, wdfinc);
		}
	    }

	    term = stemmer(term);
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
