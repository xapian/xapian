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
 * Constructs a XML node <n>c</n>.
 * @param n the tag name of the node.
 * @param c the content of the node.
 **/
CXmlNode::CXmlNode(const string & n, const string & c) 
{
    _name = n;
    _content = c;
    _iCount++;
}

/**
 * Destructor.
 * 
 * Removes all the children, and delete all their memory
 * allocations.
 **/
CXmlNode::~CXmlNode() 
{
    CXmlNode *child;
    CXmlProperty *prop;

    // ----------------------------------------
    // delete all subelements.
    // ----------------------------------------
    for(child = _children.front(); 
        !_children.empty();
        child = _children.front()) 
    {
        delete child;
        _children.pop_front();
    }
    // ----------------------------------------
    // delete all attributes.
    // ----------------------------------------
    for(prop = _proplist.front(); 
        !_proplist.empty();
        prop = _proplist.front()) 
    {
        delete prop;
        _proplist.pop_front();
    }
    // ----------------------------------------
    // one object of this type destroyed.
    // _iCount more to go.
    // ----------------------------------------
    _iCount--;
}

/**
 * Modifies the tag name of the node.
 * 
 * @param n the new tag name.
 * @return the new tag name.
 **/
string
CXmlNode::name(const string & n) 
{
    return _name = n;
}

/**
 * Modifies the content of the node.
 * 
 * @param c the new content.
 * @return the new content.
 **/
string
CXmlNode::content(const string & c) 
{
    return _content = c;
}

/**
 * Gets a list of children below this node.
 * 
 * @param tags the relation of nodes to get and this node,
 * e.g. if tags = ["Name1", "Name2"], 
 * then we want to look for nodes of type thisNode/Name1/Name2.
 *
 * @return a list of children below this node that
 * that satisfy the relation specified by tags.
 **/
CXmlNodeList 
CXmlNode::children(const list<string> & tags) const
{
    CXmlNodeList result;
    if (tags.size() > 0)
    {
        string tag = tags.front();
        // ----------------------------------------
        // get nodes with tag names equal to the 
        // first tag
        // ----------------------------------------
        CXmlNodeList childrenNodes = children(tag);
        
        list<string> tags1 = tags;
        tags1.pop_front();
        if (tags1.size() == 0)
        {
            // ----------------------------------------
            // no more tags.
            // return what we have obtained.
            // ----------------------------------------
            return childrenNodes;
        }
        else
        {
            // ----------------------------------------
            // go through each childrenNode, and call
            // this function recursively with first
            // tag removed.
            // ----------------------------------------
            while (childrenNodes.size() > 0)
            {
                CXmlNode *pNode = childrenNodes.front();
                CXmlNodeList additional_result = pNode->children(tags1);
                result.splice(result.end(), additional_result);
                childrenNodes.pop_front();
            }
        }
    }
    return result;
}
/**
 * Gets a list of children directly below this node.
 * 
 * @param n only interested in children nodes of tag name n.
 * @return a list of children directly below this node that
 * has tag name n.
 **/
CXmlNodeList
CXmlNode::children(const string & n) const 
{
    CXmlNodeList retval;
    CXmlNodeConstIterator cur;

    if(n.length() == 0)
    {
        // ----------------------------------------
        // n == "", get everything.
        // ----------------------------------------
        return _children;
    }

    for(cur = _children.begin(); 
        cur != _children.end(); 
        cur++)
    {
        if((*cur)->name() == n)
        {
            retval.push_back(*cur);
        }
    }

    return retval;
}
/**
 * Adds a child node <n>c</n> to this node.
 * 
 * @param n the tag name of the new child node.
 * @param c the content of the new child node.
 *
 * @return the newly created child node pointer.
 **/
CXmlNode *
CXmlNode::add_child(const string & n, const string & c) 
{
    CXmlNode *tmp;
    tmp = new CXmlNode(n, c);
    return add_child(tmp);
}

/**
 * Moves all our child nodes to be under *pDestNode.
 * 
 * @param pDestNode the pointer to the destination parent node.
 * @return the destination parent node.
 **/
CXmlNode * 
CXmlNode::move_child(CXmlNode * pDestNode)
{
    if (!pDestNode)
    {
        return 0;
    }

    CXmlNode *pChild;
    for (pChild = _children.front();
         !_children.empty();
         _children.front())
    {
        pDestNode->add_child(pChild);
        _children.pop_front();
    }
    return pDestNode;
}

/**
 * Moves all our child nodes, attributes, tag, content to *pDestNode.
 * original tag, content, attributes with the same name as our attributes
 * will be overwritten.
 *
 * Existing child nodes in the destination node will not be replaced.
 *
 * @param pDestNode the pointer to the destination node.
 * @return the destination node.
 **/
CXmlNode * 
CXmlNode::move(CXmlNode *pDestNode)
{
    if (!pDestNode)
    {
        return 0;
    }
    move_child   (pDestNode);
    move_property(pDestNode);
    pDestNode->name(_name);
    pDestNode->content(_content);
    return pDestNode;
}

/**
 * Adds a child node.
 *
 * This node must not be a children of another XML node.
 * @param n the child node to add.
 * @return the added child node.
 **/
