/* omnewkeylistnode.cc: Node which makes a blank keylist
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <config.h>
#include "om/omindexernode.h"
#include "node_reg.h"
#include <cctype>

/** Node which creates a new keylist structure.
 *
 *  The omnewkeylist node creates a new blank keylist object which can be
 *  used with omkeylistadd and ommakedoc nodes.
 *
 *  Inputs: none
 *
 *  Outputs:
 *  	out: The empty keylist.
 *
 *  Parameters: none
 */
class OmNewKeylistNode : public OmIndexerNode {
    public:
	OmNewKeylistNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	}
    private:
	void calculate() {
	    OmIndexerMessage terms;
	    terms.set_vector();
	    terms.append_element(std::string("keylist"));
	    set_output("out", terms);
	}
};

NODE_BEGIN(OmNewKeylistNode, omnewkeylist)
NODE_OUTPUT("out", "keys", mt_vector)
NODE_END()
