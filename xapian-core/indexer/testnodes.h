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

class ReverseNode : public OmIndexerNode {
    public:
	ReverseNode() {
	    add_output("out", &ReverseNode::get_reversed);
	}
    private:
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

	Message get_reversed(void) {
	    Message in = get_input("in");

	    Message out(new BasicMessage());
	    out->name = reverse_string(in->name);
	    out->value = reverse_string(in->value);

	    return out;
	}
};

class SplitNode : public OmIndexerNode {
    public:
	SplitNode() {
	    add_output("out1", &SplitNode::get_out1);
	    add_output("out2", &SplitNode::get_out2);
	}
    private:
	Message saved_in;

	Message get_out1(void) {
	    if (!saved_in.get()) {
		Message temp(get_input("in"));
		saved_in = temp;
	    }

	    Message out(new BasicMessage());
	    out->name = saved_in->name + "1";
	    out->value = saved_in->value + "1";

	    return out;
	}

	Message get_out2(void) {
	    if (!saved_in.get()) {
		Message temp(get_input("in"));
		saved_in = temp;
	    }

	    Message out(new BasicMessage());
	    out->name = saved_in->name + "2";
	    out->value = saved_in->value + "2";

	    return out;
	}
};

class ConcatNode : public OmIndexerNode {
    public:
	ConcatNode() {
	    add_output("out", &ConcatNode::get_out);
	}
    private:
	Message get_out(void) {
	    Message in1 = get_input("in1");
	    Message in2 = get_input("in2");

	    Message out(new BasicMessage());
	    out->name = in1->name + in2->name;
	    out->value = in1->value + in2->value;

	    return out;
	}
};

#endif // OM_HGUARD_TESTNODES_H
