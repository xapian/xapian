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
 *  value (currently string only) which can be used
 *  to supply another node's input with a particular
 *  value.
 *
 *  Inputs: none
 *
 *  Outputs:
 *  	out: the constant value
 *
 *  Parameters:
 *  	value: the string constant used for output.
 */
class OmConstantNode : public OmIndexerNode {
    public:
	OmConstantNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    // FIXME: allow non-string values too
	    value = config.get("value");
	}
    private:
	std::string value;
	// FIXME: implement config_modified()
	void calculate() {
	    set_output("out", value);
	}
};

NODE_BEGIN(OmConstantNode, omconstant)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
