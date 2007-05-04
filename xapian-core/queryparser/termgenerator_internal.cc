/** @file: termgenerator_internal.cc
 * @brief TermGenerator class internals
 */
/* Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include "termgenerator_internal.h"

#include <xapian/document.h>
#include <xapian/queryparser.h>
#include <xapian/unicode.h>

#include <algorithm>
#include <string>

using namespace std;

namespace Xapian {

// FIXME: handling for unstemmed terms?  R-prefixes for uppercase first
// character, or something more sophisticated (e.g. stemmed terms without
// positional information, unstemmed terms with).
// FIXME: handling for '.' in "I.B.M." - should generate term "IBM".

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms.
static const unsigned int MAX_PROB_TERM_LENGTH = 64;
// FIXME: threshold is currently in bytes of UTF-8 representation, not unicode
// characters - what actually makes most sense here?

inline unsigned check_wordchar(unsigned ch) {
    if (Unicode::is_wordchar(ch)) return Unicode::tolower(ch);
    return 0;
}

inline unsigned check_infix(unsigned ch) {
    if (ch == '\'' || ch == '&') return ch;
    // 0x2019 is Unicode apostrophe and single closing quote.
    // 0x201b is Unicode single opening quote with the tail rising.
    if (ch == 0x2019 || ch == 0x201b) return '\'';
    return 0;
}

inline unsigned check_suffix(unsigned ch) {
    if (ch == '+' || ch == '#') return ch;
    // FIXME: what about '-'?
    return 0;
}

void
TermGenerator::Internal::index_text(Utf8Iterator itor, termcount weight,
				    const string & prefix, bool with_positions)
{
    while (true) {
	// Advance to the start of the next term.
	unsigned ch;
	while (true) {
	    if (itor == Utf8Iterator()) return;
	    ch = check_wordchar(*itor);
	    if (ch) break;
	    ++itor;
	}

	string term = prefix;
	while (true) {
	    do {
		Unicode::append_utf8(term, ch);
		if (++itor == Utf8Iterator()) goto endofterm;
		ch = check_wordchar(*itor);
	    } while (ch);

	    // Handle things like '&' in AT&T, apostrophes, etc.
	    unsigned infix_ch = check_infix(*itor);
	    if (!infix_ch) break;
	    Utf8Iterator next(itor);
	    ++next;

	    // Need to handle the possibility that a character is both infix
	    // and suffix.
	    if (next == Utf8Iterator()) break;
	    itor = next;
	    ch = check_wordchar(*itor);
	    if (!ch) break;

	    Unicode::append_utf8(term, infix_ch);
	}

	{
	    size_t len = term.size();
	    unsigned count = 0;
	    while ((ch = check_suffix(*itor))) {
		if (++count > 3) {
		    term.resize(len);
		    break;
		}
		Unicode::append_utf8(term, ch);
		if (++itor == Utf8Iterator()) goto endofterm;
	    }
	}

endofterm:
	if (term.size() <= MAX_PROB_TERM_LENGTH &&
	    (!stopper || !(*stopper)(term))) {
	    term = stemmer(term);
	    if (with_positions) {
		doc.add_posting(term, weight, ++termpos);
	    } else {
		doc.add_term(term, weight);
	    }
	}
    }
}

}
