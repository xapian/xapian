/************************************************************
 *
 *  collection.h a very simple base class for storing a vector 
 *  of objects.
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
 *  Usage:
 *  
 *  See diff/diff.h for an example.
 * 
 *  $Id$
 * 
 ************************************************************/

#ifndef __COLLECTION_H__
#define __COLLECTION_H__

#include <vector>
using std::vector;

template<class T>
class collection
{
protected:
    vector<T> _entries;
public:
    collection() {}
    collection(const vector<T> & entries) : _entries(entries) {}
    unsigned int size() const {  return _entries.size(); }
    const T & operator[](unsigned int i) const { assert (i < size()); return _entries[i]; }
          T & operator[](unsigned int i)       { assert (i < size()); return _entries[i]; }
    void add(const T & entry)                  { _entries.push_back(entry);}
};

#endif
