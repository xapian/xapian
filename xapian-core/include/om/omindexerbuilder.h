/* omindexerbuilder.h: interface to building the indexer graph
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

#ifndef OM_HGUARD_OMINDEXERBUILDER_H
#define OM_HGUARD_OMINDEXERBUILDER_H

#include <string>
#include <om/autoptr.h>
#include <om/omindexer.h>
#include <om/omnodeconnection.h>

// available from <om/omindexerdesc.h>
class OmIndexerDesc;
// available from <om/omnodedescriptor.h>
class OmNodeDescriptor;

class OmIndexerBuilder {
    public:
	/** Constructor */
	OmIndexerBuilder();

	/** Destructor */
	~OmIndexerBuilder();

	/** Build an indexer from an XML file
	 *  
	 *  @param filename	The name of the file describing the indexer
	 *                      network.
	 */
	AutoPtr<OmIndexer> build_from_file(const std::string &filename);

	/** Build an indexer from an XML string
	 *  
	 *  @param xmldesc	The string describing the indexer network.
	 */
	AutoPtr<OmIndexer> build_from_string(const std::string &xmldesc);

	/** Build an indexer from an in-memory structure.
	 * 
	 *  @param desc		The description of the graph.
	 */
	AutoPtr<OmIndexer> build_from_desc(const OmIndexerDesc &desc);

	/** Build the OmIndexerDesc from an XML file
	 *
	 *  @param filename	The name of the file describing the indexer
	 *  			network.
	 */
	OmIndexerDesc desc_from_file(const std::string &filename);

	/** Build the OmIndexerDesc from an XML string
	 *
	 *  @param xmldesc	The string describing the indexer network.
	 */
	OmIndexerDesc desc_from_string(const std::string &xmldesc);

	/** Toplogically sort the OmIndexerDesc.
	 *  The node descriptions in desc will be put into an order such
	 *  that a node with be placed before any nodes which connect to
	 *  its outputs.  OmInvalidDataError will be thrown if a loop is
	 *  detected.
	 *
	 *  @param desc		The initial OmIndexerDesc object, which
	 *  			will be modified in place.
	 */
	static OmIndexerDesc sort_desc(OmIndexerDesc &desc);

	/** Register a new node type */
	void register_node_type(const OmNodeDescriptor &nodedesc);

	/** Structure used to describe an indexer node type. */
	struct NodeType {
	    /** The name of the node type */
	    std::string type;

	    /** The list of inputs */
	    std::vector<OmNodeConnection> inputs;
	    /** The list of outputs */
	    std::vector<OmNodeConnection> outputs;
	};

	/** Return information about a node type by name. */
	NodeType get_node_info(const std::string &type);
    private:
	class Internal;

	Internal *internal;
};

#endif /* OM_HGUARD_OMINDEXERBUILDER_H */
