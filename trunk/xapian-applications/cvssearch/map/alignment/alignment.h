/************************************************************
 *
 *  alignment.h amir's alignment algorithm.
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

#ifndef __ALIGNMENT_H__
#define __ALIGNMENT_H__

#include "virtual_ostream.h"
#include "diff_entry.h"

#include <vector>
#include <list>
#include <iostream>
#include <string>
using std::vector;
using std::ostream;
using std::list;
using std::string;

template<class T>
class alignment : public virtual_ostream
{
private:
    const T &S;
    const T &D;
    vector< vector< int > > V;
    list<diff_entry> _entries;
    unsigned int _source_offset;
    unsigned int _dest_offset;
    ostream & diff_output(ostream &, 
                          unsigned int s1,
                          unsigned int s2,
                          unsigned int d1,
                          unsigned int d2,
                          diff_type type) const;
protected:
    ostream & show(ostream &) const;
public:
    alignment(const T &s, const T &d, unsigned int source_offset = 0, unsigned int dest_offset = 0);
    void find_optimal_alignment(bool = true);
    int optimal_alignment_value() const;

    const list<diff_entry> & entries() const {return _entries;}
    ostream & dump (ostream &) const;
    ostream & diff1(ostream &) const;
    ostream & diff2(ostream &) const;                          
                        
};

#include "alignment.hpp"
#endif
