/* omtranslatenode.cc: Implementation of the translate node.
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
#include "om/omstem.h"
#include "om/omerror.h"
#include "node_reg.h"

/** Node which performs a character translation.
 *
 *  The omtranslate node maps some characters to other characters in the
 *  output.  The omtranslate node works on a string, and the
 *  omtranslatelist works on a list of strings.
 *
 *  The mapping is specified as a "from" string and a "to" string, both
 *  of the same length.  The character at position N in the "from" string
 *  is mapped to the character at position N in the "to" string.
 *  Characters not appearing in "from" stay as they are.
 *
 *  Example: from="abcdef", to="123456".  The string "wibble" would become
 *  "wi22l5".  A common use would be from="ABCDEF...XYZ" and
 *  to="abcdef...xyz", which converts all characters to lower-case.
 *
 *  Inputs:
 *  	in: The input string (for omtranslate) or list of strings (for
 *  		omtranslatelist)
 *
 *  Outputs:
 *  	out: The translated string (for omtranslate) or list of strings
 *  		(for omtranslatelist)
 *
 *  Parameters:
 *  	from: The string of characters to be mapped to other characters.
 *  	to: The characters to map to.
 */
class OmTranslateNode : public OmIndexerNode {
    public:
	OmTranslateNode(const OmSettings &config)
		: OmIndexerNode(config),
		  from(config.get("from")),
		  to(config.get("to"))
	{
	    setup_map();
	}
    private:
	std::string from;
	std::string to;
	// FIXME: this'll need changing when we deal with Unicode.
	char map[256];
	void config_modified(const std::string &key)
	{
	    bool changed = false;
	    if (key == "from") {
		from = get_config_string(key);
		changed = true;
	    } else if (key == "to") {
		to = get_config_string(key);
		changed = true;
	    }
	    if (changed) {
		// FIXME: this will fail if from or to is set to
		// a string of a different length.  Need to be able
		// to set them both atomically, really.
		setup_map();
	    }
	}
	void setup_map()
	{
	    for (int i=0; i<sizeof(map); ++i) {
		map[i] = i;
	    }
	    if (from.length() != to.length()) {
		std::string message("OmTranslateNode: translation strings `");
		message += from;
		message += "' -> `";
		message += to;
		message += "' are of different lengths";
		throw OmInvalidDataError(message);
	    }

	    for (int i=0; i<from.length(); ++i) {
		map[from[i]] = to[i];
	    }
	}
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");

	    switch (input->get_type()) {
		case OmIndexerData::rt_empty:
		    {
			set_empty_output("out");
		    }
		    break;
		case OmIndexerData::rt_vector:
		    {
			OmIndexerMessage output(new OmIndexerData(
				      std::vector<OmIndexerData>()));

			for (int i=0; i<input->get_vector_length(); ++i) {
			    std::string orig = input->get_element(i).get_string();
			    do_translate(orig);
			    output->append_element(OmIndexerData(orig));
			}
			set_output("out", output);
		    }
		    break;
		case OmIndexerData::rt_string:
		    {
			std::string result(input->get_string());
			do_translate(result);
			set_output("out", result);
		    }
		    break;
		default:
		    throw OmInvalidDataError("OmRegexReplaceNode: expected string or vector!");
	    }
	}
	void do_translate(std::string &orig) {
	    for (int i=0; i<orig.length(); ++i) {
		orig[i] = map[orig[i]];
	    }
	}
};

/* We register this node twice - once as a node operating on strings,
 * and again as one operating on lists.  The code itself can tell which
 * it's given.
 */
NODE_BEGIN(OmTranslateNode, omtranslatelist)
NODE_INPUT("in", "strings", mt_vector)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()

NODE_BEGIN(OmTranslateNode, omtranslate)
NODE_INPUT("in", "string", mt_string)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
