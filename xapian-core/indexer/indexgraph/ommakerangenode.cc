/* ommakerangenode.cc: Node which makes a numeric range
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

#include "config.h"
#include "om/omindexernode.h"
#include "node_reg.h"

/** Node which generates a range of integers.
 *
 *  The ommakerange node returns a list of integers in an arithmetic
 *  sequence of length "count", starting at "first" and going up by
 *  "step" each time.
 *
 *  Inputs: none
 *
 *  Outputs:
 *  	out: The sequence [ first, first+step, ..., first + (count-1)*step ]
 *
 *  Parameters:
 *  	first: The first number in the sequence (default 1)
 *  	step: The difference between each successive element (default 1)
 *  	count: The number of elements to produce (default 1)
 */
class OmMakeRangeNode : public OmIndexerNode {
    public:
	OmMakeRangeNode(const OmSettings &config)
		: OmIndexerNode(config),
		  first(config.get_int("first", 1)),
		  step(config.get_int("step", 1)),
		  count(config.get_int("count", 1))
	{
	}
    private:
	int first;
	int step;
	unsigned count;
	void config_modified(const std::string &key)
	{
	    if (key == "first") {
		first = get_config_int("first");
	    } else if (key == "step") {
		step = get_config_int("step");
	    } else if (key == "count") {
		count = get_config_int("count");
	    }
	}
	void calculate() {
	    OmIndexerMessage ints;
	    ints.set_vector();
	    for (size_t i=0; i<count; ++i) {
		ints.append_element(static_cast<int>(first + i*step));
	    }
	    set_output("out", ints);
	}
};

NODE_BEGIN(OmMakeRangeNode, ommakerange)
NODE_OUTPUT("out", "ints", mt_vector)
NODE_END()
