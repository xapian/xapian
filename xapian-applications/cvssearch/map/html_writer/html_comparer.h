/************************************************************
 *
 *  html_comparer.h is a class for comparing two versions and
 *  displays using HTML.
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

#ifndef __HTML_COMPARER_H__
#define __HTML_COMPARER_H__

#include "virtual_ostream.h"
#include "html_writer.h"
#include "cvs_revision.h"
#include "process.h"
#include "diff.h"
#include <vector>
using std::vector;
#include <set>
using std::set;

class html_comparer : public virtual_ostream 
{
private:
    process * p0;
    process * p1;
    process * p2;
    istream * pis0;
    istream * pis1;
    istream * pis2;
    
    const vector<unsigned int> & _input1;
    const vector<unsigned int> & _input2;    
    const string _revision0;
    const string _revision1;
    const string _revision2;
    const string & _filename;
    const string & _pathname;
    diff _diff;
    bool _short;
    unsigned int _width;
    unsigned int _diff_index;
    ostream & show  (ostream &) const;
    ostream & write (ostream &) const;
    void 
    write_line(ostream & os, 
               string & select0, unsigned int & index0, bool do0,
               string & select1, unsigned int & index1, bool do1,
               string & select2, unsigned int & index2, bool do2,
               unsigned int & diff_index) const;
    
    void get_class_type (string & select0, unsigned int index0, bool & do0,
                         string & select1, unsigned int index1, bool & do1,
                         string & select2, unsigned int index2, bool & do2,
                         unsigned int & diff_index) const;

public:
    html_comparer(const vector<unsigned int> & input1, 
                  const vector<unsigned int> & input2,
                  const string & filename, 
                  const string & pathname,
                  const string & revision,
                  const string & revision1, 
                  const string & revision2,
                  const diff & diff, 
                  bool short_output,
                  unsigned int col_width = 40);
    ~html_comparer();
};

#endif
