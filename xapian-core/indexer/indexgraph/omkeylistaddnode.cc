/* omkeylistaddnode.cc: Node which adds keys to a keylist
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
#include "om/omerror.h"

/** Node which adds items to a keylist structure.
 *
 *  The omkeylistadd node adds a list of strings to a keylist as new
 *  keys.
 *
 *  Inputs:
 *  	keylist: The keylist to act on.  A keylist can be created by
 *  		an omnewkeylist node.
 *	keys: The new keys to add to the keylist.  If invokes as an
 *		omkeylistadd node, then this is a vector of strings.
 *		If invoked as omkeylistaddone, then this is a single
 *		string.
 *
 *  Outputs:
 *  	out: The new keylist with added elements.
 *
 *  Parameters: none
 */
class OmKeylistAddNode : public OmIndexerNode {
    public:
	OmKeylistAddNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    // FIXME: parameters to set positional defaults?
	}
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage keys = get_input_record("keylist");
	    OmIndexerMessage words = get_input_record("keys");

	    switch (words.get_type()) {
		case OmIndexerMessage::rt_vector:
		    for (size_t i = 0; i<words.get_vector_length(); ++i) {
			keys.append_element(make_key(words.get_element(i)));
		    }
		    break;
		case OmIndexerMessage::rt_string:
		    keys.append_element(make_key(words));
		    break;
		default:
		    throw OmTypeError(std::string("Bad data given to keylistadd node"));
	    }
	    set_output("out", keys);
	}

	OmIndexerMessage make_key(const OmIndexerMessage &word) {
	    return word;
	}
};

/* version 1: add a vector */
NODE_BEGIN(OmKeylistAddNode, omkeylistadd)
NODE_INPUT("keys", "strings", mt_vector)
NODE_INPUT("keylist", "keys", mt_vector)
NODE_OUTPUT("out", "keys", mt_vector)
NODE_END()

/* version 2: add a string */
NODE_BEGIN(OmKeylistAddNode, omkeylistaddone)
NODE_INPUT("keys", "string", mt_string)
NODE_INPUT("keylist", "keys", mt_vector)
NODE_OUTPUT("out", "keys", mt_vector)
NODE_END()
