/************************************************************
 *
 *  xmlNode.cpp CXmlNode implementation.
 *
 *  libxml++ and this file are copyright (C) 2000 by Ari Johnson, and
 *  are covered by the GNU General Public License, which should be
 *  included with libxml++ as the file COPYING.
 *
 *  modified by Andrew Yao (andrewy@cse.unsw.edu.au)
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

#include <libxml/parser.h>
#include "xml++.h"
#include <iostream>

using std::cout;
using std::endl;

/**
 * Constructor.
 * 
 * Constructs an attribute.
 * @param n the name of the attribute.
 * @param c the value of the attribute.
 **/
CXmlProperty::CXmlProperty(const string & n, const string & v) 
    :_name(n),
     _value(v) 
{
    _iCount++;
}

/**
 * Destructor.
 *
 **/
CXmlProperty::~CXmlProperty() 
{
    _iCount--;
}

/**
 * Sets the value of the attribute.
 * 
 * @param v the new attribute value.
 * @return the new attribute value.
 **/
string
CXmlProperty::value(const string & v) 
{
    return _value = v;
}

/**
 * Obtains the number of objects of this type still in memory, 
 * used for memory-leak detection.
 * 
 * @return the total number of allocated CXmlProperty objects.
 **/
int CXmlProperty::_iCount = 0;
