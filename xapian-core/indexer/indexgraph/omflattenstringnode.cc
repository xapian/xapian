/* omflattenstringnode.cc: Node which flattens a structure containing
 * 			   strings into a single node of type string.
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

#include "omindexernode.h"
#include "om/omstem.h"
#include "om/omerror.h"
#include "node_reg.h"

class OmFlattenStringNode : public OmIndexerNode {
    public:
	OmFlattenStringNode(const OmSettings &config)
		: OmIndexerNode(config)
	{ }
    private:
	void calculate() {
	    OmIndexerMessage input = get_input_record("in");

	    set_output("out", flatten(*input));
	}
	std::string flatten(const OmIndexerData &data) {
	    switch (data.get_type()) {
		case OmIndexerData::rt_string:
		    return data.get_string();
		case OmIndexerData::rt_empty:
		    return std::string();
		    break;
		case OmIndexerData::rt_vector:
		    {
			std::string accum;
			for (int i=0; i<data.get_vector_length(); i++) {
			    accum += flatten(data.get_element(i));
			}
			return accum;
		    }
		default:
		    throw OmTypeError("Can only flatten string leaves");
	    }
	}
};

NODE_BEGIN(OmFlattenStringNode, omflattenstring)
NODE_INPUT("in", "ANY", mt_record)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
