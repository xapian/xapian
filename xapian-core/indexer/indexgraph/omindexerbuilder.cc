/* omindexerbuilder.cc: Code to build an indexer from a list of nodes.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#include <config.h>
#include "om/omindexernode.h"
#include "om/omindexerbuilder.h"
#include "omindexerinternal.h"
#include "indexerxml.h"
#include "om/omerror.h"
#include "register_core.h"
#include "toposort.h"
#include "omnodepad.h"
#include "om/omnodedescriptor.h"
#include "omnodedescriptorinternal.h"
#include "omdebug.h"
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
	void build_from_desc(const OmIndexerDesc::Internal &desc,
			     OmIndexer::Internal *indexer);

	/** Register a new node type */
	void register_node_type(const OmNodeDescriptor::Internal &nodedesc);

	/** Return a sorted order suitable for instantiating nodes.  Uses
	 *  a topological sort.
	 *
	 *  @param desc	The description of the nodes.
	 */
	static std::vector<int> sort_nodes(const OmIndexerDesc::Internal &desc);

	/** Return information about a node type by name. */
	AutoPtr<OmNodeDescriptor::Internal> get_node_info(const std::string &type);
    private:
	/** Build the node graph (with checking) and set up the final
	 *  node pointer.
	 */
	void build_graph(OmIndexer::Internal *indexer,
			 const OmIndexerDesc::Internal &desc);

	/** Verify that all the node types are valid.
	 *  Throw an OmInvalidDataError if any types in the desc are unknown.
	 */
	void verify_node_types(const OmIndexerDesc::Internal &desc);

	/** Return a count of the number of nodes which have more than
	 *  one output.
	 */
	int count_splitting_nodes(const OmIndexerDesc::Internal &desc);
	
	/** Data kept with each node as the graph is being built */
	struct type_data {
	    std::string node_name;
	    std::vector<OmNodePad> inputs;
	    std::vector<OmNodePad> outputs;
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
	OmNodePad get_outputcon(const std::string &nodetype,
				     const std::string &output_name);

	/** Get the descriptor for an input connection for a particular
	 *  node type.
	 */
	OmNodePad get_inputcon(const std::string &nodetype,
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
	    std::vector<OmNodePad> inputs;
	    std::vector<OmNodePad> outputs;
	};

	/** Node database */
	std::map<std::string, node_desc> nodetypes;
};

OmIndexerDesc
OmIndexerBuilder::desc_from_file(const std::string &filename)
{
    AutoPtr<OmIndexerDesc::Internal> desc(desc_from_xml_file(filename));

    OmIndexerDesc result(desc.get());
    desc.release();

    return result;
}

OmIndexerDesc
OmIndexerBuilder::desc_from_string(const std::string &xmldesc)
{
    AutoPtr<OmIndexerDesc::Internal> desc(desc_from_xml_string(xmldesc));

    OmIndexerDesc result(desc.get());
    desc.release();

    return result;
}

OmIndexerDesc
OmIndexerBuilder::sort_desc(OmIndexerDesc &desc)
{
    std::vector<int> sorted = Internal::sort_nodes(*desc.internal);

    AutoPtr<OmIndexerDesc::Internal> result(new OmIndexerDesc::Internal);
    result->data->output_node = desc.internal->data->output_node;
    result->data->output_pad = desc.internal->data->output_pad;

    std::vector<int>::const_iterator i = sorted.begin();
    while (i != sorted.end()) {
	result->data->nodes.push_back(desc.internal->data->nodes[*i]);
	++i;
    }
    OmIndexerDesc temp(result.get());
    result.release();
    return temp;
}

OmIndexer
OmIndexerBuilder::build_from_file(const std::string &filename)
{
    OmIndexer indexer;
    internal->build_from_file(filename, indexer.internal);
    return indexer;
}

void
OmIndexerBuilder::Internal::build_from_file(const std::string &filename,
					    OmIndexer::Internal *indexer)
{
    AutoPtr<OmIndexerDesc::Internal> doc = desc_from_xml_file(filename);

    build_graph(indexer, *doc);
}

OmIndexer
OmIndexerBuilder::build_from_desc(const OmIndexerDesc &desc)
{
    OmIndexer indexer;
    internal->build_from_desc(*desc.internal, indexer.internal);
    return indexer;
}

void
OmIndexerBuilder::Internal::build_from_desc(const OmIndexerDesc::Internal &desc,
					    OmIndexer::Internal *indexer)
{
    build_graph(indexer, desc);
}

