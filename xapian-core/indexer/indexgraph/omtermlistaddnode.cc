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

class OmTermlistAddNode : public OmIndexerNode {
    public:
	OmTermlistAddNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    // FIXME: parameters to set positional defaults?
	}
    private:
	// FIXME: implement config_modified()
	void calculate() {
	    request_inputs();
	    OmIndexerMessage terms = get_input_record("termlist");
	    OmIndexerMessage words = get_input_record("words");

	    for (size_t i = 0; i<words->get_vector_length(); ++i) {
		terms->append_element(make_term(words->get_element(i), i+1));
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

NODE_BEGIN(OmTermlistAddNode, omtermlistadd)
NODE_INPUT("words", "strings", mt_vector)
NODE_INPUT("termlist", "terms", mt_vector)
NODE_OUTPUT("out", "terms", mt_vector)
NODE_END()
