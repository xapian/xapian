/************************************************************
 *
 *  code_to_html.h converts special characters in code to HTML.
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

#ifndef __CODE_TO_HTML_H__
#define __CODE_TO_HTML_H__

#include "virtual_ostream.h"
#include <string>
using std::string;

class code_to_html : public virtual_ostream {
private:
    const string & _line;
    unsigned int _width;
protected:
    ostream & show(ostream &) const;
public:
    code_to_html(const string & line, unsigned int width) : _line(line), _width(width) {}
};

#endif
