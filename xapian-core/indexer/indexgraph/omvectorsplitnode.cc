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

class OmVectorSplitNode : public OmIndexerNode {
    public:
	OmVectorSplitNode(const OmSettings &config)
		: OmIndexerNode(config),
		  stored(0), offset(0)
		{}
    private:
	OmIndexerMessage stored;
	int offset;
	void calculate() {
	    if (!stored.get()) {
		stored = get_input_record("in");
		if (stored->get_type() == OmIndexerData::rt_empty) {
		    set_empty_output("out");
		    return;
		}
		offset = 0;
	    }
	    if (offset >= stored->get_vector_length()) {
		set_empty_output("out");
		stored = OmIndexerMessage(0);
		return;
	    }
	    set_output("out", OmIndexerMessage(new OmIndexerData(stored->get_element(offset))));
	    ++offset;

	}
};

NODE_BEGIN(OmVectorSplitNode, omvectorsplit)
NODE_INPUT("in", "*1", mt_vector)
NODE_OUTPUT("out", "*2", mt_record)
NODE_END()
