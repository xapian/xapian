/* listconcatnode.cc: Implementation of a list concatenation node
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

class OmListConcatNode : public OmIndexerNode {
    public:
	OmListConcatNode(const OmSettings &config)
		: OmIndexerNode(config)
		{}
    private:
	void calculate() {
	    OmIndexerMessage left = get_input_record("left");
	    OmIndexerMessage right = get_input_record("right");

	    for (int i=0; i<right->get_vector_length(); ++i) {
		// FIXME use a multi-append function for efficiency?
		left->append_element(right->get_element(i));
	    }
	    set_output("out", left);
	}
};

NODE_BEGIN(OmListConcatNode, omlistconcat)
NODE_INPUT("left", "*1", mt_vector)
NODE_INPUT("right", "*1", mt_vector)
NODE_OUTPUT("out", "*1", mt_vector)
NODE_END()
