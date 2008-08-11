/************************************************************
 *
 *  diff_entry.cpp implementation.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
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

#include "range.h"
#include "diff_entry.h"
#include <assert.h>
#include <string>
using namespace std;

diff_entry::~diff_entry()
{
}

diff_entry::diff_entry(bool read)
    :_read_content(read)
{

}

diff_entry::diff_entry(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, char c, bool read) 
    :_read_content(read)
{
    diff_type type = e_none;
    switch (c) {
    case 'a':
        type = e_add;
        break;
    case 'd':
        type = e_delete;
        break;
    case 'c':
        type = e_change;
        break;
    default:
        break;
    }
    init(s1,s2,d1,d2,type);
}

diff_entry::diff_entry(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type, bool read)
    :_read_content(read)
{
    init(s1,s2,d1,d2,type);
}

void
diff_entry::init(unsigned int s1, unsigned int s2, unsigned int d1, unsigned d2, diff_type type)
{
    _type = type;
    try {
        // ----------------------------------------
        // strangely, diff produces result 2a3,4
        // cerr << s1 << " " << s2 << " " << d1 << " " << d2 << " " << type << endl;        
        // meaning insert after line 2
        switch (type)
        {
        case e_add:
            assert(s1 == s2);
            _src = range(s2);
            _dst = range(d1,d2);
            _src.begin_shift(1);
            break;
        case e_delete:
            assert(d1 == d2);
            _src = range(s1,s2);
            _dst = range(d2);
            _dst.begin_shift(1);
            break;
        case e_change:
            _src = range(s1,s2);
            _dst = range(d1,d2);
            break;
        case e_none:
            break;
        }
    } catch (range_exception & e) {
        cerr << e;
    }
    read_status(true);
}

istream & 
diff_entry::read(istream & is)
{
    string line;
    while (getline(is, line))
    {
        char c;

        if (line.length() == 0)
        {
            continue;
        }
        c = line[0];
        // ----------------------------------------
        // only interested in lines begin
        // with a digit
        // ----------------------------------------
        if (c > '9' || c < '0') 
        {
            continue;
        }

        bool get_begin = true;
        bool get_first = true;
        unsigned int s1 = 0, s2 = 0, d1 = 0, d2 = 0;
        diff_type type = e_change;
        for (unsigned int i = 0; i < line.length(); ++i)
        {
            c = line[i];
        
            if (0) {
            } else if ( c == ',') {
                // ----------------------------------------
                // this means any subsequent calls
                // is for getting the second number.
                // ----------------------------------------
                get_begin = false;
            } else if ( c == e_change || c == e_add || c == e_delete) {
                // ----------------------------------------
                // getting the diff type
                // ----------------------------------------
                type = (diff_type) c;
                if (get_begin) {
                    s2 = s1;
                }
                // ----------------------------------------
                // getting the second range
                // ----------------------------------------
                get_begin = true;
                get_first = false;
            } else if (c > '9' || c < '0') {
            } else if ( get_begin &&  get_first) {
                s1 *= 10;
                s1 += c - '0';
            } else if (!get_begin &&  get_first) {
                s2 *= 10;
                s2 += c - '0';
            } else if ( get_begin && !get_first) {
                d1 *= 10;
                d1 += c - '0';
            } else if (!get_begin && !get_first) {
                d2 *= 10;
                d2 += c - '0';
            }
        }
        if (get_begin) {
            d2 = d1;
        }
        init(s1,s2,d1,d2,type);

        if (_type == e_change && _read_content) 
        {
            string subline;
            for (unsigned int i = 0; is && i < _src.size(); ++i)
            {
                getline(is, line);
                if (line.length() && line[0] == '<')
                {
                    subline = line.substr(2);
                    _src_lines.push_back(subline);
                }
            }

            // ----------------------------------------
            // get separator
            // ----------------------------------------
            do {
                getline(is,line);
            } while (is && (line.length() == 0 || line[0] != '-'));
            
            for (unsigned int i = 0; is && i < _dst.size(); ++i)
            {
                getline(is,line);
                if (line.length() && line[0] == '>')
                {
                    subline = line.substr(2);
                    _dst_lines.push_back(subline);
                }
            }
        }
        break;
    }
    return is;
}

ostream & 
diff_entry::show(ostream & os) const
{
    if (read_status())
    {
        switch (_type)
        {
        case e_add:
            os << _src.begin()-1 << "a" << _dst;
            break;
        case e_change:
            os << _src << "c" << _dst;
            break;
        case e_delete:
            os << _src << "d" << _dst.begin()-1;
            break;
        case e_none:
            break;
        }
    }
    return os;
}

int
diff_entry::size() const
{
    switch (_type) 
    {
    case e_add:
        return _dst.size();
        break;
    case e_change:
        return _dst.size() - _src.size();
        break;
    case e_delete:
        return -_src.size();
        break;
    case e_none:
        return 0;
        break;
    }
    return 0;
}

bool
diff_entry::operator==(const diff_entry & r) const
{
    if (_src != r.source()) 
    {
        return false;
    }
    
    if (_dst != r.dest())
    {
        return false;
    }

    if (_type != r.type())
    {
        return false;
    }

    return true;
}
