/* omindexerinternal.h: An indexing structure built from an XML definition
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

#ifndef OM_HGUARD_OMINDEXERINTERNAL_H
#define OM_HGUARD_OMINDEXERINTERNAL_H

#include "om/omindexer.h"
#include "om/omindexernode.h"
#include "om/omerror.h"
#include "deleter_map.h"

class OmIndexerStartNode;

class OmIndexer::Internal {
    public:
	typedef deleter_map<std::string, OmIndexerNode *> NodeMap;

	/** Storage for the indexer nodes, by id.
	 */
	NodeMap nodemap;

	/** The final node (which produces the final results) */
	OmIndexerNode *final;

	/** The name of the output from final which produces the final results */
	std::string final_out;

	/** The start node */
	OmIndexerStartNode *start;
};

/** A special node used internally as the START node by the indexer. */
class OmIndexerStartNode : public OmIndexerNode
{
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new OmIndexerStartNode(settings);
	}

	void set_message(OmIndexerMessage msg) {
	    //cout << "Setting message:" << msg << endl;
	    message = msg;
	    message_valid = true;
	};
    private:
	OmIndexerStartNode(const OmSettings &settings)
		: OmIndexerNode(settings), message_valid(false) {}
	void calculate() {
	    if (!message_valid) {
		throw OmInvalidArgumentError("Message read more than once from START!");
	    }
	    if (!message.get()) {
		throw OmInvalidArgumentError("Message is null!");
	    }
	    set_output("out", message);
	    message_valid = false;
	}

	OmIndexerMessage message;
	bool message_valid;
};

#endif /* OM_HGUARD_OMINDEXERINTERNAL_H */
