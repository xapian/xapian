/************************************************************
 *
 *  char_sequence.cpp implementation.
 *
 *  (c) 2001 Amir Michail (amir@users.sourceforge.net)
 *  Modified by Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#include "char_sequence.h"

char
char_sequence::space() 
{
    return (char)1;
}

int
char_sequence::score( const char c1, const char c2 ) 
{
    assert( c1 != space() || c2 != space() );
    if ( c1 == c2 ) {
        return +2;
    }
    return -1;
}
