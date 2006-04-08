/************************************************************
 *
 *  xmlTree.cpp CXmlTree implementation.
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

static CXmlNode *readnode(xmlNodePtr);
static void writenode(xmlDocPtr, CXmlNode *, xmlNodePtr, const string & _namespace, int = 0);

/**
 * Constructor.
 * Constructs a blank XML document.
 **/
CXmlTree::CXmlTree() 
{
    _filename = "";
    _root = NULL;
    _compression = 0;
    _iCount++;
}

/**
 * Constructor.
 * Constructs a XML document from a file.
 *
 * @param fn the name of the file.
 **/
CXmlTree::CXmlTree(const string & fn) 
{
    _filename = fn;
    _root = NULL;
    _compression = 0;
    read();
    _iCount++;
}

/**
 * Destructor.
 **/
CXmlTree::~CXmlTree() 
{
    if(_root)
    {
        delete _root;
    }
    _iCount--;
}

/**
 * Reads from the filename specified by _filename.
 *
 * @return true if reading is success, false otherwise.
 **/
bool
CXmlTree::read() 
{
    xmlDocPtr doc;

    if(_root)
    {
        delete _root;
    }

    xmlKeepBlanksDefault(0);

    doc = xmlParseFile(_filename.c_str());
    if(!doc) 
    {
        _initialized = false;
        return false;
    }
    xmlNsPtr ns = xmlSearchNs(doc, doc->children, 0);
    if (ns)
    {
        _namespace = string((char *)ns->href);
    }
    _root = readnode(xmlDocGetRootElement(doc));
    xmlFreeDoc(doc);
    _initialized = true;

    return true;
}

/**
 * Reads the document from a file.
 *
 * @param fn where to read the document from.
 * @return true if success, false otherwise.
 **/
bool
CXmlTree::read(const string & fn) 
{
    _filename = fn;

    return read();
}

/**
 * Writes the document to _filename.
 * 
 * @return true if success.
 **/
bool CXmlTree::write(void) const
{
    xmlDocPtr doc;

    CXmlNodeList children;

    xmlKeepBlanksDefault(0);
    doc = xmlNewDoc((const unsigned char *) "1.0");

    xmlSetDocCompressMode(doc, _compression);
    writenode(doc, _root, doc->children, _namespace, 1);
    xmlSaveFile(_filename.c_str(), doc);
    xmlFreeDoc(doc);
    return true;
}

/**
 * write the document to a file.
 *
 * @param fn filename of the output file.
 * @return true if success.
 **/
bool CXmlTree::write(const string & fn) 
{
    _filename = fn;
    return write();
}

/**
 * Gets node whose location in the document is specified
 * by tags.
 *
 * @param tags specify the location to get nodes from.
 * e.g. if tags = ["Node1", "Node2"], will get all nodes
 * of type /Node1/Node2.
 * 
 * @return the list of nodes each is of type /Node1/Node2.
 **/
CXmlNodeList 
CXmlTree::children(const list<string> & tags) const
{
    CXmlNodeList result;
    if (tags.size() > 0 && _root)
    {
        string tag = tags.front();
        if (tag == _root->name())
        {
            list<string> tags1 = tags;
            tags1.pop_front();
            return _root->children(tags1);
        }
    }
    return result;
}




static CXmlNode *readnode(xmlNodePtr node) 
{
    string name, content;
    xmlNodePtr child;
    CXmlNode *tmp;
    xmlAttrPtr attr;

    name = (char *) node->name;
    tmp = new CXmlNode(name);

    for(attr = node->properties; 
        attr; 
        attr = attr->next) 
    {
        name = (char *) attr->name;
        content = "";
        if(attr->children)
            content = (char *) attr->children->content;
        tmp->add_property(name, content);
    }

    for(child = node->children; 
        child; 
        child = child->next) 
    {
        if(xmlNodeIsText(child)) 
        {
            content = (char *) child->content;
            tmp->content(content);
        }
        else 
        {
            tmp->add_child(readnode(child));
        }
    }

    return tmp;
}

static void writenode(xmlDocPtr doc, CXmlNode *n, xmlNodePtr p, const string & _namespace, int root) 
{
    CXmlNodeList children;
    CXmlNodeIterator curchild;
    CXmlPropertyList props;
    CXmlPropertyIterator curprop;
    xmlNodePtr node;

    if(root)
    {
        node = doc->children = xmlNewDocNode(doc, 0,
                                             (const unsigned char *) n->name().c_str(),
                                             (const unsigned char *) n->content().c_str());
        xmlNewNs(node, (const unsigned char *) _namespace.c_str(), 0);
    }
    else
    {
        node = xmlNewChild(p, 0, (const unsigned char *) n->name().c_str(),
                           (const unsigned char *) n->content().c_str());
    }
    props = n->properties();
    for(curprop = props.begin(); 
        curprop != props.end(); 
        curprop++)
    {
        xmlSetProp(node, (const unsigned char *) (*curprop)->name().c_str(),
                   (const unsigned char *) (*curprop)->value().c_str());
    }

    children = n->children();
    for(curchild = children.begin(); 
        curchild != children.end(); 
        curchild++)
    {
        writenode(doc, *curchild, node, _namespace );
    }
}


int CXmlTree::_iCount = 0;

