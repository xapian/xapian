/* omvectorsplitnode.cc: Split a vector into separate messages
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

/** Node which splits a vector into several messages.
 *
 *  The omvectorsplit node is a utility node which converts a list
 *  of N elements into N separate messages.  This can be useful when
 *  splitting a source document into more than one index document,
 *  which can only be indexed one at a time.
 *
 *  The output "out" can be read N times with different results before
 *  the "in" input is reread.
 *
 *  Inputs:
 *  	in: A list of any type.
 *
 *  Outputs:
 *  	out: Part of the input list.
 */
class OmVectorSplitNode : public OmIndexerNode {
    public:
	OmVectorSplitNode(const OmSettings &config)
		: OmIndexerNode(config),
		  stored(0), offset(0)
		{}
    private:
	OmIndexerMessage stored;
	bool stored_valid;
	size_t offset;
	void calculate() {
	    if (!stored_valid) {
		request_inputs();
		stored = get_input_record("in");
		if (stored.get_type() == OmIndexerMessage::rt_empty) {
		    stored_valid = false;
		    set_empty_output("out");
		    return;
		}
		stored_valid = true;
		offset = 0;
	    }
	    if (offset >= stored.get_vector_length()) {
		set_empty_output("out");
		stored_valid = false;
		return;
	    }
	    set_output("out", stored.get_element(offset));
	    ++offset;

	}
};

NODE_BEGIN(OmVectorSplitNode, omvectorsplit)
NODE_INPUT("in", "*1", mt_vector)
NODE_OUTPUT("out", "*2", mt_record)
NODE_END()
