/************************************************************
 *
 *  xmlTree.h A class stores an XML Document.
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

#ifndef __XML_TREE_H
#define __XML_TREE_H

// ----------------------------------------
// added to remove a silly warning generated
// by VC++.
// ----------------------------------------
#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#include "xmlNode.h"

/**
 * A XML Tree structure.
 * This class stores a XML document.
 *
 * @author  Andrew Yao (andrewy@cse.unsw.edu.au)
 **/
class CXmlTree 
{
private:
    /// initialisation flag.
    bool _initialized;
    /// the document filename.
    string _filename;
    /// the root node.
    CXmlNode *_root;
    /// compression level if zlib is installed.
    int _compression;
    /// total number of allocated CXmlTree objects.
    static int _iCount;
    /// the name space.
	string _namespace;

protected:
    /**
     * Gets the compression level, it is always zero meaning no compression.
     *
     * @return the compression level.
     **/
    int compression() const { return _compression; };

public:
    CXmlTree();

    CXmlTree(const string & fn);

    virtual ~CXmlTree();

    /**
     * Gets the initialization flag.
     *
     * @return true if the document is initialized, false otherwise.
     **/
    bool initialized() { return _initialized; };

    /**
     * Gets the root node.
     * 
     * @return the root node of the document, may be NULL.
     **/
    CXmlNode *root() const { return _root; };


    /**
     * Gets the filename of the document.
     *
     * @return the filename of the document.
     **/
    string filename() const { return _filename; };

    /**
     * Sets the filename of the document.
     *
     * @param fn the new filename of the document.
     * @return the new filename of the document.
     **/
    string filename(string & fn) { return _filename = fn; };

    /** 
     * Sets the root node.
     * 
     * @param n the new root node.
     * @return the new root node.
     **/
    CXmlNode *root(CXmlNode *n) { return _root = n; };

    bool read();

    bool write() const;

    bool read(const string & fn);

    bool write(const string & fn);

    /**
     * Gets the name space.
     **/
	string defaultnamespace() {return _namespace;}
    
    /**
     * Sets the name space.
     *
     * @param n the new name space.
     * @return the new name space.
     **/
	string defaultnamespace(const string & n) {return _namespace = n;}

    CXmlNodeList children(const list<string> & tags) const;

    /**
     * Obtains the number of objects of this type still in memory, 
     * used for memory-leak detection.
     * 
     * @return the total number of allocated CXmlTree objects.
     **/
    static int count() {return _iCount;}

};
#endif

