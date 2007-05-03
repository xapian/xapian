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

using Xapian::Unicode::is_wordchar;

namespace Xapian {

void
TermGenerator::Internal::index_text(Utf8Iterator itor, termcount weight,
				    const string & prefix, bool with_positions)
{
    while (true) {
	itor = find_if(itor, Utf8Iterator(), is_wordchar);
	if (itor == Utf8Iterator()) break;
	string term = prefix;
	do {
	    Unicode::append_utf8(term, Unicode::tolower(*itor));
	    ++itor;
	} while (itor != Utf8Iterator() && is_wordchar(*itor));

	if (!stopper || !(*stopper)(term)) {
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
