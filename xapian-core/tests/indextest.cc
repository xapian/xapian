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
#include "om/omstem.h"

bool test_basic1()
{
    OmIndexerBuilder builder;

    return true;
}

bool test_basic2()
{
    OmIndexerBuilder builder;

    builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<output node='START' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    return true;
}

bool test_basic3()
{
    bool success = false;

    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<output node='START' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    indexer->set_input(OmIndexerMessage(new OmIndexerData("garbage")));
    OmIndexerMessage result = indexer->get_raw_output();
    if (result->get_string() == "garbage") {
        // garbage in, garbage out
	success = true;
    }

    return success;
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
             "</omindexer>\n"));

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

bool test_omsplitter1()
{
    bool success = false;

    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omsplitter' id='only'>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
         "<output node='only' out_name='right'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    indexer->set_input(OmIndexerMessage(new OmIndexerData("garbage")));
    OmIndexerMessage result = indexer->get_raw_output();
    if (result->get_string() == "garbage") {
        // garbage in, garbage out
	success = true;
    }

    return success;
}

bool test_omstemmer1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omstemmer' id='only'>\n"
	     "<param type='string' name='language' value='english'/>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
         "<output node='only' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    OmIndexerMessage result;
    OmStem stemmer("english");

#if 0
    // FIXME: should OmStemmerNode work with scalars as well as vectors?
    indexer->set_input(OmIndexerMessage(new OmIndexerData("garbage")));
    OmIndexerMessage result = indexer->get_raw_output();
    if (result->get_string() == stemmer.stem_word("garbage")) {
        // garbage in, garbage out
	return false;
    }
#endif

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("word"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("sponge"));
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("elephants"));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));
    result = indexer->get_raw_output();
    if (verbose && result->get_type() != OmIndexerData::rt_vector) {
        cout << "Non-vector result: " << result << endl;
    }
    if (result->get_vector_length() != v.size()) {
        return false;
    }

    for (int i=0; i<v.size(); ++i) {
        if (result->get_element(i).get_string() !=
			stemmer.stem_word(v[i].get_string())) {
	    return false;
	}
    }

    return true;
}

bool test_omprefix1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omprefix' id='only'>\n"
	     "<param type='string' name='prefix' value='WIB'/>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
         "<output node='only' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    OmIndexerMessage result;

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("word"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("sponge"));
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("elephants"));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));
    result = indexer->get_raw_output();
    if (verbose && result->get_type() != OmIndexerData::rt_vector) {
        cout << "Non-vector result: " << result << endl;
    }
    if (result->get_vector_length() != v.size()) {
        return false;
    }

    for (int i=0; i<v.size(); ++i) {
        if (result->get_element(i).get_string() !=
			std::string("WIB") + v[i].get_string()) {
	    return false;
	}
    }

    return true;
}

bool test_omstopword1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omstopword' id='only'>\n"
	     "<param type='list' name='stopwords'>\n"
	         "<item value='stop1'/>\n"
		 "<item value='2stop'/>\n"
	     "</param>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
         "<output node='only' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    OmIndexerMessage result;

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("2stop"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("stop1"));
    v.push_back(OmIndexerData("elephants"));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));
    result = indexer->get_raw_output();
    if (verbose && result->get_type() != OmIndexerData::rt_vector) {
        cout << "Non-vector result: " << result << endl;
    }
    if (result->get_vector_length() != v.size() - 2) {
        return false;
    }

    for (int i=0; i<v.size()-2; ++i) {
        if (result->get_element(i).get_string() !=
			v[i*2].get_string()) {
	    if (verbose) {
		cout << "Result: " << result << endl;
	    }
	    return false;
	}
    }

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"basic1",		&test_basic1},
    {"basic2",		&test_basic2},
    {"basic3",		&test_basic3},
    {"flowcheck1",	&test_flowcheck1},
    {"omsplitter1",	&test_omsplitter1},
    {"omstemmer1",	&test_omstemmer1},
    {"omprefix1",	&test_omprefix1},
    {"omstopword1",	&test_omstopword1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
