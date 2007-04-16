/* indextext.h: split text into terms
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

#include <xapian.h>

#include <string>

// Put a limit on the size of terms to help prevent the index being bloated
// by useless junk terms
static const unsigned int MAX_PROB_TERM_LENGTH = 64;

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
