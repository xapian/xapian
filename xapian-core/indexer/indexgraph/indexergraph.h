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

template <class T, class U>
class deleter_map : private std::map<T, U> {
    public:
	using std::map<T, U>::iterator;
	using std::map<T, U>::const_iterator;
	using std::map<T, U>::begin;
	using std::map<T, U>::end;
	using std::map<T, U>::operator[];
	using std::map<T, U>::find;
	~deleter_map() {
	    for (typename std::map<T,U>::iterator i = begin();
		 i != end();
		 ++i) {
		delete i->second;
	    }
	}
};

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

	/** A function pointer to a node object creator. */
	typedef OmIndexerNode *(*NodeCreator)(const OmSettings &config);

	/** The description of an input or output connection. */
	struct NodeConnection {
	    NodeConnection(std::string name_,
			   std::string type_,
			   OmIndexerMessageType phys_type_)
		    : name(name_), type(type_), phys_type(phys_type_) {}
	    NodeConnection(const NodeConnection &other)
		    : name(other.name), type(other.type),
	              phys_type(other.phys_type) {}
	    NodeConnection()
		    : name(""), type(""), phys_type() {}

	    /** The name of this input or output */
	    std::string name;

	    /** The high-level type of this connection. */
	    std::string type;

	    /** The low-level type of this connection. */
	    OmIndexerMessageType phys_type;
	};

	/** Register a new node type */
	void register_node_type(const std::string &nodename,
				NodeCreator create,
				const std::vector<NodeConnection> &inputs,
				const std::vector<NodeConnection> &outputs);

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
	NodeConnection get_outputcon(const std::string &nodetype,
				     const std::string &output_name);

	/** Get the descriptor for an input connection for a particular
	 *  node type.
	 */
	NodeConnection get_inputcon(const std::string &nodetype,
				    const std::string &input_name);

	/** Create a node given a name
	 *
	 *  @param type  The node type.
	 */
	OmIndexerNode *make_node(const std::string &type);

	/** Node descriptor */
	struct node_desc {
	    NodeCreator create;
	    std::vector<NodeConnection> inputs;
	    std::vector<NodeConnection> outputs;
	};

	/** Node database */
	std::map<std::string, node_desc> nodetypes;
};

#endif // HAVE_LIBXML

#endif /* OM_HGUARD_OMINDEXERGRAPH_H */
