/* omconstantnode.cc: Node which provides a constant predefined output
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
#include <cctype>

/** Node used to provide a constant value.
 *
 *  The omconstant node is configured with a constant
 *  value which can be used to supply another node's input
 *  with a particular value.
 *
 *  Inputs: none
 *
 *  Outputs:
 *  	out: the constant value
 *
 *  Parameters:
 *  	type: A string describing the type of the data.  Can be "string",
 *  		"int", "double", or "list".  The default, if specified,
 *  		is "string".
 *  	value: the constant used for output.
 */
class OmConstantNode : public OmIndexerNode {
    public:
	OmConstantNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    std::string type = config.get("type", "string");
	    set_config_string("type", type);
	    set_value();
	}
    private:
	OmIndexerData value;
	void set_value()
	{
	    std::string type = get_config_string("type");
	    if (type == "string") {
		value.set_string(get_config_string("value"));
	    } else if (type == "int") {
		value.set_int(get_config_int("value"));
	    } else if (type == "double") {
		value.set_double(get_config_double("value"));
	    } else if (type == "list") {
		std::vector<std::string> vec = get_config_vector("value");
		std::vector<OmIndexerData> newvec;
		std::copy(vec.begin(), vec.end(),
			  back_inserter(newvec));
		value.set_vector(newvec.begin(), newvec.end());
	    }
	}

	void config_modified(const std::string &key)
	{
	    if (key == "value" || key == "type") {
		set_value();
	    }
	}
	void calculate() {
	    set_output("out", value);
	}
};

NODE_BEGIN(OmConstantNode, omconstant)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
