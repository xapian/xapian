/* omregexmatchnode.cc: Implementation of the regex match node
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
#include "omdebug.h"

/** Node which matches a regular expression against a string and
 *  returns the list of submatches.
 *
 *  The omregexmatch node takes a string or a list of strings as
 *  input.  For each string, a regular expression match is run to
 *  produce a list of output strings.
 *
 *  If the regular expression does not match, then the output list
 *  is empty.  If it does match, then element 0 will be the substring
 *  of the input matching the entire regular expression.  Each
 *  subsequent element will be the substring matching a particular
 *  bracketed subexpression.  For example, using the regular expression
 *  '\([A-Z]\+\)=\(.*\)' with the string "1 FOO=wibble" will give the
 *  output list: ["FOO=wibble", "FOO", "wibble"]
 *
 *  Inputs:
 *  	in: The input string or list of strings.
 *  	regex: The regular expression to use, in POSIX syntax.  Will be
 *  		ignored if the parameter is specified.
 *
 *  Outputs:
 *  	output: The output list of submatches (if input is a string) or
 *  		list of lists of submatches (if input is a list).
 *
 *  Parameters:
 *  	regex: The regular expression used for matching.  The syntax is
 *  		the standard POSIX regular expression syntax.  This
 *  		parameter, if specified, causes the regex input to be
 *  		ignored.
 */
class OmRegexMatchNode : public OmIndexerNode {
    public:
	OmRegexMatchNode(const OmSettings &config)
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

	    switch (input.get_type()) {
		case OmIndexerMessage::rt_empty:
		    {
			// propagate empty result
			set_output("out", input);
			return;
		    }
		    break;
		case OmIndexerMessage::rt_vector:
		    {
			OmIndexerMessage output;
			output.set_vector();

			for (size_t i=0; i<input.get_vector_length(); ++i) {
			    std::string orig = input.get_element(i).get_string();
			    output.append_element(do_getmatches(orig));
			}
			set_output("out", output);
		    }
		    break;
		case OmIndexerMessage::rt_string:
		    set_output("out", do_getmatches(input.get_string()));
		    break;
		default:
		    throw OmInvalidDataError("OmRegexMatchNode: expected string or vector!");
	    }
	}
	OmIndexerMessage do_getmatches(const std::string &s) {
	    OmIndexerMessage results;
	    results.set_vector();

	    if (regex.matches(s)) {
		DEBUGLINE(INDEXER, "Regex `" << regex.get_pattern()
			  << "' matches `" << s << "'");
		for (int i=0; i<regex.num_subexprs(); ++i) {
		    if (regex.submatch_defined(i)) {
			std::string ms(regex.match_string(i));
			DEBUGLINE(INDEXER, "Regex submatch: `" << ms << "'");
			results.append_element(ms);
		    } else {
			DEBUGLINE(INDEXER, "Regex submatch undefined");
			results.append_element(OmIndexerMessage(""));
		    }
		}
	    } else {
		DEBUGLINE(INDEXER, "Regular expression `"
			  << regex.get_pattern()
			  << "' didn't match `" << s << "'");
	    }
	    return results;
	}
};

NODE_BEGIN(OmRegexMatchNode, omregexmatch)
NODE_INPUT("in", "*1", mt_record)
NODE_OUTPUT("out", "*2", mt_vector)
NODE_END()
