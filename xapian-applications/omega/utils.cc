/* utils.cc: string utils for omega
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
#include <string>
#include <vector>
#include <stdio.h>

using std::string;
using std::vector;

int
string_to_int(const string &s)
{
    return atoi(s.c_str());
}

string
int_to_string(int i)
{
    char buf[20];
    sprintf(buf, "%d", i);
    return string(buf);
}

vector<string>
split(const string &s, char at)
{
    size_t p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}

vector<string>
split(const string &s, const string &at)
{
    size_t p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find_first_of(at, p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}
