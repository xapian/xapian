/************************************************************
 *
 *  xmlProperty.h A class stores an XML Attribute.
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

#ifndef __XML_PROPERTY_H
#define __XML_PROPERTY_H

// ----------------------------------------
// added to remove a silly warning generated
// by VC++.
// ----------------------------------------
#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <string>
using std::string;


/**
 * A XML attribute structure.
 * This class stores a XML attribute.
 * 
 * @author  Andrew Yao (andrewy@cse.unsw.edu.au)
 **/
class CXmlProperty 
{
private:
    /// the attribute name.
    string _name;
    /// the attribute value.
    string _value;
    /// total number of allocated CXmlProperty objects.
    static int _iCount;

public:

    CXmlProperty(const string & n, const string & c = "");
    virtual ~CXmlProperty();

    /**
     * Gets the name of the attribute.
     * @return the name of the attribute.
     **/
    string name(void) const { return _name; };
    
    /**
     * Gets the value of the attribute.
     * @return the value of the attribute.
     **/
    string value(void) const { return _value; };
    string value(const string & v);
    static int count() {return _iCount;}
};

#endif

