/* ommakepairnode.cc: Implementation of a make-pair node
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

#include "om/omindexernode.h"
#include "node_reg.h"

/** Node which pairs two messages into a pair.
 *
 *  The ommakepair node takes any messages as input and produces one
 *  vector of two elements from the inputs.
 *
 *  Inputs:
 *  	left: The first item to add to the pair
 *  	right: The second item to add to the pair
 *
 *  Outputs:
 *  	out: The pair containing left and right.
 *
 *  Parameters: none
 */
class OmMakePairNode : public OmIndexerNode {
    public:
	OmMakePairNode(const OmSettings &config)
		: OmIndexerNode(config)
		{}
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage left = get_input_record("left");
	    OmIndexerMessage right = get_input_record("right");

	    OmIndexerMessage out;
	    out.set_vector();

	    out.append_element(left);
	    out.append_element(right);

	    set_output("out", out);
	}
};

NODE_BEGIN(OmMakePairNode, ommakepair)
NODE_INPUT("left", "*1", mt_record)
NODE_INPUT("right", "*2", mt_record)
NODE_OUTPUT("out", "*3", mt_vector)
NODE_END()
