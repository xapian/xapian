/* omselectitemsnode.cc: Implementation of a select-from-list node
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
#include <vector>
#include <cstdio>

using std::vector;

class OmSelectItemsNode : public OmIndexerNode {
    public:
	OmSelectItemsNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    vector<std::string> sitems = config.get_vector("items");

	    vector<std::string>::const_iterator i;
	    for (i = sitems.begin();
		 i != sitems.end();
		 ++i) {
		int num;
		sscanf(i->c_str(), "%d", &num);
		items.push_back(num);
	    }
	}
    private:
	vector<int> items;
	// FIXME: implement config_modified()
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");

	    OmIndexerMessage output(new OmIndexerData(
				    std::vector<OmIndexerData>()));

	    vector<int>::const_iterator i;
	    for (i = items.begin();
		 i != items.end();
		 ++i) {
		output->append_element(input->get_element(*i));
	    }
	    set_output("out", output);
	}
};

NODE_BEGIN(OmSelectItemsNode, omselectitems)
NODE_INPUT("in", "*1", mt_vector)
NODE_OUTPUT("out", "*1", mt_vector)
NODE_END()
