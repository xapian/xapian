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
#include "toposort.h"
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

std::vector<int>
OmIndexerBuilder::sort_nodes(const OmIndexerDesc &desc)
{
    /* First build up a mapping from node ids to positions in the desc.
     */
    std::map<std::string, int> node_num;
    for (size_t i = 0; i < desc.nodes.size(); ++i) {
	node_num[desc.nodes[i].id] = i;
    }
    /* temporarily add the START node in.  We'll remove it before returning
     * the result.  (The START node is special, and is not mentioned in
     * desc.)
     */
    int num_elements = desc.nodes.size();
    node_num["START"] = num_elements;
    ++num_elements;

    TopoSort tsort(num_elements);

    for (size_t i=0; i<desc.nodes.size(); ++i) {
	const OmIndexerDesc::NodeInstance &node = desc.nodes[i];
	std::vector<OmIndexerDesc::Connect>::const_iterator j;
	for (j = node.input.begin();
	     j != node.input.end();
	     ++j) {
	    tsort.add_pair(node_num[j->feeder_node], i);
	}
    }
    TopoSort::result_type result = tsort.get_result();

    /* now remove the entry for START from the list */
    result.erase(std::find(result.begin(), result.end(), num_elements-1));

    return result;
}

void
OmIndexerBuilder::build_graph(OmIndexer *indexer,
			      const OmIndexerDesc &desc)
{
    typemap types;

    /* sort the list of nodes so that nodes aren't referred to before
     * being instantiated.
     */
    std::vector<int> sorted = sort_nodes(desc);
    
    indexer->nodemap["START"] = make_node("START", OmSettings());
    indexer->start = dynamic_cast<OmIndexerStartNode *>(indexer->nodemap["START"]);
    types["START"].outputs = nodetypes["START"].outputs;
    types["START"].node_name = "START";

    for (int nodeind = 0;
	 nodeind < sorted.size();
	 ++nodeind) {
	const OmIndexerDesc::NodeInstance *node =
		&desc.nodes[sorted[nodeind]];

	if (indexer->nodemap.find(node->id) != indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Duplicate node id ")
					 + node->id);
	}

	OmIndexerNode *newnode = make_node(node->type, node->param);
	types[node->id].node_name = node->id;
	types[node->id].inputs = nodetypes[node->type].inputs;
	types[node->id].outputs = nodetypes[node->type].outputs;
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
	    typecheck(types[input->feeder_node],// the input node's type
		      input->feeder_out, 	// the input node's output
		      types[node->id],		// this node's type
		      input->input_name);	// this node's input name
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

static const OmNodeConnection &find_conn(const std::vector<OmNodeConnection> &v,
					 const std::string &name,
					 const std::string &node_name)
{
    std::vector<OmNodeConnection>::const_iterator i;
    for (i=v.begin(); i!=v.end(); ++i) {
	if (i->name == name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Failed to find connection ") +
			     node_name + "[" + name + "]");
}

static void replace_type(std::vector<OmNodeConnection> &v,
			 const std::string &wildcard,
			 const std::string &real_type,
			 OmIndexerMessageType phys_type)
{
    std::vector<OmNodeConnection>::iterator i;
    for (i = v.begin(); i!= v.end(); ++i) {
	if (i->type == wildcard) {
	    i->type = real_type;
	    i->phys_type = phys_type;
	}
    }
}


void
OmIndexerBuilder::typecheck(type_data &feeder_node,
			    const std::string &feeder_output,
			    type_data &receiver_node,
			    const std::string &receiver_input)
{
    const OmNodeConnection &sendercon = find_conn(feeder_node.outputs,
						  feeder_output,
						  feeder_node.node_name);
    const OmNodeConnection &receivercon = find_conn(receiver_node.inputs,
						  receiver_input,
						  receiver_node.node_name);

    // First check that the physical type is compatible
    if (sendercon.phys_type != receivercon.phys_type) {
	if (sendercon.phys_type == mt_record ||
	    receivercon.phys_type == mt_record) {
	    // this is ok - things can be converted (up to a point)
	    // between records and primitives.
	} else {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 feeder_node.node_name +
					 "[" + feeder_output + "]" +
					 " and " + receiver_node.node_name +
					 "[" + receiver_input +
					 "] are not physically compatible.");
	}
    };

    if (sendercon.type != receivercon.type) {
	if (receivercon.type.length() > 0 && receivercon.type[0] == '*') {
	    // handle a wildcard type
	    std::string wildcard = receivercon.type;
	    replace_type(receiver_node.inputs,
			 wildcard,
			 sendercon.type,
			 sendercon.phys_type);
	    replace_type(receiver_node.outputs,
			 wildcard,
			 sendercon.type,
			 sendercon.phys_type);
	} else {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 feeder_node.node_name +
					 "[" + feeder_output + "]" +
					 " and " + receiver_node.node_name +
					 "[" + receiver_input +
					 "] are not compatible: " + 
					 sendercon.type + " vs " +
					 receivercon.type);
	}
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
