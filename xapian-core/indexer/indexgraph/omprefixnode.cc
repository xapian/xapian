/* omprefixnode.cc: Implementation of a prefixing node
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

class OmPrefixNode : public OmIndexerNode {
    public:
	OmPrefixNode(const OmSettings &config)
		: OmIndexerNode(config)
		{}
    private:
	void calculate() {
	    OmIndexerMessage input = get_input_record("in");
	    if (input->is_empty()) {
		set_empty_output("out");
		return;
	    }

	    OmIndexerMessage output(new OmIndexerData(
				      std::vector<OmIndexerData>()));

	    std::string prefix = get_config_string("prefix");

	    for (int i=0; i<input->get_vector_length(); ++i) {
		output->append_element(
	            OmIndexerData(prefix +
				  input->get_element(i).get_string()));
	    }

	    set_output("out", output);
	}
};

NODE_BEGIN(OmPrefixNode, omprefix)
NODE_INPUT("in", "strings", mt_vector)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
