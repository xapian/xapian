/* omindexergraph.h: interface to building the indexer graph
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

#ifndef OM_HGUARD_OMINDEXERGRAPH_H
#define OM_HGUARD_OMINDEXERGRAPH_H

#include "config.h"

#ifdef HAVE_LIBXML

#include <string>
#include <map>
#include <parser.h>
#include "omindexernode.h"
#include "om/omsettings.h"
#include "deleter_map.h"

class OmIndexerStartNode;

class OmIndexer {
    public:
	friend class OmIndexerBuilder;

	/** Set the input
	 */
	void set_input(Message msg);
	/** Get the output
	 */
	Message get_output();

	/** Destructor
	 */
	~OmIndexer();
    private:
	/** Construct a blank indexer
	 */
	OmIndexer();
    private:
	typedef deleter_map<std::string, OmIndexerNode *> NodeMap;

	/** Storage for the indexer nodes, by id.
	 */
	NodeMap nodemap;

	/** The final node (which produces the final results) */
	OmIndexerNode *final;

	/** The start node */
	OmIndexerStartNode *start;
};

/** A function pointer to a node object creator. */
typedef OmIndexerNode *(*OmNodeCreator)(const OmSettings &config);

/** The description of an input or output connection. */
struct OmNodeConnection {
    OmNodeConnection(std::string name_,
		   std::string type_,
		   OmIndexerMessageType phys_type_)
	    : name(name_), type(type_), phys_type(phys_type_) {}
    OmNodeConnection(const OmNodeConnection &other)
	    : name(other.name), type(other.type),
    phys_type(other.phys_type) {}
    OmNodeConnection()
	    : name(""), type("") {}

    /** The name of this input or output */
    std::string name;

    /** The high-level type of this connection. */
    std::string type;

    /** The low-level type of this connection. */
    OmIndexerMessageType phys_type;
};

/** A description of a new node type */
class OmNodeDescriptor {
    public:
	/** Constructor for node descriptor.
	 *
	 *  @param nodename	The name of this node type
	 *
	 *  @param creator	The function used to create an instance of
	 *  			this node.
	 */
	OmNodeDescriptor(const std::string &nodename_,
			 OmNodeCreator creator_);

	/** Add an input description to this node.
	 *
	 * @param name		The name of this input.
	 * @param type		The input type (a string description)
	 * @param phys_type	The input basic type
	 */
	void add_input(const std::string &name,
		       const std::string &type,
		       OmIndexerMessageType phys_type);

	/** Add an output description to this node.
	 *
	 * @param name		The name of this input.
	 * @param type		The output type (a string description)
	 * @param phys_type	The output basic type
	 */
	void add_output(const std::string &name,
			const std::string &type,
			OmIndexerMessageType phys_type);
    private:
	friend class OmIndexerBuilder;
	std::string nodename;
	OmNodeCreator creator;
	std::vector<OmNodeConnection> inputs;
	std::vector<OmNodeConnection> outputs;
};

class OmIndexerBuilder {
    public:
	/** Constructor */
	OmIndexerBuilder();

	/** Build an indexer from an XML file
	 *  
	 *  @param filename	The name of the file describing the indexer
	 *                      network.
	 */
	auto_ptr<OmIndexer> build_from_file(std::string filename);

	/** Build an indexer from an XML string
	 *  
	 *  @param xmldesc	The string describing the indexer network.
	 */
	auto_ptr<OmIndexer> build_from_string(std::string xmldesc);

	/** Register a new node type */
	void register_node_type(const OmNodeDescriptor &nodedesc);

    private:
	/** Get the XML parse tree from a file */
	xmlDocPtr get_xmltree_from_file(const std::string &filename);

	/** Get the XML parse tree from a string*/
	xmlDocPtr get_xmltree_from_string(const std::string &xmldesc);

	/** Build the node graph (with checking) and set up the final
	 *  node pointer.
	 */
	void build_graph(OmIndexer *indexer, xmlDocPtr doc);

	/** Make sure that the types at each end of a connection are
	 *  compatible.  Throw an exception if not.
	 */
	void typecheck(const std::string &sendertype,
		       const std::string &senderout,
		       const std::string &receivertype,
		       const std::string &receiverin);

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
	OmIndexerNode *make_node(const std::string &type);

	/** Node descriptor */
	struct node_desc {
	    OmNodeCreator create;
	    std::vector<OmNodeConnection> inputs;
	    std::vector<OmNodeConnection> outputs;
	};

	/** Node database */
	std::map<std::string, node_desc> nodetypes;
};

#endif // HAVE_LIBXML

#endif /* OM_HGUARD_OMINDEXERGRAPH_H */
