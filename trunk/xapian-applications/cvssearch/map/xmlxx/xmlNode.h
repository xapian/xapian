/************************************************************
 *
 *  xmlNode.h A class stores an XML Node.
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

#ifndef __XML_NODE_H
#define __XML_NODE_H

// ----------------------------------------
// added to remove a silly warning generated
// by VC++.
// ----------------------------------------
#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include <string>
#include <list>
#include <map>

using std::list;
using std::map;
using std::string;

/*  struct hash<string> { */
/*    size_t operator()(const string &s) const { */
/*      return __stl_hash_string(s.c_str()); */
/*    }; */
/*  }; */

class CXmlNode;
typedef list<CXmlNode *> CXmlNodeList;
typedef CXmlNodeList::iterator CXmlNodeIterator;
typedef CXmlNodeList::const_iterator CXmlNodeConstIterator;

class CXmlProperty;
typedef list<CXmlProperty *> CXmlPropertyList;
typedef CXmlPropertyList::iterator CXmlPropertyIterator;
typedef CXmlPropertyList::const_iterator CXmlPropertyConstIterator;
typedef map<string, CXmlProperty *> CXmlPropertyHash;


/**
 * A XML Node structure.
 * This class stores a XML node using DOM.
 * 
 * @author  Andrew Yao (andrewy@cse.unsw.edu.au)
 **/
class CXmlNode 
{
private:
    /// the tag part of the node.
    string _name;
    /// the content part of the node.
    string _content;
    /// a list of child nodes.
    CXmlNodeList _children;
    /// a list of attributes.
    CXmlPropertyList _proplist;
    /// a hashtable of attributes.
    CXmlPropertyHash _prophash;
    /// total number of allocated CXmlNode objects.
    static int _iCount;

protected:
    CXmlProperty *add_property(CXmlProperty * p);
    bool has_property(CXmlProperty * p);
    void remove_property(CXmlProperty * p);

public:
    CXmlNode(const string & n, const string & c = "");
    virtual ~CXmlNode();
    
    /**
     * gets the tag name of the node.
     *
     * @return the tag name.
     **/
    string name(void) const { return _name; };
    string name(const string & n);

    /**
     *gets the content of the node.
     **/
    string content(void) const { return _content; } ;
    string content(const string & c);

    CXmlNodeList children(const list<string> & tags) const;
    CXmlNodeList children(const string & n = "") const;
    
    CXmlNode *get_child(const string & n) const ;
    CXmlNode *add_child(const string & n, const string & c = "");
    CXmlNode *add_child(CXmlNode *n);

    void remove_child(CXmlNode * p);
    void remove_child(const string & n = "");

    CXmlPropertyList properties() const;
    CXmlProperty *property(const string & n) const;

    CXmlProperty *add_property(const string & n, const string & v= "");
    void remove_property(const string & n= "");

    CXmlNode * move_child(CXmlNode * pDestNode);
    CXmlNode * move_property(CXmlNode * pDestNode);
    CXmlNode * move(CXmlNode * pDestNode);

    bool has_property(const string & name, const string & value);
    bool has_property(const string & name);

    /**
     * obtains the number of objects of this type still in memory, 
     * used for memory-leak detection.
     * 
     * @return the total number of allocated CXmlNode objects.
     **/
    static int count() {return _iCount;}
};

#endif
