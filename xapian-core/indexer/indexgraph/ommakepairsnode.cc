/* ommakepairsnode.cc: Implementation of a make-pairs node
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

/** Node which combines two lists into a list of pairs.
 *
 *  The ommakepairs node takes two equal-length lists as input.  It combines
 *  them into a new list of the same length, each element l[i] being a list
 *  of each corresponding input element, ie (left[i], right[i]).  (Compare
 *  to ommakepair)
 *
 *  Inputs:
 *  	left: The first input list
 *  	right: The second input list
 *
 *  Outputs:
 *  	out: The list of pairs built from left and right.
 *
 *  Parameters: none
 */
class OmMakePairsNode : public OmIndexerNode {
    public:
	OmMakePairsNode(const OmSettings &config)
		: OmIndexerNode(config)
		{}
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage left = get_input_record("left");
	    OmIndexerMessage right = get_input_record("right");

	    OmIndexerMessage out;
	    out.set_vector();

	    for (size_t i=0; i<left.get_vector_length(); ++i) {
		out.append_element(make_pair(left.get_element(i),
					     right.get_element(i)));
	    }

	    set_output("out", out);
	}

	OmIndexerMessage make_pair(const OmIndexerMessage &left,
				   const OmIndexerMessage &right) {
	    OmIndexerMessage pair;
	    pair.set_vector();
	    pair.append_element(left);
	    pair.append_element(right);
	    return pair;
	}
};

NODE_BEGIN(OmMakePairsNode, ommakepairs)
NODE_INPUT("left", "*1", mt_vector)
NODE_INPUT("right", "*2", mt_vector)
NODE_OUTPUT("out", "*3", mt_vector)
NODE_END()
