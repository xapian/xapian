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

#include <string>
#include <map>
#include <parser.h>
#include "omindexernode.h"

template <class T, class U>
class deleter_map : private std::map<T, U> {
    public:
	using std::map<T, U>::iterator;
	using std::map<T, U>::begin;
	using std::map<T, U>::end;
	~deleter_map() {
	    for (iterator i = begin(); i != end(); ++i) {
		delete i->second;
	    }
	}
};

class OmIndexer {
    public:
	// FIXME: different constructors for filename/block of data?
	/** Construct the indexer from the XML file <filename>.
	 *
	 *  @param filename  The name of the file containing the definition
	 *                   of the index graph.
	 */
	OmIndexer(std::string filename);

	/** Destructor
	 */
	~OmIndexer();
    private:
	typedef deleter_map<std::string, OmIndexerNode *> NodeMap;

	/** Storage for the indexer nodes, by id.
	 */
	NodeMap nodemap;

	/** The final node (which produces the final results) */
	OmIndexerNode *final;

	/** Get the XML parse tree */
	xmlDocPtr get_xmltree(const std::string &filename);

	/** Build the node graph (with checking) and set up the final
	 *  node pointer.
	 */
	void build_graph(xmlDocPtr doc);
};

#endif /* OM_HGUARD_OMINDEXERGRAPH_H */
