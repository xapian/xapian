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
	static OmIndexerNode *create(const OmSettings &) {
	    return new ReverseNode();
	}
    private:
	ReverseNode() {};
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
	    Message in = get_input_record("in");

	    Record out;
	    out.name = reverse_string(in->name);
	    if (out.type == mt_string) {
		*out.u.string_val = reverse_string(*out.u.string_val);
	    }

	    set_output_record("out", out);
	}
};

class SplitNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &config) {
	    return new SplitNode();
	}
    private:
	SplitNode() {};

	void calculate() {
	    Message msg = get_input_record("in");
	    Record temp1(*msg);
	    temp1.name += "1";
	    if (temp1.type == mt_string) {
		*temp1.u.string_val += "1";
	    }
	    Record temp2(*msg);
	    temp2.name += "2";
	    if (temp2.type == mt_string) {
		*temp2.u.string_val += "2";
	    }
	    set_output_record("out1", temp1);
	    set_output_record("out2", temp2);
	}
};

class ConcatNode : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &config) {
	    return new ConcatNode();
	}
    private:
	ConcatNode() {};
	void calculate() {
	    Message in1 = get_input_record("in1");
	    Message in2 = get_input_record("in2");

	    Record out;
	    out.name = in1->name + in2->name;
	    out.type = mt_string;
	    if (in1->type == mt_string && in2->type == mt_string) {
		out.u.string_val = new std::string(*in1->u.string_val +
						   *in2->u.string_val);
	    } else {
		out.u.string_val = new std::string("somestring");
	    }

	    set_output_record("out", out);
	}
};

#endif // OM_HGUARD_TESTNODES_H
