/* omtermlistaddnode.cc: Node which adds words to a termlist
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
#include "om/omerror.h"

/** Node which adds items to a termlist structure.
 *
 *  The omtermlistadd node adds a list of strings to a termlist as new
 *  terms.
 *
 *  Inputs:
 *  	termlist: The termlist to act on.  A termlist can be created by
 *  		an omnewtermlist node.
 *	words: The new terms to add to the termlist.  If invokes as an
 *		omtermlistadd node, then this is a vector of strings.
 *		If invoked as omtermlistaddone, then this is a single
 *		string.
 *
 *  Outputs:
 *  	out: The new termlist with added elements.
 *
 *  Parameters: none
 */
class OmTermlistAddNode : public OmIndexerNode {
    public:
	OmTermlistAddNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    // FIXME: parameters to set positional defaults?
	}
    private:
	void calculate() {
	    request_inputs();
	    OmIndexerMessage terms = get_input_record("termlist");
	    OmIndexerMessage words = get_input_record("words");

	    switch (words->get_type()) {
		case OmIndexerData::rt_vector:
		    for (size_t i = 0; i<words->get_vector_length(); ++i) {
			terms->append_element(make_term(words->get_element(i), i+1));
		    }
		    break;
		case OmIndexerData::rt_string:
		    terms->append_element(make_term(*words, 1));
		    break;
		default:
		    throw OmTypeError(std::string("Bad data given to termlistadd node"));
	    }
	    set_output("out", terms);
	}

	OmIndexerData make_term(const OmIndexerData &word, int pos) {
	    std::vector<OmIndexerData> empty;
	    OmIndexerData retval(empty);
	    retval.append_element(word);
	    retval.append_element(1);  // wdf
	    retval.append_element(1);  // termfreq
	    OmIndexerData positions(empty);
	    positions.append_element(pos);
	    retval.append_element(positions);

	    return retval;
	}
};

/* version 1: add a vector */
NODE_BEGIN(OmTermlistAddNode, omtermlistadd)
NODE_INPUT("words", "strings", mt_vector)
NODE_INPUT("termlist", "terms", mt_vector)
NODE_OUTPUT("out", "terms", mt_vector)
NODE_END()

/* version 2: add a string */
NODE_BEGIN(OmTermlistAddNode, omtermlistaddone)
NODE_INPUT("words", "string", mt_string)
NODE_INPUT("termlist", "terms", mt_vector)
NODE_OUTPUT("out", "terms", mt_vector)
NODE_END()
