/* testnodes.h: Some simple test nodes
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

#ifndef OM_HGUARD_TESTNODES_H
#define OM_HGUARD_TESTNODES_H
#include "omindexernode.h"
#include "om/omsettings.h"

class ReverseNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new ReverseNode(settings);
	}
    private:
	ReverseNode(const OmSettings &settings) : OmIndexerNode(settings) {};
	static std::string reverse_string(std::string s) {
	    std::string result(s);
	    std::string::size_type len = result.length();
	    for (std::string::size_type i = 0;
		 i < (len / 2);
		 ++i) {
		swap(result[i], result[len-1-i]);
	    }
	    return result;
	}

	void calculate() {
	    std::string in = get_input_string("in");

	    std::string out(in);
	    out = reverse_string(in);

	    set_output("out", out);
	}
};

class SplitNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &config) {
	    return new SplitNode(config);
	}
    private:
	SplitNode(const OmSettings &settings) : OmIndexerNode(settings) {};

	void calculate() {
	    OmIndexerMessage msg = get_input_record("in");
	    OmIndexerMessage temp1(new OmIndexerData(msg->get_name() + "1",
				     msg->get_string() + "1"));

	    OmIndexerMessage temp2(new OmIndexerData(msg->get_name() + "2",
				     msg->get_string() + "2"));
	    set_output("out1", temp1);
	    set_output("out2", temp2);
	}
};

class ConcatNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &config) {
	    return new ConcatNode(config);
	}
    private:
	ConcatNode(const OmSettings &settings) : OmIndexerNode(settings) {};
	void calculate() {
	    std::string in1 = get_input_string("in1");
	    std::string in2 = get_input_string("in2");

	    std::string out = in1 + in2;

	    set_output("out", out);
	}
};

#endif // OM_HGUARD_TESTNODES_H
