/************************************************************
 *
 *  line_sequence.h is used to compare lines.
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

#ifndef __LINE_SEQUENCE_H__
#define __LINE_SEQUENCE_H__

#include "sequence.h"
#include <string>
#include <iostream>
using std::string;
using std::vector;
using std::istream;

class line_sequence : public sequence<string>
{
protected:
    static string trim(const string & s);
public:
    line_sequence(istream & is);
    line_sequence(const string & filename);
    line_sequence(const vector<string> & entries) : sequence<string>(entries) {}
    friend istream & operator >>(istream & in, line_sequence & r);
    static int score(const string & l1, const string & l2 );
    static string space();
};

#endif
