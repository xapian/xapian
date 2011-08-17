/* hashterm.cc: generate a URL term, truncating and hashing very long URLs.
 *
 * Copyright (C) 2003 Lemur Consulting Ltd.
 * Copyright (C) 2003,2004,2006,2011 Olly Betts
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
#include "hashterm.h"

using namespace std;

/* Hash is computed as an unsigned long, and then converted to a
 * string by writing 6 bits of it to each output byte.  So length is
 * ceil(4 * 8 / 6) (we use 4 rather than sizeof(unsigned long) so
 * that the hash is the same regardless of the platform).
 */
const unsigned int HASH_LEN = ((4 * 8 + 5) / 6);

/* Make a hash of a string - this isn't a very good hashing algorithm, but
 * it's fast.  A collision would result in a document overwriting a different
 * document, which is not desirable, but also wouldn't be a total disaster.
 */
static string
hash_string(const string &s)
{
    unsigned long int h = 1;
    for (string::const_iterator i = s.begin(); i != s.end(); ++i) {
	h += (h << 5) + static_cast<unsigned char>(*i);
    }
    h &= 0xffffffff; // In case sizeof(unsigned long) > 4
    // FIXME: It's quirky that we make leading zeros ' ' here, but "embedded"
    // zeros become char(33) below.  Not a problem, but perhaps change ' ' to
    // char(33) if we need to break backwards compatibility for some other
    // reason.
    string result(HASH_LEN, ' ');
    size_t j = 0;
    while (h != 0) {
	char ch = char((h & 63) + 33);
	result[j++] = ch;
	h = h >> 6;
    }
    return result;
}

string
hash_long_term(const string &term, unsigned int max_length)
{
    if (term.length() <= max_length) return term;
    string result(term);
    max_length -= HASH_LEN;
    result.replace(max_length, string::npos,
		   hash_string(result.substr(max_length)));
    return result;
}
