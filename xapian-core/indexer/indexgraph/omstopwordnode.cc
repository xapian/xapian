/* omstopwordnode.cc: Implementation of a stopword node
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
#include "node_reg.h"
#include <set>

class OmStopWordNode : public OmIndexerNode {
    public:
	OmStopWordNode(const OmSettings &config)
		: OmIndexerNode(config)
	{
	    static const char *const swords[] = {
		"he", "her", "herself", "hi", "him", "himself", "it",
		"itself", "me", "my", "myself", "our", "ourselv", "she",
		"they", "their", "them", "themselv", "we", "you", "your",
		"yourself", "yourselv", "am", "are", "be", "been", "can",
		"could", "did", "do", "doe", "ha", "had", "have", "is",
		"mai", "must", "shall", "should", "wa", "were", "will",
		"would", "becaus", "here", "that", "there", "these", "thi",
		"those", "what", "when", "where", "which", "who", "whom",
		"why", "about", "after", "again", "against", "all", "an",
		"and", "ani", "as", "at", "a", "befor", "below", "between",
		"both", "but", "by", "down", "dure", "each", "few", "first",
		"for", "from", "further", "how", "if", "in", "into", "more",
		"most", "no", "nor", "not", "of", "off", "on", "onc", "one",
		"onli", "or", "other", "out", "over", "own", "per", "same",
		"so", "some", "such", "than", "the", "then", "through",
		"to", "too", "under", "until", "up", "veri", "while",
		"with", "everi", "least", "less", "mani", "now", "ever",
		"never", "sai", "said", "also", "get", "go", "just", "made",
		"make", "put", "see", "seen", "whether" 
	    };
	    for (unsigned int i=0; i< (sizeof(swords) / sizeof(swords[0])); ++i) {
	        stopwords.insert(swords[i]);
	    }
	}
    private:
	std::set<std::string> stopwords;
	void calculate() {
	    OmIndexerMessage input = get_input_record("in");

	    OmIndexerMessage output(new OmIndexerData("words",
				      std::vector<OmIndexerData>()));

	    for (int i=0; i<input->get_vector_length(); ++i) {
	        std::string word = input->get_element(i).get_string();
	        if (stopwords.find(word) == stopwords.end()) {
		    output->append_element(OmIndexerData("word", word));
		}
	    }

	    set_output("out", output);
	}
};

NODE_BEGIN(OmStopWordNode, omstopword)
NODE_INPUT("in", "words", mt_vector)
NODE_OUTPUT("out", "words", mt_vector)
NODE_END()
