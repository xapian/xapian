/* indexergraph.cc: An indexing structure built from an XML definition
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include "omindexernode.h"
#include "indexergraph.h"
#include "testnodes.h"
#include <parser.h>  // libxml

OmIndexer::OmIndexer(std::string filename)
{
    xmlDocPtr doc = get_xmltree(filename);

    build_graph(doc);
}

OmIndexer::~OmIndexer()
{
}

xmlDocPtr
OmIndexer::get_xmltree(const std::string &filename)
{
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    // FIXME: Check the validity of the document
    if (!doc) {
	throw "bad";
    }

    return doc;
}

OmIndexerNode *
make_node(const std::string &type)
{
    // FIXME: have a runtype-addable map
    if (type == "reverse") {
	return new ReverseNode();
    } else if (type == "split") {
	return new SplitNode();
    } else if (type == "concat") {
	return new ConcatNode();
    } else {
	throw "bad node type";
    }
}

// FIXME: handle the unicode stuff rather than char *
std::map<std::string, std::string> attr_to_map(xmlAttrPtr attr)
{
    std::map<std::string, std::string> result;
    while (attr) {
	std::string name = (char *)attr->name;
	xmlNodePtr val = attr->val;
	std::string value = (char *)val->content;
	//cout << "\tAttr " << name << "=" << value << endl;

	result[name] = value;

	attr = attr->next;
    }
    return result;
}

void
OmIndexer::build_graph(xmlDocPtr doc)
{
    xmlNodePtr root = doc->root;
    if (!root) {
	throw "no root";
    }
    std::string rootname = (char *)root->name;
    if (rootname != "omindexer") {
	throw "Bad root tag";
    }
    //cout << "root name is " << rootname << endl;

    {
	Message origmsg(new BasicMessage());
	origmsg->name = "foo";
	origmsg->value = "bar";
	nodemap["START"] = new OmOrigNode(origmsg);
    }
    
    for (xmlNodePtr node = root->childs;
	 node != 0;
	 node = node->next) {
	std::string type = (char *)node->name;
	// cout << "node name = " << type << endl;
	if (type == "node") {
	    std::map<std::string, std::string> attrs(attr_to_map(node->properties));
	    OmIndexerNode *newnode = make_node(attrs["type"]);
	    nodemap[attrs["id"]] = newnode;
	    // connect the inputs
	    for (xmlNodePtr input = node->childs;
		 input != 0;
		 input = input->next) {
		if (std::string((char *)input->name) != "input") {
		    throw "Expected input";
		}
		std::map<std::string, std::string> attrs(attr_to_map(input->properties));
		NodeMap::const_iterator i = nodemap.find(attrs["node"]);
		if (i == nodemap.end()) {
		    throw "input node not found";
		}
		newnode->connect_input(attrs["name"],
				       i->second,
				       attrs["out_name"]);
	    }
	} else if (type == "output") {
	    std::map<std::string, std::string> attrs(attr_to_map(node->properties));
	    NodeMap::const_iterator i = nodemap.find(attrs["node"]);
	    if (i == nodemap.end()) {
		cerr << "bad node" << endl;
		throw "bad node";
	    }
	    final = i->second;
	} else {
	    throw "Unknown tag";
	}
    }
}

Message
OmIndexer::get_output()
{
    return final->get_output("out");
}