CXmlNode *
CXmlNode::add_child(CXmlNode *n) 
{
    if(!n)
    {
        return NULL;
    }
    _children.push_back(n);
    return n;
}
/**
 * Removes child nodes with tag n.
 *
 * @param n the tag name of nodes we want to remove.
 **/
void
CXmlNode::remove_child(const string & n) 
{
    CXmlNodeList tmp;
    CXmlNodeIterator cur;

    tmp = children(n);
    for(cur = tmp.begin(); 
        cur != tmp.end(); 
        cur++)
    {
        _children.remove(*cur);
    }
}
/**
 * Removes a child node.
 * It is now the client responsibility to delete the node n.
 *
 * @param n the pointer to the child node to remove.
 **/
void
CXmlNode::remove_child(CXmlNode *n) 
{
    if(n)
    {
        _children.remove(n);
    }
}
/**
 * Gets a list of attributes.
 *
 * @return a copy of attributes to this node.
 **/

CXmlPropertyList
CXmlNode::properties(void) const 
{
    return _proplist;
}
/**
 * Gets a attribute with name n.
 *
 * @param n the name of attribute to get.
 * @return a pointer to a CXmlProperty structure
 * that holds the attribute, there is no
 * need to delete this pointer, it will be
 * NULL if the node contains no attributes with name n.
 **/
CXmlProperty *
CXmlNode::property(const string & n) const
{
    if(_prophash.find(n) == _prophash.end())
    {
        return NULL;
    }
    return (*(_prophash.find(n))).second;
}
/**
 * Adds an attribute n->v to this node, or replace the existing
 * attribute with name n.
 *
 * @param n the name of the attribute to add.
 * @param v the value of the attribute to add.
 * 
 * @return the pointer to a CXmlProperty that stores (n->v).
 **/
CXmlProperty *
CXmlNode::add_property(const string & n, const string & v) 
{
    CXmlProperty *tmp;

    if(_prophash.find(n) != _prophash.end())
    {
        _prophash[n]->value(v);
        return _prophash[n];
    }

    tmp = new CXmlProperty(n, v);
    return add_property(tmp);
}


/**
 * Adds an attribute.
 *
 * @param p the pointer to the CXmlProperty to be added.
 * @return the added pointer to the added CXmlProperty.
 **/
CXmlProperty *
CXmlNode::add_property(CXmlProperty *p) 
{
    if(!p)
    {
        return NULL;
    }

    if(_prophash.find(p->name()) != _prophash.end())
    {
        delete p;
        return NULL;
    }

    _prophash[p->name()] = p;
    _proplist.push_back(p);

    return p;
}

/**
 * Removes an attribute.
 *
 * @param n the name of the attribute to delete.
 **/
void
CXmlNode::remove_property(const string & n) 
{
    if(_prophash.find(n) != _prophash.end()) 
    {
        _proplist.remove(_prophash[n]);
        _prophash.erase(n);
    }
}

/**
 * Removes an attribute.
 * It is now the client responsibility to delete the attribute p.
 * 
 * @param p the pointer containing the name of the attribute to remove.
 **/
void
CXmlNode::remove_property(CXmlProperty *p) 
{
    if(p)
    {
        remove_property(p->name());
    }
}

/**
 * Moves all the attributes from this node
 * to another node.
 *
 * @param pDestNode the pointer to the destination node.
 * @return the destination node.
 **/
CXmlNode *
CXmlNode::move_property(CXmlNode * pDestNode)
{
    if (!pDestNode)
    {
        return 0;
    }

    CXmlProperty *prop;
    for(prop = _proplist.front(); 
        !_proplist.empty();
        prop = _proplist.front()) 
    {
        pDestNode->add_property(prop);
        remove_property(prop);
    }
    return pDestNode;
}

/**
 * Tests to see if there is an attribute 
 * of name p->name(), value p->value().
 *
 * @param p the pointer to the CXmlProperty test object.
 * @return true if (p->name(), p->value()) is
 * an attribute of this node, false otherwise.
 **/
bool 
CXmlNode::has_property(CXmlProperty *p)
{
    if (!p)
    {
        return false;
    }
    return (has_property(p->name(), p->value()));
}

/**
 * Tests to see if name, value is an attribute of this node.
 * 
 * @param name the name of the attribute to test.
 * @param value the value of the attribute to test.
 * @return true if (name, value) is an attribute of this node,
 * false otherwise.
 **/
bool 
CXmlNode::has_property(const string & name, const string & value)
{
    CXmlProperty *p = property(name);
    if (p)
    {
        return (p->value() == value);
    }
    return false;
}
/**
 * Tests to see if name is an attribute name of this node.
 * 
 * @param name the name of the attribute to test.
 * @return true if name is an attribute name of this node,
 * false otherwise.
 **/
bool 
CXmlNode::has_property(const string & name)
{
    return (property(name) != 0);
}

int CXmlNode::_iCount = 0;

/**
 * Gets the first child with the given name. 
 **/
CXmlNode *
CXmlNode::get_child(const string & n) const 
{
    CXmlNodeList::const_iterator itr;
    for (itr = _children.begin(); itr!= _children.end(); ++itr) {
        if (!strcmp((*itr)->name().c_str(), n.c_str())) {
            return *itr;
        }
    }
    return 0;
}
