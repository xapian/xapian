/* omindexerbuilder.cc: Code to build an indexer from a list of nodes.
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

#include "om/omindexernode.h"
#include "om/omindexerbuilder.h"
#include "omindexerinternal.h"
#include "indexerxml.h"
#include "om/omerror.h"
#include "register_core.h"
#include "toposort.h"
#include "omnodeconnection.h"
#include "om/omnodedescriptor.h"
#include "omnodedescriptorinternal.h"
#include <algorithm>

class OmIndexerBuilder::Internal {
    public:
	/** Constructor. */
	Internal();

	/** Build an indexer from an XML file
	 *  
	 *  @param filename	The name of the file describing the indexer
	 *                      network.
	 */
	void build_from_file(const std::string &filename,
			     OmIndexer::Internal *indexer);

	/** Build an indexer from an XML string
	 *  
	 *  @param xmldesc	The string describing the indexer network.
	 */
	void build_from_string(const std::string &xmldesc,
			       OmIndexer::Internal *indexer);

	/** Build an indexer from an in-memory structure.
	 * 
	 *  @param desc		The description of the graph.
	 */
	void build_from_desc(const OmIndexerDesc &desc);

	/** Register a new node type */
	void register_node_type(const OmNodeDescriptor::Internal &nodedesc);

    private:
	/** Build the node graph (with checking) and set up the final
	 *  node pointer.
	 */
	void build_graph(OmIndexer::Internal *indexer,
			 const OmIndexerDesc &desc);

	/** Return a sorted order suitable for instantiating nodes.  Uses
	 *  a topological sort.
	 *
	 *  @param desc	The description of the nodes.
	 */
	std::vector<int> sort_nodes(const OmIndexerDesc &desc);
	
	/** Data kept with each node as the graph is being built */
	struct type_data {
	    std::string node_name;
	    std::vector<OmNodeConnection> inputs;
	    std::vector<OmNodeConnection> outputs;
	};

	/** The structure with information about each node's connections. */
	typedef std::map<std::string, type_data> typemap;


	/** Make sure that the types at each end of a connection are
	 *  compatible.  Throw an exception if not.
	 */
	void typecheck(type_data &feeder_node,
		       const std::string &feeder_output,
		       type_data &receiver_node,
		       const std::string &receiver_input);

	/** Get the descriptor for an output connection for a particular
	 *  node type.
	 */
	OmNodeConnection get_outputcon(const std::string &nodetype,
				     const std::string &output_name);

	/** Get the descriptor for an input connection for a particular
	 *  node type.
	 */
	OmNodeConnection get_inputcon(const std::string &nodetype,
				    const std::string &input_name);

	/** Create a node given a name
	 *
	 *  @param type  The node type.
	 */
	OmIndexerNode *make_node(const std::string &type,
				 const OmSettings &config);

	/** Node descriptor */
	struct node_desc {
	    OmNodeCreator create;
	    std::vector<OmNodeConnection> inputs;
	    std::vector<OmNodeConnection> outputs;
	};

	/** Node database */
	std::map<std::string, node_desc> nodetypes;
};

AutoPtr<OmIndexer>
OmIndexerBuilder::build_from_file(const std::string &filename)
{
    AutoPtr<OmIndexer> indexer(new OmIndexer());
    internal->build_from_file(filename, indexer->internal);
    return indexer;
}

void
OmIndexerBuilder::Internal::build_from_file(const std::string &filename,
					    OmIndexer::Internal *indexer)
{
    AutoPtr<OmIndexerDesc> doc = desc_from_xml_file(filename);

    build_graph(indexer, *doc);
}

AutoPtr<OmIndexer>
OmIndexerBuilder::build_from_string(const std::string &xmldesc)
{
    AutoPtr<OmIndexer> indexer(new OmIndexer());
    internal->build_from_string(xmldesc, indexer->internal);
    return indexer;
}

void
OmIndexerBuilder::Internal::build_from_string(const std::string &xmldesc,
					      OmIndexer::Internal *indexer)
{
    AutoPtr<OmIndexerDesc> doc = desc_from_xml_string(xmldesc);

    build_graph(indexer, *doc);
}

OmIndexerNode *
OmIndexerBuilder::Internal::make_node(const std::string &type,
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
OmIndexerBuilder::Internal::sort_nodes(const OmIndexerDesc &desc)
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
OmIndexerBuilder::Internal::build_graph(OmIndexer::Internal *indexer,
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

    for (unsigned int nodeind = 0;
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
	    OmIndexer::Internal::NodeMap::const_iterator i =
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
    OmIndexer::Internal::NodeMap::const_iterator i =
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
OmIndexerBuilder::Internal::typecheck(type_data &feeder_node,
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
	} else if (sendercon.type.length() > 0 && sendercon.type[0] == '*') {
	    // handle an un-instantiated output wildcard type
	    std::string wildcard = sendercon.type;
	    replace_type(feeder_node.inputs,
			 wildcard,
			 receivercon.type,
			 receivercon.phys_type);
	    replace_type(feeder_node.outputs,
			 wildcard,
			 receivercon.type,
			 receivercon.phys_type);
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
OmIndexerBuilder::Internal::get_outputcon(const std::string &nodetype,
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
OmIndexerBuilder::Internal::get_inputcon(const std::string &nodetype,
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

OmIndexerBuilder::OmIndexerBuilder()
	: internal(new Internal)
{
    try {
	OmNodeDescriptor ndesc("START", &OmIndexerStartNode::create);
	ndesc.add_output("out", "mystr", mt_record);
	register_node_type(ndesc);

	register_core_nodes(*this);
    } catch (...) {
	delete internal;
	throw;
    }
}

OmIndexerBuilder::~OmIndexerBuilder()
{
    delete internal;
}

OmIndexerBuilder::Internal::Internal()
{
}

void
OmIndexerBuilder::register_node_type(const OmNodeDescriptor &ndesc_)
{
    internal->register_node_type(*ndesc_.internal);
}

void
OmIndexerBuilder::Internal::register_node_type(const OmNodeDescriptor::Internal &ndesc_)
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
