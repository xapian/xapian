/* omsplitternode.cc: Implementation of the Splitter node
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
#include "node_reg.h"

class OmSplitterNode : public OmIndexerNode {
    public:
	OmSplitterNode(const OmSettings &config)
		: OmIndexerNode(config)
		{}
    private:
	void calculate() {
	    OmIndexerMessage msg = get_input_record("in");

	    set_output("left", OmIndexerMessage(new OmIndexerData(*msg)));
	    set_output("right", msg);
	}
};

NODE_BEGIN(OmSplitterNode, omsplitter)
NODE_INPUT("in", "*1", mt_record)
NODE_OUTPUT("left", "*1", mt_record)
NODE_OUTPUT("right", "*1", mt_record)
NODE_END()
