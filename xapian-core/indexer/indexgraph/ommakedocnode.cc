/* ommakedocnode.cc: Node to combine parts of a document into a suitable
 *                   form for turning into an OmDocumentContents.
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
#include "om/omindexermessage.h"
#include "node_reg.h"
#include <cctype>

class OmMakeDocNode : public OmIndexerNode {
    public:
	OmMakeDocNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	}
    private:
	// FIXME: implement config_modified()
	void calculate() {
	    request_inputs();
	    OmIndexerMessage data = get_input_record("data");
	    OmIndexerMessage terms = get_input_record("terms");
	    OmIndexerMessage keys = get_input_record("keys");

	    OmIndexerMessage output(new OmIndexerData(
				    std::vector<OmIndexerData>()));
	    output->append_element(*data);

	    std::vector<OmIndexerData> empty;
	    OmIndexerData termvec(empty);
	    termvec.append_element(OmIndexerData("termlist"));
	    for (size_t i=0; i<terms->get_vector_length(); ++i) {
		termvec.append_element(terms->get_element(i));
	    }
	    output->append_element(termvec);

	    OmIndexerData keyvec(empty);
	    keyvec.append_element(OmIndexerData("keylist"));
	    for (size_t i=0; i<keys->get_vector_length(); ++i) {
		keyvec.append_element(keys->get_element(i));
	    }
	    output->append_element(keyvec);

	    set_output("out", output);
	}
};

NODE_BEGIN(OmMakeDocNode, ommakedoc)
NODE_INPUT("terms", "terms", mt_vector)
NODE_INPUT("keys", "strings", mt_vector)
NODE_INPUT("data", "string", mt_string)
NODE_OUTPUT("out", "document", mt_vector)
NODE_END()
