/* omsplitoncharsnode.cc: Implementation of the character-based word split
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

/** Node which splits a string into separate words.
 *
 *  This node splits a string into words.  A word is defined as a
 *  non-empty string of characters from a "word character" set.
 *  This performs a subset of functions that the omregexsplit node
 *  performs, but should be more efficient for those cases.
 *
 *  Example: input string 'foo bar, wibble,  wobble' with wordchars
 *  'a-z' would return ["foo", "bar", "wibble", "wobble"].
 *
 *  Inputs:
 *  	in: The input string 
 *  	wordchars: The set of characters which are "word" characters
 *  	       (as opposed to "separator" words).  The set is specified
 *  	       in the same way as a regular expression character set,
 *  	       so "a-ej" is the characters from "a" to "e" and the
 *  	       character "j".  An initial "^" causes the set to be
 *  	       completented, ie characters specified become separator
 *  	       characters, and the rest are word characters.  Brackets
 *  	       ("[" and "]") have no special meaning.  A "-" character
 *  	       at the start (or just after the "^") or end of the string
 *  	       loses its special meaning.
 *  	       This input is ignored if the parameter is specified.
 *
 *  Outputs:
 *  	output: The list of strings between matches of the regular
 *  		expression.
 *
 *  Parameters:
 *  	wordchars: The character class matching word characters.  (See
 *  		the description of the input of the same name). This
 *  		parameter, if specified, causes the wordchars input to be
 *  		ignored.
 */
class OmSplitOnCharsNode : public OmIndexerNode {
    public:
	OmSplitOnCharsNode(const OmSettings &config)
		: OmIndexerNode(config),
		  config_wchars(config.get("wordchars", "")),
		  chars_from_config(config_wchars.length() > 0)
	{
	    if (chars_from_config) {
		init_isword(config_wchars);
	    }
	}
    private:
	// FIXME: needs to know about unicode
	std::string config_wchars;
	bool chars_from_config;
	bool isword[256];
	void config_modified(const std::string &key)
	{
	    if (key == "wordchars") {
		config_wchars = get_config_string(key);
		chars_from_config = config_wchars.length() > 0;
		if (chars_from_config) {
		    init_isword(config_wchars);
		}
	    }
	}

	void init_isword(const std::string &s)
	{
	    bool default_word = false;
	    std::string::size_type pos = 0;
	    if (s.length() > 0 && s[0] == '^') {
		default_word = true;
		pos++;
	    }
	    for (int i=0; i<256; ++i) {
		isword[i] = default_word;
	    }
	    int last_char = -1;
	    while (pos < s.length()) {
		int charnum = static_cast<unsigned char>(s[pos]);
		if (s[pos] == '-') {
		    if (last_char == -1) {
			isword[charnum] = !default_word;
		    } else if (pos == s.length() - 1) {
			isword[charnum] = !default_word;
		    } else {
			/* Character range */
			int newcharnum = static_cast<unsigned char>(s[pos+1]);
			for (int i=charnum + 1; i<=newcharnum; ++i) {
			    isword[i] = !default_word;
			}
			pos++;
		    }
		} else {
		    isword[charnum] = !default_word;
		}
		last_char = charnum;
		pos++;
	    }
	}
	void calculate() {
	    request_inputs();
	    OmIndexerMessage mess = get_input_record("in");
	    if (mess->get_type() == OmIndexerData::rt_empty) {
		set_empty_output("out");
		return;
	    }
	    std::string input = mess->get_string();

	    if (!chars_from_config) {
		init_isword(get_input_string("wordchars"));
	    }

	    OmIndexerMessage output(new OmIndexerData(
				      std::vector<OmIndexerData>()));

	    // current position in input string
	    std::string::size_type pos = 0;
	    std::string::size_type last = input.length();
	    while (pos < last) {
		while (pos < last &&
		       !isword[static_cast<unsigned char>(input[pos])]) {
		    /* skip over non-word characters */
		    pos++;
		}
		std::string::size_type word_start = pos;
		std::string::size_type len = 0;
		while (pos < last &&
		       isword[static_cast<unsigned char>(input[pos])]) {
		    pos++;
		    len++;
		}
		if (len > 0) {
		    output->append_element(input.substr(word_start, len));
		}
	    }
	    set_output("out", output);
	}
};

NODE_BEGIN(OmSplitOnCharsNode, omsplitonchars)
NODE_INPUT("in", "string", mt_string)
NODE_INPUT("wordchars", "string", mt_string)
NODE_OUTPUT("out", "strings", mt_vector)
NODE_END()
