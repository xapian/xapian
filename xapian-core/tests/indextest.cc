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

// srcdir, used by some tests
std::string srcdir;

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

AutoPtr<OmIndexer> make_indexer_one_node(const std::string &type,
					 const std::string &input = "in",
					 const std::string &output = "out",
					 const std::string &param = "")
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      std::string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='") + type + "' id='only'>\n"
	     + param + 
	     ((input.length() > 0)?
	       std::string("<input name='")
	       + input + "' node='START' out_name='out'/>\n"
	     : std::string("")) + 
	 "</node>\n"
         "<output node='only' out_name='" + output + "'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

      return indexer;
}

bool test_omsplitter1()
{
    bool success = false;

    AutoPtr<OmIndexer> indexer = make_indexer_one_node("omsplitter",
						       "in", "right");

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

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omstemmer", "in", "out",
		"<param type='string' name='language' value='english'/>\n");

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

    for (unsigned i=0; i<v.size(); ++i) {
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

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omprefix", "in", "out",
	     "<param type='string' name='prefix' value='WIB'/>\n");

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

    for (unsigned i=0; i<v.size(); ++i) {
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

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omstopword", "in", "out",
	     "<param type='list' name='stopwords'>\n"
	         "<item value='stop1'/>\n"
		 "<item value='2stop'/>\n"
	     "</param>\n");

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

    for (unsigned i=0; i<v.size()-2; ++i) {
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

bool test_omflattenstring1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = make_indexer_one_node("omflattenstring");

    OmIndexerMessage result;

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("elephants"));

    std::vector<OmIndexerData> v2;
    v2.push_back(OmIndexerData("nested1"));
    v2.push_back(OmIndexerData("nested2"));

    v.push_back(OmIndexerData(v2));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));
    result = indexer->get_raw_output();
    if (verbose && result->get_type() != OmIndexerData::rt_string) {
        cout << "Non-string result: " << result << endl;
    }
    if (result->get_string() != "penguinsflyingelephantsnested1nested2") {
	if (verbose) {
	    cout << "Bad result: `" << result << "'" << endl;
	}
        return false;
    }

    return true;
}

bool test_omtranslate1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omtranslate", "in", "out",
	     "<param type='string' name='from' value='abcdefghijklmnopqrstuvwxyz'/>\n"
	     "<param type='string' name='to' value='nopqrstuvwxyzabcdefghijklm'/>\n");

    OmIndexerMessage result;

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("word"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("sponge"));
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("elephants"));

    std::vector<OmIndexerData> answer;
    // rot13 versions
    answer.push_back(OmIndexerData("jbeq"));
    answer.push_back(OmIndexerData("sylvat"));
    answer.push_back(OmIndexerData("fcbatr"));
    answer.push_back(OmIndexerData("crathvaf"));
    answer.push_back(OmIndexerData("ryrcunagf"));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));
    result = indexer->get_raw_output();
    if (verbose && result->get_type() != OmIndexerData::rt_vector) {
        cout << "Non-vector result: " << result << endl;
    }
    if (result->get_vector_length() != v.size()) {
        return false;
    }

    for (unsigned i=0; i<v.size(); ++i) {
        if (result->get_element(i).get_string() != answer[i].get_string()) {
	    return false;
	}
    }

    return true;
}

bool
test_omfilereader1()
{
    std::string datadir = srcdir + "/testdata/";

    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omfilereader", "", "out",
		std::string("<param type='string' name='filename' value='") +
	           datadir + "indextest_filereader.txt'/>\n");

    OmIndexerMessage result = indexer->get_raw_output();

    if (verbose && result->get_type() != OmIndexerData::rt_string) {
        cout << "Non-string result: " << result << endl;
	return false;
    }

    if (result->get_string() != "correct answer\n") {
	if (verbose) {
	    cout << "Got string: `" << result->get_string() << "'" << endl;
	}
	return false;
    }

    return true;
}

bool test_omfilereader2()
{
    std::string datadir = srcdir + "/testdata/";

    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = 
	    make_indexer_one_node("omfilereader", "filename");

    indexer->set_input(OmIndexerMessage(new OmIndexerData(datadir
				+ "indextest_filereader.txt")));

    OmIndexerMessage result = indexer->get_raw_output();

    if (verbose && result->get_type() != OmIndexerData::rt_string) {
        cout << "Non-string result: " << result << endl;
	return false;
    }

    if (result->get_string() != "correct answer\n") {
	if (verbose) {
	    cout << "Got string: `" << result->get_string() << "'" << endl;
	}
	return false;
    }

    return true;
}

bool test_omvectorsplit1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer =
	    make_indexer_one_node("omvectorsplit");

    OmIndexerMessage result;

    std::vector<OmIndexerData> v;
    v.push_back(OmIndexerData("word"));
    v.push_back(OmIndexerData("flying"));
    v.push_back(OmIndexerData("sponge"));
    v.push_back(OmIndexerData("penguins"));
    v.push_back(OmIndexerData("elephants"));

    // now test with a vector
    indexer->set_input(OmIndexerMessage(new OmIndexerData(v)));

    for (size_t i=0; i<v.size(); ++i) {
	result = indexer->get_raw_output();
	if (verbose && result->get_type() != OmIndexerData::rt_string) {
	    cout << "Non-string result: " << result << endl;
	}
	if (result->get_string() != v[i].get_string()) {
	    if (verbose) {
		cout << "Got " << result << ", expected " << v[i] << endl;
	    }
	    return false;
	}
    }
    result = indexer->get_raw_output();
    if (result->get_type() != OmIndexerData::rt_empty) {
	if (verbose) {
	    cout << "Expected empty at end, got: " << result << endl;
	}
	return false;
    }

    return true;
}

bool test_omlistconcat1()
{
    OmIndexerBuilder builder;

    AutoPtr<OmIndexer> indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omsplitter' id='split'>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
         "<node type='omlistconcat' id='only'>\n"
	     "<input name='left' node='split' out_name='left'/>\n"
	     "<input name='right' node='split' out_name='right'/>\n"
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
    if (result->get_vector_length() != 2*v.size()) {
        return false;
    }

    for (unsigned i=0; i<v.size(); ++i) {
        if (result->get_element(i).get_string() != v[i].get_string()) {
	    return false;
	}
        if (result->get_element(i + v.size()).get_string() != v[i].get_string()) {
	    return false;
	}
    }

    return true;
}

bool
test_badnode1()
{
    bool success = false;
    try {
	make_indexer_one_node("non-existant");
    } catch (OmInvalidDataError &) {
	success = true;
    }
    return success;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"basic1",			&test_basic1},
    {"basic2",			&test_basic2},
    {"basic3",			&test_basic3},
    {"flowcheck1",		&test_flowcheck1},
    {"omsplitter1",		&test_omsplitter1},
    {"omstemmer1",		&test_omstemmer1},
    {"omprefix1",		&test_omprefix1},
    {"omstopword1",		&test_omstopword1},
    {"omflattenstring1",	&test_omflattenstring1},
    {"omtranslate1",		&test_omtranslate1},
    {"omfilereader1",		&test_omfilereader1},
    {"omfilereader2",		&test_omfilereader2},
    {"omvectorsplit1",		&test_omvectorsplit1},
    {"omlistconcat1",		&test_omlistconcat1},
    {"badnode1",		&test_badnode1},
    // FIXME: add tests for regex nodes
    {0, 0}
};

int main(int argc, char *argv[])
{
    srcdir = test_driver::get_srcdir(argv[0]);

    return test_driver::main(argc, argv, tests);
}
