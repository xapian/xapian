/* indextest.cc: test of the OpenMuscat indexing system
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
#include <iostream>
#include <string>
using std::cout;
using std::endl;

#include "om/om.h"
#include "om/omindexerbuilder.h"
#include "testsuite.h"

bool test_basic1()
{
    OmIndexerBuilder builder;

    return true;
}

bool test_basic2()
{
    OmIndexerBuilder builder;

    builder.build_from_string("<?xml version=\"1.0\"?><!DOCTYPE omindexer SYSTEM '../../om/indexer/indexgraph/omindexer.dtd'><omindexer><output node='START' out_name='out'/></omindexer>");

    return true;
}

class readtwice : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new readtwice(settings);
	}
    private:
	readtwice(const OmSettings &settings)
		: OmIndexerNode(settings) {}

	void calculate() {
	    request_inputs();
	    get_input_record("in");
	    request_inputs();
	    set_output("out", get_input_record("in"));
	}
};

class writetwice : public OmIndexerNode {
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new writetwice(settings);
	}
    private:
	writetwice(const OmSettings &settings)
		: OmIndexerNode(settings), saved_msg(0), have_msg(false) {}

	OmIndexerMessage saved_msg;
	bool have_msg;

	void calculate() {
	    if (have_msg) {
		set_output("out", saved_msg);
		have_msg = false;
	    } else {
		request_inputs();
		OmIndexerMessage in = get_input_record("in");
		saved_msg = OmIndexerMessage(new OmIndexerData(*in));
		have_msg = true;
		set_output("out", in);
	    }
	}
};

/** Test that the checking for data mismatches in the indexer data flow works
 */
bool test_flowcheck1()
{
    bool success = false;
    try {
	OmIndexerBuilder builder;

	{
	    OmNodeDescriptor ndesc("readtwice", &readtwice::create);
	    ndesc.add_input("in", "*1", mt_record);
	    ndesc.add_output("out", "*1", mt_record);
	    builder.register_node_type(ndesc);
	}
	{
	    OmNodeDescriptor ndesc("writetwice", &writetwice::create);
	    ndesc.add_input("in", "*1", mt_record);
	    ndesc.add_output("out", "*1", mt_record);
	    builder.register_node_type(ndesc);
	}

	AutoPtr<OmIndexer> indexer(builder.build_from_string(
	     "<?xml version=\"1.0\"?>"
	     "<!DOCTYPE omindexer SYSTEM '../../om/indexer/indexgraph/omindexer.dtd'>"
	     "<omindexer>"
	         "<node type='writetwice' id='wtwo'>"
		      "<input name='in' node='START' out_name='out'/>"
		 "</node>"
	         "<node type='omsplitter' id='split'>"
		      "<input name='in' node='wtwo' out_name='out'/>"
		 "</node>"
	         "<node type='readtwice' id='rtwo'>"
		      "<input name='in' node='split' out_name='right'/>"
		 "</node>"
	         "<node type='omlistconcat' id='concat'>"
		      "<input name='left' node='split' out_name='left'/>"
		      "<input name='right' node='rtwo' out_name='out'/>"
		 "</node>"
	         "<output node='concat' out_name='out'/>"
             "</omindexer>"));

	indexer->set_input(OmIndexerMessage(new OmIndexerData(vector<OmIndexerData>())));
	OmIndexerMessage result = indexer->get_raw_output();
	if (verbose) {
	    cerr << "got output: " << result << endl;
	}
    } catch (OmDataFlowError &) {
	success = true;
    }
    return success;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"basic1",		&test_basic1},
    {"basic2",		&test_basic2},
    {"flowcheck1",	&test_flowcheck1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
