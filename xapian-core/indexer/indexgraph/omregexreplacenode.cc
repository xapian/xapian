/* omregexreplacenode.cc: Implementation of the regex replace node
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

/** Node which performs a regular expression search and replace
 *  operation.
 *
 *  The omregexreplace node takes a string (or a list of strings
 *  in the case of omregexreplacelist) and performs a regular
 *  expression search and replace.  For each string, any matches
 *  of the regular expression are replaced by the replace expression.
 *
 *  The replace expression is a string which may optionally contain
 *  references of the form \N, where N is a digit from 0-9.  Such a
 *  reference is replaced by the substring matching the Nth
 *  bracketed subexpression in the regular expression (or the whole
 *  matching string if N is 0).
 *
 *  For example, with the regular expression '\([a-z]*\)-\([a-z]*\)'
 *  and replace expression '\2-\1', the string "a-b foo-bar" becomes
 *  "b-a bar-foo".
 *
 *  Inputs:
 *  	in: The input string (for omregexreplace) or list of strings (for
 *  		omregexreplacelist).
 *  	regex: The regular expression to use, in POSIX syntax.  Will be
 *  		ignored if the parameter is specified.
 *  	replace_expr: The replace expression.  Will be ignored if the
 *  		parameter is specified.
 *
 *  Outputs:
 *  	output: The replaced string (for omregexreplace) or list of
 *  		replaced strings (for omregexreplacelist).
 *
 *  Parameters:
 *  	regex: The regular expression used for matching.  The syntax is
 *  		the standard POSIX regular expression syntax.  This
 *  		parameter, if specified, causes the regex input to be
 *  		ignored.
 *  	replace_expr: The replace expression.  If specified, the input
 *  		of the same name is not used.
 */
class OmRegexReplaceNode : public OmIndexerNode {
    public:
	OmRegexReplaceNode(const OmSettings &config)
		: OmIndexerNode(config),
		  config_regex(config.get("regex", "")),
		  replace_expr(config.get("replace_expr", "")),
		  regex_from_config(config_regex.length() > 0),
		  replace_from_config(replace_expr.length() > 0)
	{
	    if (regex_from_config) {
		regex.set(config_regex);
	    }
	}
    private:
	std::string config_regex;
	std::string replace_expr;
	bool regex_from_config;
	bool replace_from_config;
	Regex regex;
	void config_modified(const std::string &key)
	{
	    if (key == "regex") {
		config_regex = get_config_string(key);
		regex_from_config = config_regex.length() > 0;
		if (regex_from_config) {
		    regex.set(config_regex);
		}
	    } else if (key == "replace_expr") {
		replace_expr = get_config_string(key);
		replace_from_config = replace_expr.length() > 0;
	    }
	}
	void calculate() {
	    request_inputs();
	    OmIndexerMessage input = get_input_record("in");

	    if (!regex_from_config) {
		regex.set(get_input_string("regex"));
	    }
	    if (!replace_from_config) {
		replace_expr = get_input_string("replace_expr");
	    }

	    switch (input.get_type()) {
		case OmIndexerMessage::rt_empty:
		    {
			// propagate empty result
			set_empty_output("out");
			return;
		    }
		case OmIndexerMessage::rt_vector:
		    {
			OmIndexerMessage output;
			output.set_vector();

			for (size_t i=0; i<input.get_vector_length(); ++i) {
			    std::string orig = input.get_element(i).get_string();
			    output.append_element(do_replace(orig));
			}
			set_output("out", output);
		    }
		    break;
		case OmIndexerMessage::rt_string:
		    set_output("out", do_replace(input.get_string()));
		    break;
		default:
		    throw OmInvalidDataError("OmRegexReplaceNode: expected string or vector!");
	    }
	}

	std::string do_replace(std::string orig) {
	    std::string replaced;
	    while (orig.length() > 0 && regex.matches(orig)) {
		size_t start = regex.match_start(0);
		size_t end = regex.match_end(0);
		replaced += orig.substr(0, start);
		if (end < orig.length()) {
		    orig = orig.substr(end);
		} else {
		    orig = "";
		}
		replaced += replace_match(replace_expr,
					  regex);
	    }
	    replaced += orig;
	    return replaced;
	}

	std::string replace_match(const std::string &replace,
				  const Regex &regex)
	{
	    std::string result;
	    std::string::size_type i = 0;
	    while (i < replace.length()) {
		if (replace[i] == '\\') {
		    ++i;
		    if (i >= replace.length()) {
			result += '\\';
			continue;
		    }
		    if (isdigit(replace[i])) {
			size_t match_no = replace[i] - '0';
			result += regex.match_string(match_no);
		    } else {
			result += '\\';
			result += replace[i];
		    }
		    ++i;
		} else {
		    result += replace[i];
		    ++i;
		}
	    }
	    return result;
	}
};

/* We register this node twice - once as a node operating on strings,
 * and again as one operating on lists.  The code itself can tell which
 * it's given.
 */
NODE_BEGIN(OmRegexReplaceNode, omregexreplace)
NODE_INPUT("in", "string", mt_string)
NODE_INPUT("regex", "string", mt_string)
NODE_INPUT("replace_expr", "string", mt_string)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()

NODE_BEGIN(OmRegexReplaceNode, omregexreplacelist)
NODE_INPUT("in", "strings", mt_vector)
NODE_INPUT("regex", "string", mt_string)
NODE_INPUT("replace_expr", "string", mt_string)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
