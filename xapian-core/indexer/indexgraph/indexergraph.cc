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
#include "om/omerror.h"
#include <parser.h>  // libxml
#include <algorithm>

class OmIndexerStartNode : public OmIndexerNode
{
    public:
	static OmIndexerNode *create(const OmSettings &) {
	    return new OmIndexerStartNode();
	}

	void set_message(Message msg) {
	    //cout << "Setting message:" << msg << endl;
	    message = msg;
	};
    private:
	void calculate() {
	    set_output_record("out", *message);
	    Message temp(0);
	    message = temp;
	}

	Message message;
};

OmIndexer::OmIndexer()
{
}

OmIndexer::~OmIndexer()
{
}

auto_ptr<OmIndexer>
OmIndexerBuilder::build_from_file(std::string filename)
{
    xmlDocPtr doc = get_xmltree_from_file(filename);

    auto_ptr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), doc);

    return indexer;
}

auto_ptr<OmIndexer>
OmIndexerBuilder::build_from_string(std::string filename)
{
    xmlDocPtr doc = get_xmltree_from_string(filename);

    auto_ptr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), doc);

    return indexer;
}

xmlDocPtr
OmIndexerBuilder::get_xmltree_from_file(const std::string &filename)
{
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    // FIXME: Check the validity of the document
    if (!doc) {
	throw "Invalid document";
    }

    return doc;
}

xmlDocPtr
OmIndexerBuilder::get_xmltree_from_string(const std::string &xmldesc)
{
    xmlDocPtr doc = xmlParseMemory(const_cast<char *>(xmldesc.c_str()),
				   xmldesc.length());

    // FIXME: Check the validity of the document
    if (!doc) {
	throw "Invalid Document";
    }

    return doc;
}

OmIndexerNode *
OmIndexerBuilder::make_node(const std::string &type)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(type);
    if (i == nodetypes.end()) {
	throw OmInvalidArgumentError(std::string("Unknown node type ") + type);
    } else {
	return i->second.create(OmSettings());
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
OmIndexerBuilder::build_graph(OmIndexer *indexer, xmlDocPtr doc)
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
    std::map<std::string, std::string> types;
    
    indexer->nodemap["START"] = make_node("START");
    indexer->start = dynamic_cast<OmIndexerStartNode *>(indexer->nodemap["START"]);
    types["START"] = "START";
    
    for (xmlNodePtr node = root->childs;
	 node != 0;
	 node = node->next) {
	std::string type = (char *)node->name;
	// cout << "node name = " << type << endl;
	if (type == "node") {
	    std::map<std::string, std::string> node_attrs(attr_to_map(node->properties));
	    if (indexer->nodemap.find(node_attrs["id"]) != indexer->nodemap.end()) {
		throw "Duplicate node id!";
	    }
	    OmIndexerNode *newnode = make_node(node_attrs["type"]);
	    types[node_attrs["id"]] = node_attrs["type"];
	    indexer->nodemap[node_attrs["id"]] = newnode;
	    // connect the inputs
	    for (xmlNodePtr input = node->childs;
		 input != 0;
		 input = input->next) {
		if (std::string((char *)input->name) != "input") {
		    throw "Expected input";
		}
		std::map<std::string, std::string> input_attrs(attr_to_map(input->properties));
		OmIndexer::NodeMap::const_iterator i =
			indexer->nodemap.find(input_attrs["node"]);
		if (i == indexer->nodemap.end()) {
		    throw "input node not found";
		}
		// typecheck throws on an error
		typecheck(node_attrs["type"], // this node's type
			  input_attrs["name"],// this node's input name
			  types[input_attrs["node"]], // the input node's type
			  input_attrs["out_name"]);  // the input node's output
		newnode->connect_input(input_attrs["name"],
				       i->second,
				       input_attrs["out_name"]);
	    }
	} else if (type == "output") {
	    std::map<std::string, std::string> attrs(attr_to_map(node->properties));
	    OmIndexer::NodeMap::const_iterator i = indexer->nodemap.find(attrs["node"]);
	    if (i == indexer->nodemap.end()) {
		cerr << "bad node" << endl;
		throw "bad node";
	    }
	    indexer->final = i->second;
	} else {
	    throw "Unknown tag";
	}
    }
}

void
OmIndexerBuilder::typecheck(const std::string &receivertype,
			    const std::string &receiverin,
			    const std::string &sendertype,
			    const std::string &senderout)
{
    NodeConnection sendercon = get_outputcon(sendertype, senderout);
    NodeConnection receivercon = get_inputcon(receivertype, receiverin);

    // First check that the physical type is compatible
    if (sendercon.phys_type != receivercon.phys_type) {
	if (sendercon.phys_type == mt_record ||
	    receivercon.phys_type == mt_record) {
	    throw OmUnimplementedError("Automatic type conversions not implemented, so rejecting.");
	} else {
	    throw OmInvalidArgumentError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not physically compatible.");
	}
    } else if (sendercon.type != receivercon.type) {
	    throw OmInvalidArgumentError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not compatible.");
    }
}

OmIndexerBuilder::NodeConnection
OmIndexerBuilder::get_outputcon(const std::string &nodetype,
				const std::string &output_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidArgumentError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<NodeConnection>::const_iterator i;
    for (i=type->second.outputs.begin(); i!= type->second.outputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == output_name) {
	    return *i;
	}
    }
    throw OmInvalidArgumentError(std::string("Invalid output connection ") +
				 nodetype + "[" + output_name + "]");
}

OmIndexerBuilder::NodeConnection
OmIndexerBuilder::get_inputcon(const std::string &nodetype,
			       const std::string &input_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidArgumentError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<NodeConnection>::const_iterator i;
    for (i=type->second.inputs.begin(); i!= type->second.inputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == input_name) {
	    return *i;
	}
    }
    throw OmInvalidArgumentError(std::string("Invalid input connection ") +
				 nodetype + "[" + input_name + "]");
}

Message
OmIndexer::get_output()
{
    return final->get_output_record("out");
}

void
OmIndexer::set_input(Message msg)
{
    start->set_message(msg);
}

OmIndexerBuilder::OmIndexerBuilder()
{
    std::vector<NodeConnection> outputs;
    outputs.push_back(OmIndexerBuilder::NodeConnection("out",
						       "mystr",
						       mt_record));
    register_node_type("START",
		       &OmIndexerStartNode::create,
		       std::vector<NodeConnection>(),
		       outputs);
}

void
OmIndexerBuilder::register_node_type(const std::string &nodename,
				     NodeCreator create,
				     const std::vector<NodeConnection> &inputs,
				     const std::vector<NodeConnection> &outputs)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(nodename);
    if (i != nodetypes.end()) {
	throw OmInvalidArgumentError(string("Attempt to register node type ")
				     + nodename + ", which already exists.");
    }

    node_desc ndesc;
    ndesc.create = create;
    std::copy(inputs.begin(), inputs.end(),
	      std::back_inserter(ndesc.inputs));
    std::copy(outputs.begin(), outputs.end(),
	      std::back_inserter(ndesc.outputs));

    nodetypes[nodename] = ndesc;
}
