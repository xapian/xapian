/* accentnormalisingitor.h: AccentNormalisingItor class for normalising accents.
 *
 * Copyright (C) 2003,2004,2005 Olly Betts
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
 */

#include "symboltab.h"

#include <string>

using std::string;

/** A wrapper class for a char which returns the char if dereferenced 
 *  with *.  We need this to implement input_iterator semantics.
 */
class CharWrapper {
    private:
	char ch;
    public:
	CharWrapper(char ch_) : ch(ch_) { }
	char operator*() const { return ch; }
};

class AccentNormalisingItor {
  private:
    string::const_iterator itor;
    char queued;
    size_t trans;

  public:
    AccentNormalisingItor()
	: itor(), queued(0), trans(0) {}
    AccentNormalisingItor(string::const_iterator itor_)
	: itor(itor_), queued(0), trans(0) {}
    void operator=(string::const_iterator itor_)
    {
	itor = itor_;
	queued = 0;
	trans = 0;
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
#if CHAR_BIT > 8 // Avoid compiler warning.
		      && ch < 256
#endif
				 ) return TRANSLIT1[ch - 160];
	return (char)ch;
    }
    AccentNormalisingItor & operator++() {
	if (queued) {
	    queued = 0;
	} else {
	    unsigned char ch = (unsigned char)*itor;
	    if (ch >= 160
#if CHAR_BIT > 8 // Avoid compiler warning.
			  && ch < 256
#endif
				     ) {
		++trans;
		ch = TRANSLIT2[ch - 160];
		if (ch != ' ') {
		    queued = ch;
		    return *this;
		}
	    }
	}
	++itor;
	return *this;
    }
    CharWrapper operator++(int) {
	char tmp = **this;
	operator++();
	return CharWrapper(tmp);
    }
    size_t transliterations() const { return trans; }
    string::const_iterator raw() const { return itor; }

    /// We implement the semantics of an STL input_iterator.
    //@{
    typedef std::input_iterator_tag iterator_category;
    typedef char value_type;
    typedef string::size_type difference_type;
    typedef const char * pointer;
    typedef const char & reference;
    //@}
};