OmIndexer
OmIndexerBuilder::build_from_string(const std::string &xmldesc)
{
    OmIndexer indexer;
    internal->build_from_string(xmldesc, indexer.internal);
    return indexer;
}

void
OmIndexerBuilder::Internal::build_from_string(const std::string &xmldesc,
					      OmIndexer::Internal *indexer)
{
    AutoPtr<OmIndexerDesc::Internal> doc = desc_from_xml_string(xmldesc);

    DEBUGLINE(INDEXER, __LINE__ << ": desc size " << doc->data->nodes.size());
    build_graph(indexer, *doc);
}

OmNodeDescriptor
OmIndexerBuilder::get_node_info(const std::string &type)
{
    return OmNodeDescriptor((internal->get_node_info(type)).release());
}

AutoPtr<OmNodeDescriptor::Internal>
OmIndexerBuilder::Internal::get_node_info(const std::string &type)
{
    AutoPtr<OmNodeDescriptor::Internal> result(new OmNodeDescriptor::Internal);
    result->data->type = type;

    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(type);

    if (i == nodetypes.end()) {
	throw OmRangeError(std::string("Unknown node type ") + type);
    }
    result->data->inputs = i->second.inputs;
    result->data->outputs = i->second.outputs;

    return result;
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
OmIndexerBuilder::Internal::sort_nodes(const OmIndexerDesc::Internal &desc)
{
    DEBUGLINE(INDEXER, "sort_nodes(): sorting desc of length "
	      << desc.data->nodes.size());
    /* First build up a mapping from node ids to positions in the desc.
     */
    std::map<std::string, int> node_num;
    for (size_t i = 0; i < desc.data->nodes.size(); ++i) {
	node_num[desc.data->nodes[i].id] = i;
    }
    /* temporarily add the START node in.  We'll remove it before returning
     * the result.  (The START node is special, and is not mentioned in
     * desc.)
     */
    int num_elements = desc.data->nodes.size();
    node_num["START"] = num_elements;
    ++num_elements;

    TopoSort tsort(num_elements);

    for (size_t i=0; i<desc.data->nodes.size(); ++i) {
	const OmIndexerDesc::Internal::NodeInstance &node = desc.data->nodes[i];
	std::vector<OmIndexerDesc::Internal::Connect>::const_iterator j;
	for (j = node.input.begin();
	     j != node.input.end();
	     ++j) {
	    tsort.add_pair(node_num[j->feeder_node], i);
	}
    }
    TopoSort::result_type result = tsort.get_result();
    DEBUGLINE(INDEXER, "At line " << __LINE__ << ", result.size(): " <<
	      result.size());

    /* now remove the entry for START from the list */
    result.erase(std::find(result.begin(), result.end(), num_elements-1));
    DEBUGLINE(INDEXER, "At line " << __LINE__ << ", result.size(): " <<
	      result.size());

    return result;
}

void
OmIndexerBuilder::Internal::verify_node_types(const OmIndexerDesc::Internal &desc)
{
    for (size_t i=0; i<desc.data->nodes.size(); ++i) {
	std::map<std::string, node_desc>::const_iterator type;
	type = nodetypes.find(desc.data->nodes[i].type);
	if (type == nodetypes.end()) {
	    throw OmInvalidDataError(std::string("Unknown node type ") +
				     desc.data->nodes[i].type);
	}
    }
}

int
OmIndexerBuilder::Internal::count_splitting_nodes(const OmIndexerDesc::Internal &desc)
{
    int count = 0;
    for (size_t i=0; i<desc.data->nodes.size(); ++i) {
	if (nodetypes[desc.data->nodes[i].type].outputs.size() > 1) {
	    ++count;
	}
    }
    return count;
}

void
OmIndexerBuilder::Internal::build_graph(OmIndexer::Internal *indexer,
			      const OmIndexerDesc::Internal &desc)
{
    typemap types;

    /* sort the list of nodes so that nodes aren't referred to before
     * being instantiated.
     */
    DEBUGLINE(INDEXER, "Desc size is " << desc.data->nodes.size());
    std::vector<int> sorted = sort_nodes(desc);
    DEBUGLINE(INDEXER, "Desc size after sort_nodes:" << desc.data->nodes.size());
    
    /* First check that all the node types are valid.
     * This must be called before any accidental use of
     * nodetypes[desc....], such as in count_splitting_nodes.
     */
    verify_node_types(desc);

    // get a count of the number of nodes with more than one output
    int num_splitting_nodes = count_splitting_nodes(desc);
    int next_splitting_node_num = 0;
    DEBUGLINE(INDEXER, "count of splitting nodes: " << num_splitting_nodes);

    OmSettings startconfig;
    startconfig.set("omindexer_numsplitting", num_splitting_nodes);
    startconfig.set("omindexer_node_id", "START");

    indexer->nodemap["START"] = make_node("START", startconfig);
    indexer->start = dynamic_cast<OmIndexerStartNode *>(indexer->nodemap["START"]);
    types["START"].outputs = nodetypes["START"].outputs;
    types["START"].node_name = "START";

    DEBUGLINE(INDEXER, "Nodes in description: " << desc.data->nodes.size());
    DEBUGLINE(INDEXER, "Nodes in sorted: " << sorted.size());
    for (unsigned int nodeind = 0;
	 nodeind < sorted.size();
	 ++nodeind) {
	const OmIndexerDesc::Internal::NodeInstance *node =
		&desc.data->nodes[sorted[nodeind]];

	if (indexer->nodemap.find(node->id) != indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Duplicate node id ")
					 + node->id);
	}

	OmSettings config(node->param);

	// add some internal data to the configuration
	config.set("omindexer_numsplitting", num_splitting_nodes);
	config.set("omindexer_node_id", node->id);
	if (nodetypes[node->type].outputs.size() > 1) {
	    config.set("omindexer_mysplitid", next_splitting_node_num++);
	}
	OmIndexerNode *newnode = make_node(node->type, config);
	types[node->id].node_name = node->id;
	types[node->id].inputs = nodetypes[node->type].inputs;
	types[node->id].outputs = nodetypes[node->type].outputs;
	indexer->nodemap[node->id] = newnode;

	// connect the inputs
	std::vector<OmIndexerDesc::Internal::Connect>::const_iterator input;
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
	    indexer->nodemap.find(desc.data->output_node);
    if (i == indexer->nodemap.end()) {
	for (i=indexer->nodemap.begin(); i!=indexer->nodemap.end(); ++i) {
	    DEBUGLINE(INDEXER, "nodemap element " << i->first);
	}
	throw OmInvalidDataError(std::string("Unknown output node ") +
				 desc.data->output_node);
    }
    indexer->final = i->second;
    indexer->final_out = desc.data->output_pad;
}

static const OmNodePad &find_conn(const std::vector<OmNodePad> &v,
					 const std::string &name,
					 const std::string &node_name)
{
    std::vector<OmNodePad>::const_iterator i;
    for (i=v.begin(); i!=v.end(); ++i) {
	if (i->name == name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Failed to find connection ") +
			     node_name + "[" + name + "]");
}

static void replace_type(std::vector<OmNodePad> &v,
			 const std::string &wildcard,
			 const std::string &real_type,
			 OmIndexerMessageType phys_type)
{
    std::vector<OmNodePad>::iterator i;
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
    const OmNodePad &sendercon = find_conn(feeder_node.outputs,
						  feeder_output,
						  feeder_node.node_name);
    const OmNodePad &receivercon = find_conn(receiver_node.inputs,
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
    }

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

OmNodePad
OmIndexerBuilder::Internal::get_outputcon(const std::string &nodetype,
				const std::string &output_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodePad>::const_iterator i;
    for (i=type->second.outputs.begin(); i!= type->second.outputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == output_name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Invalid output connection ") +
				 nodetype + "[" + output_name + "]");
}

OmNodePad
OmIndexerBuilder::Internal::get_inputcon(const std::string &nodetype,
			       const std::string &input_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodePad>::const_iterator i;
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
	ndesc.add_output("out", "*", mt_record);
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
    i = nodetypes.find(ndesc_.data->nodename);
    if (i != nodetypes.end()) {
	throw OmInvalidArgumentError(std::string("Attempt to register node type ")
				     + ndesc_.data->nodename + ", which already exists.");
    }

    node_desc ndesc;
    ndesc.create = ndesc_.data->creator;
    std::copy(ndesc_.data->inputs.begin(), ndesc_.data->inputs.end(),
	      std::back_inserter(ndesc.inputs));
    std::copy(ndesc_.data->outputs.begin(), ndesc_.data->outputs.end(),
	      std::back_inserter(ndesc.outputs));

    nodetypes[ndesc_.data->nodename] = ndesc;
}
