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

#ifndef HAVE_SSTREAM
#include "omstringstream.h"
#include "utils.h"

om_ostringstream::om_ostringstream()
{
}

om_ostringstream::~om_ostringstream()
{
}

std::string
om_ostringstream::str() const
{
    return mystring;
}

om_ostringstream & om_ostringstream::operator << (const std::string & msg)
{
    mystring += msg;
    return *this;
}

om_ostringstream & om_ostringstream::operator << (const char * msg)
{
    if (msg) {
	mystring += std::string(msg);
    } else {
	mystring += "<NULL>";
    }
    return *this;
}

om_ostringstream & om_ostringstream::operator << (const void * msg)
{
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (char msg)
{
    mystring += msg;
    return *this;
}

om_ostringstream & om_ostringstream::operator << (unsigned char msg)
{
    mystring += msg;
    return *this;
}

om_ostringstream & om_ostringstream::operator << (int msg) {
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (unsigned int msg) {
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (long msg) {
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (unsigned long msg) {
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (double msg) {
    return *this << om_tostring(msg);
}

om_ostringstream & om_ostringstream::operator << (bool msg) {
    return *this << om_tostring(msg);
}

#if 0
om_ostringstream & om_ostringstream::operator << (ostream& (*msg)(ostream&)) {
    if (msg == std::endl) return *this << '\n';
    if (msg == std::flush) return *this;
    return *this;
}
#endif

#endif // HAVE_SSTREAM
