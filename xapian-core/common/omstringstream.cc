/* omstringstream.cc : A replacement for stringstream.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "config.h"
#include "omstringstream.h"

#ifndef HAVE_SSTREAM

template <class Ch, class Tr>
om_stringbuf<Ch, Tr>::om_stringbuf()
{
}

template <class Ch, class Tr>
int om_stringbuf<Ch, Tr>::overflow(int c)
{
    buffer += std::string(pbase(), pptr());
    if (c != EOF) {
	buffer += c;
    }
    return c;
}

om_ostringstream::om_ostringstream()
	: ostream(0), ombuf(new om_stringbuf<char>())
{
    rdbuf(ombuf.get());
}

om_ostringstream::~om_ostringstream()
{
}

std::string
om_ostringstream::str() const
{
    ombuf->overflow();

    return ombuf->buffer;
}

#endif // HAVE_SSTREAM
