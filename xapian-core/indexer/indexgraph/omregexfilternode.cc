/* omregexfilternode.cc: Implementation of the regex replace node
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
#include "regexcommon.h"
#include "om/omindexernode.h"
#include "node_reg.h"
#include <cctype>

/** Node which removes strings not matching a regular expression from a
 *  list.
 *
 *  The omregexfilter nodes takes a list of strings as input.  The
 *  output is a (probably) smaller list containing only the elements
 *  of the input list matching a given regular expression specified
 *  as a parameter or input.
 *
 *  Inputs:
 *  	in: The input list of strings
 *  	regex: The regular expression to use, in POSIX syntax.  Will be
 *  		ignored if the parameter is specified.
 *
 *  Outputs:
 *  	output: The filtered list of strings.
 *
 *  Parameters:
 *  	regex: The regular expression used for matching.  The syntax is
 *  		the standard POSIX regular expression syntax.  This
 *  		parameter, if specified, causes the regex input to be
 *  		ignored.
 */
class OmRegexFilterNode : public OmIndexerNode {
    public:
	OmRegexFilterNode(const OmSettings &config)
		: OmIndexerNode(config),
		  config_regex(config.get("regex", "")),
		  regex_from_config(config_regex.length() > 0)
	{
	    if (regex_from_config) {
		regex.set(config_regex);
	    }
	}
    private:
	std::string config_regex;
	bool regex_from_config;
	Regex regex;

	void config_modified(const std::string &key)
	{
	    if (key == "regex") {
		config_regex = get_config_string(key);
		regex_from_config = config_regex.length() > 0;
		if (regex_from_config) {
		    regex.set(config_regex);
		}
	    }
	}
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");

	    if (!regex_from_config) {
		regex.set(get_input_string("regex"));
	    }

	    OmIndexerMessage output;
	    output.set_vector();

	    for (size_t i=0; i<input.get_vector_length(); ++i) {
		std::string orig = input.get_element(i).get_string();
		if (regex.matches(orig)) {
		    output.append_element(orig);
		}
	    }
	    set_output("out", output);
	}
};

NODE_BEGIN(OmRegexFilterNode, omregexfilter)
NODE_INPUT("in", "strings", mt_vector)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
