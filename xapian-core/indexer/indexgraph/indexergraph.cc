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
#include "indexerxml.h"
#include "om/omerror.h"
#include "register_core.h"
#include <algorithm>

class OmIndexerStartNode : public OmIndexerNode
{
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new OmIndexerStartNode(settings);
	}

	void set_message(OmIndexerMessage msg) {
	    //cout << "Setting message:" << msg << endl;
	    message = msg;
	};
    private:
	OmIndexerStartNode(const OmSettings &settings)
		: OmIndexerNode(settings) {}
	void calculate() {
	    set_output("out", message);
	}

	OmIndexerMessage message;
};

OmIndexer::OmIndexer()
{
}

OmIndexer::~OmIndexer()
{
}

AutoPtr<OmIndexer>
OmIndexerBuilder::build_from_file(std::string filename)
{
    AutoPtr<OmIndexerDesc> doc = desc_from_xml_file(filename);

    AutoPtr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), *doc);

    return indexer;
}

AutoPtr<OmIndexer>
OmIndexerBuilder::build_from_string(std::string filename)
{
    AutoPtr<OmIndexerDesc> doc = desc_from_xml_string(filename);

    AutoPtr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), *doc);

    return indexer;
}

OmIndexerNode *
OmIndexerBuilder::make_node(const std::string &type,
			    const OmSettings &config)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(type);
    if (i == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") + type);
    } else {
	return i->second.create(config);
    }
}

typedef std::map<std::string, std::string> typemap;

void
OmIndexerBuilder::build_graph(OmIndexer *indexer,
			      const OmIndexerDesc &desc)
{
    typemap types;
    
    indexer->nodemap["START"] = make_node("START", OmSettings());
    indexer->start = dynamic_cast<OmIndexerStartNode *>(indexer->nodemap["START"]);
    types["START"] = "START";

    std::vector<OmIndexerDesc::NodeInstance>::const_iterator node;
    for (node = desc.nodes.begin();
	 node != desc.nodes.end();
	 ++node) {

	if (indexer->nodemap.find(node->id) != indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Duplicate node id ")
					 + node->id);
	}

	OmIndexerNode *newnode = make_node(node->type, node->param);
	types[node->id] = node->type;
	indexer->nodemap[node->id] = newnode;

	// connect the inputs
	std::vector<OmIndexerDesc::Connect>::const_iterator input;
	for (input = node->input.begin();
	     input != node->input.end();
	     ++input) {
	    OmIndexer::NodeMap::const_iterator i =
		    indexer->nodemap.find(input->feeder_node);
	    if (i == indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Input node ") +
					 input->feeder_node + " not found");
	    }

	    // typecheck throws on an error
	    typecheck(node->type, // this node's type
		      input->input_name,// this node's input name
		      types[input->feeder_node], // the input node's type
		      input->feeder_out);  // the input node's output
	    newnode->connect_input(input->input_name,
				   i->second,
				   input->feeder_out);
	}
    }

    /* connect the output of the whole graph */
    OmIndexer::NodeMap::const_iterator i =
	    indexer->nodemap.find(desc.output_node);
    if (i == indexer->nodemap.end()) {
	throw OmInvalidDataError(std::string("Unknown output node ") +
				 desc.output_node);
    }
    indexer->final = i->second;
    indexer->final_out = desc.output_conn;
}

void
OmIndexerBuilder::typecheck(const std::string &receivertype,
			    const std::string &receiverin,
			    const std::string &sendertype,
			    const std::string &senderout)
{
    OmNodeConnection sendercon = get_outputcon(sendertype, senderout);
    OmNodeConnection receivercon = get_inputcon(receivertype, receiverin);

    // First check that the physical type is compatible
    if (sendercon.phys_type != receivercon.phys_type) {
	if (sendercon.phys_type == mt_record ||
	    receivercon.phys_type == mt_record) {
	    // this is ok - things can be converted (up to a point)
	    // between records and primitives.
	} else {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not physically compatible.");
	}
    } else if (sendercon.type != receivercon.type) {
	if (receivercon.type != "ANY" && sendercon.type != "ANY") {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not compatible.");
	}
	// else at least one of them is "universal"
    }
}

OmNodeConnection
OmIndexerBuilder::get_outputcon(const std::string &nodetype,
				const std::string &output_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodeConnection>::const_iterator i;
    for (i=type->second.outputs.begin(); i!= type->second.outputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == output_name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Invalid output connection ") +
				 nodetype + "[" + output_name + "]");
}

OmNodeConnection
OmIndexerBuilder::get_inputcon(const std::string &nodetype,
			       const std::string &input_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodeConnection>::const_iterator i;
    for (i=type->second.inputs.begin(); i!= type->second.inputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == input_name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Invalid input connection ") +
				 nodetype + "[" + input_name + "]");
}

OmIndexerMessage
OmIndexer::get_output()
{
    return final->get_output_record(final_out);
}

void
OmIndexer::set_input(OmIndexerMessage msg)
{
    start->set_message(msg);
}

void
OmIndexer::set_node_config(const std::string &node_id,
			   const std::string &key,
			   const std::string &value)
{
    NodeMap::iterator i = nodemap.find(node_id);
    if (i == nodemap.end()) {
	throw OmInvalidDataError(std::string("Node id ") + node_id +
				 " doesn't exist");
    }
    i->second->set_config_string(key, value);
}

OmIndexerBuilder::OmIndexerBuilder()
{
    OmNodeDescriptor ndesc("START", &OmIndexerStartNode::create);
    ndesc.add_output("out", "mystr", mt_record);
    register_node_type(ndesc);

    register_core_nodes(*this);
}

void
OmIndexerBuilder::register_node_type(const OmNodeDescriptor &ndesc_)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(ndesc_.nodename);
    if (i != nodetypes.end()) {
	throw OmInvalidArgumentError(string("Attempt to register node type ")
				     + ndesc_.nodename + ", which already exists.");
    }

    node_desc ndesc;
    ndesc.create = ndesc_.creator;
    std::copy(ndesc_.inputs.begin(), ndesc_.inputs.end(),
	      std::back_inserter(ndesc.inputs));
    std::copy(ndesc_.outputs.begin(), ndesc_.outputs.end(),
	      std::back_inserter(ndesc.outputs));

    nodetypes[ndesc_.nodename] = ndesc;
}

OmNodeDescriptor::OmNodeDescriptor(const std::string &nodename_,
				   OmNodeCreator creator_)
	: nodename(nodename_), creator(creator_)
{
}

void
OmNodeDescriptor::add_input(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    inputs.push_back(OmNodeConnection(name, type, phys_type));
}

void
OmNodeDescriptor::add_output(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    outputs.push_back(OmNodeConnection(name, type, phys_type));
}
