/* indextext.h: split text into terms
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2004 Olly Betts
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

#include <xapian.h>

#include <limits.h>

#include "symboltab.h"

using std::string;

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

static inline void
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
	unsigned char ch = (unsigned char)*itor;
	if (ch >= 160
#if CHAR_BIT > 8
		      && ch < 256
#endif
				 ) return TRANSLIT1[ch - 160];
	return (char)ch;
    }
    AccentNormalisingItor & operator++() {
	this->operator++(0);
	return *this;
    }
    void operator++(int) {
	if (queued) {
	    queued = 0;
	} else {
	    unsigned char ch = (unsigned char)*itor;
	    if (ch >= 160
#if CHAR_BIT > 8
			  && ch < 256
#endif
				     ) {
		ch = TRANSLIT2[(unsigned char)ch - 160];
		if (ch != ' ') {
		    queued = ch;
		    return;
		}
	    }
	}
	++itor;
    }
    string::const_iterator raw() const { return itor; }
};

Xapian::termpos
index_text(const std::string &s, Xapian::Document &doc, Xapian::Stem &stemmer,
	   Xapian::termcount wdfinc, const std::string &prefix,
	   Xapian::termpos pos = static_cast<Xapian::termpos>(-1)
	   // Not in GCC 2.95.2 numeric_limits<Xapian::termpos>::max()
	   );

inline Xapian::termpos
index_text(const std::string &s, Xapian::Document &doc, Xapian::Stem &stemmer,
	   Xapian::termpos pos)
{
    return index_text(s, doc, stemmer, 1, std::string(), pos);
}
