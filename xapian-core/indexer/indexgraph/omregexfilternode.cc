/* omregexfilternode.cc: Implementation of the regex replace node
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
#include "regexcommon.h"
#include "om/omindexernode.h"
#include "node_reg.h"
#include <cctype>

class OmRegexFilterNode : public OmIndexerNode {
    public:
	OmRegexFilterNode(const OmSettings &config)
		: OmIndexerNode(config),
		  config_regex(config.get("regex", "")),
		  regex_from_config(config_regex.length() > 0)
	{
	    if (regex_from_config) {
		regex.set(config_regex);
	    }
	}
    private:
	std::string config_regex;
	bool regex_from_config;
	Regex regex;
	// FIXME: implement config_modified()
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");

	    if (!regex_from_config) {
		regex.set(get_input_string("regex"));
	    }

	    OmIndexerMessage output(new OmIndexerData(
				    std::vector<OmIndexerData>()));

	    for (int i=0; i<input->get_vector_length(); ++i) {
		std::string orig = input->get_element(i).get_string();
		if (regex.matches(orig)) {
		    output->append_element(OmIndexerData(orig));
		}
		set_output("out", output);
	    }
	}
};

NODE_BEGIN(OmRegexFilterNode, omregexfilter)
NODE_INPUT("in", "strings", mt_vector)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
