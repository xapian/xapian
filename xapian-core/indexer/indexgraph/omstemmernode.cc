/* omstemmernode.cc: Implementation of the stemmer node.
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

#include "omindexernode.h"
#include "om/omstem.h"
#include "om/omerror.h"

class OmStemmerNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &config) {
	    return new OmStemmerNode(config);
	}
    private:
	std::string language;
	bool language_from_config;
	OmStemmerNode(const OmSettings &config)
		: OmIndexerNode(config),
		  language(config.get("language", "")),
		  language_from_config(language.length() > 0)
	{ }
	void calculate() {
	    OmIndexerMessage input = get_input_record("in");

	    OmIndexerMessage output(new OmIndexerData("words",
				      std::vector<OmIndexerData>()));

	    std::string lang = language_from_config?
		    		   language : get_input_string("language");
	    OmStem stemmer(lang);

	    for (int i=0; i<input->get_vector_length(); ++i) {
		output->append_element(
	            OmIndexerData("word",
			   stemmer.stem_word(input->get_element(i).get_string())));
	    }

	    set_output("out", output);
	}
};

#if 0
	{
	// FIXME: generalise into configurable stemmer
	    OmNodeDescriptor ndesc("englishstemmer",
				   &EnglishStemmerNode::create);
	    ndesc.add_input("in", "words", mt_vector);
	    ndesc.add_output("out", "words", mt_vector);
	    builder.register_node_type(ndesc);
	}
#endif
