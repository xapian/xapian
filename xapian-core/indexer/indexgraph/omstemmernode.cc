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

#include "om/omindexernode.h"
#include "om/omstem.h"
#include "om/omerror.h"
#include "node_reg.h"

/** Node which stems words.
 *
 *  The omstemmer node applies a stemming algorithm to each word
 *  in an input list.  For the list of available stemming languages,
 *  see the documentation from OmStem.
 *
 *  Inputs:
 *  	in: A list of strings (words) to be stemmed.
 *  	language: The language of the stemmer to invoke.  This input is
 *  		not used if the language is provided as a paramter.
 *
 *  Outputs:
 *  	out: The list of stemmed words.
 *
 *  Parameters:
 *  	language: The language of the stemmer to invoke.  If this parameter
 *  		is present, then the input of the same name is ignored.
 */
class OmStemmerNode : public OmIndexerNode {
    public:
	OmStemmerNode(const OmSettings &config)
		: OmIndexerNode(config),
		  language(config.get("language", "")),
		  language_from_config(language.length() > 0)
	{ }
    private:
	std::string language;
	bool language_from_config;
	void config_modified(const std::string &key)
	{
	    if (key == "language") {
		language = get_config_string(key);
		language_from_config = language.length() > 0;
	    }
	}
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");
	    if (input.is_empty()) {
		set_empty_output("out");
		return;
	    }

	    std::string lang = language_from_config?
		    		   language : get_input_string("language");
	    OmStem stemmer(lang);

	    for (size_t i=0; i<input.get_vector_length(); ++i) {
		OmIndexerMessage &word = input.get_element(i);
		word.set_string(stemmer.stem_word(word.get_string()));
	    }

	    set_output("out", input);
	}
};

NODE_BEGIN(OmStemmerNode, omstemmer)
NODE_INPUT("in", "strings", mt_vector)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
