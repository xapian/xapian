/* nodetest.cc: temporary program for trying out OmIndexerNode
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
#include "testnodes.h"
#include "indexergraph.h"
#include "om/omerror.h"

int main() {
    try {
	OmIndexerBuilder builder;
	{
	    OmNodeDescriptor ndesc("split", &SplitNode::create);
	    ndesc.add_input("in", "ANY", mt_record);
	    ndesc.add_output("out1", "ANY", mt_record);
	    ndesc.add_output("out2", "ANY", mt_record);
	    builder.register_node_type(ndesc);
	}
	{
	    OmNodeDescriptor ndesc("reverse", &ReverseNode::create);
	    ndesc.add_input("in", "string", mt_string);
	    ndesc.add_output("out", "string", mt_string);
	    builder.register_node_type(ndesc);
	}
	{
	    OmNodeDescriptor ndesc("concat", &ConcatNode::create);
	    ndesc.add_input("in1", "string", mt_string);
	    ndesc.add_input("in2", "string", mt_string);
	    ndesc.add_output("out", "string", mt_string);
	    builder.register_node_type(ndesc);
	}
	AutoPtr<OmIndexer> indexer = builder.build_from_file("test.xml");
	Message msg(new Record("foo", "bar"));
	indexer->set_input(msg);
	Message result = indexer->get_output();

	cout << "Name: " << result->get_name() << endl;
	switch (result->get_type()) {
	    case Record::rt_int:
		cout << result->get_int();
		break;
	    case Record::rt_double:
		cout << result->get_double();
		break;
	    case Record::rt_string:
		cout << result->get_string();
		break;
	    case Record::rt_vector:
		cout << "Vector";
		break;
	    case Record::rt_empty:
		cout << "Empty";
		break;
	};
	cout << endl;
    } catch (const std::string &s) {
	cout << "Got exception: " << s << endl;
    } catch (const char * s) {
	cout << "Got exception: " << s << endl;
    } catch (char *s) {
	cout << "Got exception: " << s << endl;
    } catch (OmError &e) {
	cout << "Got " << e.get_type() << ": " << e.get_msg() << endl;
    } catch (...) {
	cout << "Got unknown exception" << endl;
    }
}
