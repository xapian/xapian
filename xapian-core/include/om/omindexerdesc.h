/* omindexerdesc.h: An intermediate form for the graph description
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_OMINDEXERDESC_H
#define OM_HGUARD_OMINDEXERDESC_H

#include <string>
#include "om/omsettings.h"

class OmNodeInstanceIterator;

/** An intermediate form for the node graphs.
 *  This is a description of an indexer graph which can be used instead
 *  of an XML file to build an indexer.  It contains the same information.
 */
class OmIndexerDesc {
    public:
	class Internal;
	Internal *internal;

	/** The default constructor. */
	OmIndexerDesc();

	/** The copy constructor */
	OmIndexerDesc(const OmIndexerDesc &other);

	/** The assignment operator */
	void operator=(const OmIndexerDesc &other);

	/** Destructor */
	~OmIndexerDesc();

	/** Add an instance of a node to the graph.
	 *  Returns an iterator which can be used to eg add input
	 *  connections.
	 */
	OmNodeInstanceIterator add_node(const std::string &type,
					const std::string &id,
					const OmSettings &param = OmSettings());

	/** Return an iterator to the beginning of the list of
	 *  node instances described by this object.
	 */
	OmNodeInstanceIterator nodes_begin() const;

	/** Return an iterator to one past the end of the list of
	 *  node instances described by this object.
	 */
	OmNodeInstanceIterator nodes_end() const;

	/** Return a handle to a node identified by name.
	 *  If no node exists with the given id, then the result
	 *  is equal to the result of nodes_end()
	 */
	OmNodeInstanceIterator find_node(const std::string &id) const;

	/** Return the id of the output node */
	std::string get_output_node() const;

	/** Return the id of the output pad */
	std::string get_output_pad() const;

	/** Set the name of the node providing the indexer's output,
	 *  plus the name of the output pad on that node. */
	void set_output(const std::string &output_node,
			const std::string &output_pad);

    private:
	friend class OmIndexerBuilder;

	OmIndexerDesc(Internal *internal_);
};

#endif /* OM_HGUARD_OMINDEXERDESC_H */
