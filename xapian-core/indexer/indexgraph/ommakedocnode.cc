/* ommakedocnode.cc: Node to combine parts of a document into a suitable
 *                   form for turning into an OmDocument.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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
#include "om/omindexermessage.h"
#include "node_reg.h"
#include <cctype>

/** Node which makes the final document representation.
 *
 *  The ommakedoc node combines a key list, term list, and document
 *  data into the form expected by the indexer output.
 *
 *  Inputs:
 *  	terms: A term list, in the form used by omnewtermlist and
 *  		omtermlistadd.
 *  	keys: A key list, in the form used by omnewkeylist and
 *  		omkeylistadd.
 *  	data: Arbitrary document data, as a string.
 *
 *  Outputs:
 *  	out: The document structure.  This is normally connected to
 *  		the indexer's output, rather than to any node.
 *
 *  Parameters: none
 */
class OmMakeDocNode : public OmIndexerNode {
    public:
	OmMakeDocNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	}
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage data = get_input_record("data");
	    OmIndexerMessage terms = get_input_record("terms");
	    OmIndexerMessage keys = get_input_record("keys");

	    OmIndexerMessage output;
	    output.set_vector();
	    output.append_element(data);
	    output.append_element(terms);
	    output.append_element(keys);

	    set_output("out", output);
	}
};

NODE_BEGIN(OmMakeDocNode, ommakedoc)
NODE_INPUT("terms", "terms", mt_vector)
NODE_INPUT("keys", "keys", mt_vector)
NODE_INPUT("data", "string", mt_string)
NODE_OUTPUT("out", "document", mt_vector)
NODE_END()
