/* indextext.cc: split text into terms
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003 Olly Betts
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

#include "indextext.h"

#include <ctype.h>

using namespace std;

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

static void
lowercase_term(string &term)
{
    string::iterator i = term.begin();
    while (i != term.end()) {
	*i = tolower(*i);
	i++;
    }
}

class AccentNormalisingItor {
  private:
    string::const_iterator itor;
    char queued;

  public:
    AccentNormalisingItor()
	: itor(), queued(0) {}
    AccentNormalisingItor(string::const_iterator itor_)
	: itor(itor_), queued(0) {}
    void operator=(string::const_iterator itor_)
    {
	itor = itor_;
	queued = 0;
    }
    bool operator==(const AccentNormalisingItor &o) const {
	return queued == o.queued && itor == o.itor;
    }
    bool operator!=(const AccentNormalisingItor &o) const {
	return !(*this == o);
    }
    char operator*() const {
	if (queued) return queued;
	char ch = *itor;
	char cache; // dummy - don't want the value
	switch ((unsigned char)ch) {
#include "symboltab.h" // FIXME: rework symboltab into arrays ch[] and cache[]
	}
	return ch;
    }
    AccentNormalisingItor & operator++() {
	this->operator++(0);
	return *this;
    }
    void operator++(int) {
	if (queued) {
	    queued = 0;
	    ++itor;
	} else {
	    char ch; // dummy - don't want the value
	    char cache = 0;
	    switch ((unsigned char)*itor) {
#include "symboltab.h" // FIXME: rework symboltab into arrays ch[] and cache[]
	    }
	    queued = cache;
	    if (!queued) ++itor;
	}
    }
};

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
	    string::size_type len = term.length();
	    last = j;
	    while (j != s_end && p_plusminus(*j)) {
		term += *j;
		++j;
	    }
	    if (j != s_end && isalnum(*j)) {
		term.resize(len);
	    } else {
		last = j;
	    }
	}
	if (term.length() <= MAX_PROB_TERM_LENGTH) {
	    lowercase_term(term);
	    if (isupper(*first) || isdigit(*first)) {
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
