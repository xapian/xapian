/* omnodedescriptor.h: Data used in registering new indexer nodes
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

#ifndef OM_HGUARD_OMNODEDESCRIPTOR_H
#define OM_HGUARD_OMNODEDESCRIPTOR_H

#include <string>
#include <om/omindexercommon.h>

// available in <om/omindexernode.h>
class OmIndexerNode;

// available in <om/omsettings.h>
class OmSettings;

/** A function pointer to a node object creator. */
typedef OmIndexerNode *(*OmNodeCreator)(const OmSettings &config);

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

	/** Destructor. */
	~OmNodeDescriptor();

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
	class Internal;
	Internal *internal;
};

#endif /* OM_HGUARD_OMNODEDESCRIPTOR_H */
