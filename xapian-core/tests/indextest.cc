/* indextest.cc: test of the Omseek indexing system
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
#include <iostream>
#include <string>

#include "om/om.h"
#include "om/omindexerbuilder.h"
#include "om/omnodeinstanceiterator.h"
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

    OmIndexer indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<output node='START' out_name='out'/>\n"
      "</omindexer>\n");
      // FIXME: on PPC, it seems to miss the last character, so complains
      // that there's no final >.  Stop the bodge, and investigate.

    indexer.set_input(OmIndexerMessage("garbage"));
    OmIndexerMessage result = indexer.get_raw_output();
    if (result.get_string() == "garbage") {
        // garbage in, garbage out
	success = true;
    }

    return success;
}

/* Test the copy-on-write behaviour of OmIndexerMessage */
bool test_omindexermessage1()
{
    OmIndexerMessage foo("wibble");

    OmIndexerMessage bar(foo);

    OmIndexerMessage baz;
    baz = bar;

    TEST(foo.get_string() == "wibble");
    TEST(bar.get_string() == "wibble");
    TEST(baz.get_string() == "wibble");

    bar.set_string("wobble");

    TEST(foo.get_string() == "wibble");
    TEST(bar.get_string() == "wobble");
    TEST(baz.get_string() == "wibble");

    foo = OmIndexerMessage("wabble");

    TEST(foo.get_string() == "wabble");
    TEST(bar.get_string() == "wobble");
    TEST(baz.get_string() == "wibble");

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
		saved_msg = in;
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

	OmIndexer indexer(builder.build_from_string(
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
	
	OmIndexerMessage empty;
	empty.set_vector();
	indexer.set_input(empty);
	OmIndexerMessage result = indexer.get_raw_output();
	if (verbose) {
	    std::cerr << "got output: " << result << '\n';
	}
    } catch (OmDataFlowError &) {
	success = true;
    }
    return success;
}

OmIndexerDesc make_indexerdesc_one_node(const std::string &type,
					const std::string &input = "in",
					const std::string &output = "out",
					const std::string &param = "")
{
    OmIndexerBuilder builder;

    OmIndexerDesc desc = builder.desc_from_string(
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

      return desc;
}

OmIndexer make_indexer_one_node(const std::string &type,
				const std::string &input = "in",
				const std::string &output = "out",
				const std::string &param = "")
{
    OmIndexerBuilder builder;

    return builder.build_from_desc(make_indexerdesc_one_node(type,
							     input,
							     output,
							     param));
}

bool test_omsplitter1()
{
    bool success = false;

    OmIndexer indexer = make_indexer_one_node("omsplitter",
						       "in", "right");

    indexer.set_input(std::string("garbage"));
    OmIndexerMessage result = indexer.get_raw_output();
    if (result.get_string() == "garbage") {
        // garbage in, garbage out
	success = true;
    }

    return success;
}

bool test_omstemmer1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer =
	    make_indexer_one_node("omstemmer", "in", "out",
		"<param type='string' name='language' value='english'/>\n");

    OmIndexerMessage result;
    OmStem stemmer("english");

#if 0
    // FIXME: should OmStemmerNode work with scalars as well as vectors?
    indexer.set_input(OmIndexerMessage(new OmIndexerData("garbage")));
    OmIndexerMessage result = indexer.get_raw_output();
    if (result.get_string() == stemmer.stem_word("garbage")) {
        // garbage in, garbage out
	return false;
    }
#endif

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("word"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("sponge"));
    v.append_element(std::string("penguins"));
    v.append_element(std::string("elephants"));

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_vector) {
        tout << "Non-vector result: " << result << '\n';
    }
    if (result.get_vector_length() != v.get_vector_length()) {
        return false;
    }

    for (unsigned i=0; i<v.get_vector_length(); ++i) {
        if (result.get_element(i).get_string() !=
			stemmer.stem_word(v[i].get_string())) {
	    tout << "Stemming test failed at element " << i <<
		    "Got " << result.get_element(i).get_string() <<
		    ", expected " << stemmer.stem_word(v[i].get_string()) <<
		    '\n';
	    tout << "Result = " << result << "\n";
	    return false;
	}
    }

    return true;
}

bool test_omprefix1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer =
	    make_indexer_one_node("omprefix", "in", "out",
	     "<param type='string' name='prefix' value='WIB'/>\n");

    OmIndexerMessage result;

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("word"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("sponge"));
    v.append_element(std::string("penguins"));
    v.append_element(std::string("elephants"));

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_vector) {
        tout << "Non-vector result: " << result << '\n';
    }
    if (result.get_vector_length() != v.get_vector_length()) {
        return false;
    }

    for (unsigned i=0; i<v.get_vector_length(); ++i) {
        if (result.get_element(i).get_string() !=
			std::string("WIB") + v[i].get_string()) {
	    return false;
	}
    }

    return true;
}

bool test_omstopword1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer =
	    make_indexer_one_node("omstopword", "in", "out",
	     "<param type='string' name='stopwords'"
	         "value='stop1 2stop'>\n"
	     "</param>\n");

    OmIndexerMessage result;

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("penguins"));
    v.append_element(std::string("2stop"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("stop1"));
    v.append_element(std::string("elephants"));

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_vector) {
        tout << "Non-vector result: " << result << '\n';
    }
    if (result.get_vector_length() != v.get_vector_length() - 2) {
        return false;
    }

    for (unsigned i=0; i<v.get_vector_length()-2; ++i) {
        if (result.get_element(i).get_string() !=
			v[i*2].get_string()) {
	    if (verbose) {
		tout << "Result: " << result << '\n';
	    }
	    return false;
	}
    }

    return true;
}

bool test_omflattenstring1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer = make_indexer_one_node("omflattenstring");

    OmIndexerMessage result;

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("penguins"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("elephants"));

    OmIndexerMessage v2;
    v2.set_vector();
    v2.append_element(std::string("nested1"));
    v2.append_element(std::string("nested2"));

    v.append_element(v2);

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_string) {
        tout << "Non-string result: " << result << '\n';
    }
    if (result.get_string() != "penguinsflyingelephantsnested1nested2") {
	if (verbose) {
	    tout << "Bad result: `" << result << "'" << '\n';
	}
        return false;
    }

    return true;
}

bool test_omtranslate1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer =
	    make_indexer_one_node("omtranslate", "in", "out",
	     "<param type='string' name='from' value='abcdefghijklmnopqrstuvwxyz'/>\n"
	     "<param type='string' name='to' value='nopqrstuvwxyzabcdefghijklm'/>\n");

    OmIndexerMessage result;

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("word"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("sponge"));
    v.append_element(std::string("penguins"));
    v.append_element(std::string("elephants"));

    OmIndexerMessage answer;
    answer.set_vector();
    // rot13 versions
    answer.append_element(std::string("jbeq"));
    answer.append_element(std::string("sylvat"));
    answer.append_element(std::string("fcbatr"));
    answer.append_element(std::string("crathvaf"));
    answer.append_element(std::string("ryrcunagf"));

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_vector) {
        tout << "Non-vector result: " << result << '\n';
    }
    if (result.get_vector_length() != v.get_vector_length()) {
        return false;
    }

    for (unsigned i=0; i<v.get_vector_length(); ++i) {
        if (result.get_element(i).get_string() != answer[i].get_string()) {
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

    OmIndexer indexer =
	    make_indexer_one_node("omfilereader", "", "out",
		std::string("<param type='string' name='filename' value='") +
	           datadir + "indextest_filereader.txt'/>\n");

    OmIndexerMessage result = indexer.get_raw_output();

    if (verbose && result.get_type() != OmIndexerMessage::rt_string) {
        tout << "Non-string result: " << result << '\n';
	return false;
    }

    if (result.get_string() != "correct answer\n") {
	if (verbose) {
	    tout << "Got string: `" << result.get_string() << "'" << '\n';
	}
	return false;
    }

    return true;
}

bool test_omfilereader2()
{
    std::string datadir = srcdir + "/testdata/";

    OmIndexerBuilder builder;

    OmIndexer indexer = 
	    make_indexer_one_node("omfilereader", "filename");

    indexer.set_input(OmIndexerMessage(datadir
				+ "indextest_filereader.txt"));

    OmIndexerMessage result = indexer.get_raw_output();

    if (verbose && result.get_type() != OmIndexerMessage::rt_string) {
        tout << "Non-string result: " << result << '\n';
	return false;
    }

    if (result.get_string() != "correct answer\n") {
	if (verbose) {
	    tout << "Got string: `" << result.get_string() << "'" << '\n';
	}
	return false;
    }

    return true;
}

bool test_omvectorsplit1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer =
	    make_indexer_one_node("omvectorsplit");

    OmIndexerMessage result;

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("word"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("sponge"));
    v.append_element(std::string("penguins"));
    v.append_element(std::string("elephants"));

    // now test with a vector
    indexer.set_input(v);

    for (size_t i=0; i<v.get_vector_length(); ++i) {
	result = indexer.get_raw_output();
	if (verbose && result.get_type() != OmIndexerMessage::rt_string) {
	    tout << "Non-string result: " << result << '\n';
	}
	if (result.get_string() != v[i].get_string()) {
	    if (verbose) {
		tout << "Got " << result << ", expected " << v[i] << '\n';
	    }
	    return false;
	}
    }
    result = indexer.get_raw_output();
    if (result.get_type() != OmIndexerMessage::rt_empty) {
	if (verbose) {
	    tout << "Expected empty at end, got: " << result << '\n';
	}
	return false;
    }

    return true;
}

bool test_omlistconcat1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer = builder.build_from_string(
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

    OmIndexerMessage v;
    v.set_vector();
    v.append_element(std::string("word"));
    v.append_element(std::string("flying"));
    v.append_element(std::string("sponge"));
    v.append_element(std::string("penguins"));
    v.append_element(std::string("elephants"));

    // now test with a vector
    indexer.set_input(v);
    result = indexer.get_raw_output();
    if (verbose && result.get_type() != OmIndexerMessage::rt_vector) {
        tout << "Non-vector result: " << result << '\n';
    }
    if (result.get_vector_length() != 2*v.get_vector_length()) {
	tout << "Result length " << result.get_vector_length()
		<< ", expected " << 2*v.get_vector_length() << "\n";
        return false;
    }

    for (unsigned i=0; i<v.get_vector_length(); ++i) {
        if (result.get_element(i).get_string() != v[i].get_string()) {
	    return false;
	}
        if (result.get_element(i + v.get_vector_length()).get_string() != v[i].get_string()) {
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

bool
test_ommakepair1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omsplitter' id='split'>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
	 "<node type='omtranslate' id='trans'>\n"
	     "<param name='from' type='string' value='abc'/>\n"
	     "<param name='to' type='string' value='ABC'/>\n"
	     "<input name='in' node='split' out_name='left'/>\n"
	 "</node>\n"
         "<node type='ommakepair' id='only'>\n"
	     "<input name='left' node='trans' out_name='out'/>\n"
	     "<input name='right' node='split' out_name='right'/>\n"
	 "</node>\n"
         "<output node='only' out_name='out'/>\n"
      "</omindexer>\n");

    indexer.set_input(OmIndexerMessage("cab"));

    OmIndexerMessage result = indexer.get_raw_output();
    if (result.get_type() != OmIndexerMessage::rt_vector) {
	if (verbose) {
	    tout << "Expected pair, got: " << result << '\n';
	}
	return false;
    }
    if (result.get_vector_length() != 2) {
	if (verbose) {
	    tout << "Expected pair, got: " << result << '\n';
	}
	return false;
    }
    if (result.get_element(0).get_string() != "CAB" ||
	result.get_element(1).get_string() != "cab") {
	if (verbose) {
	    tout << "Expected [ 'CAB', 'cab' ], got: " << result << '\n';
	}
	return false;
    }
    return true;
}

bool
test_ommakepairs1()
{
    OmIndexerBuilder builder;

    OmIndexer indexer = builder.build_from_string(
      "<?xml version=\"1.0\"?>\n"
      "<omindexer>\n"
         "<node type='omsplitter' id='split'>\n"
	     "<input name='in' node='START' out_name='out'/>\n"
	 "</node>\n"
	 "<node type='omtranslatelist' id='trans'>\n"
	     "<param name='from' type='string' value='abc'/>\n"
	     "<param name='to' type='string' value='ABC'/>\n"
	     "<input name='in' node='split' out_name='left'/>\n"
	 "</node>\n"
         "<node type='ommakepairs' id='only'>\n"
	     "<input name='left' node='trans' out_name='out'/>\n"
	     "<input name='right' node='split' out_name='right'/>\n"
	 "</node>\n"
         "<output node='only' out_name='out'/>\n"
      "</omindexer>\n");

    OmIndexerMessage vec;
    vec.set_vector();
    vec.append_element(OmIndexerMessage("cab"));
    vec.append_element(OmIndexerMessage("abc"));
    indexer.set_input(vec);

    OmIndexerMessage result = indexer.get_raw_output();
    if (result.get_type() != OmIndexerMessage::rt_vector) {
	if (verbose) {
	    tout << "Expected pair, got: " << result << '\n';
	}
	return false;
    }
    if (result.get_vector_length() != vec.get_vector_length()) {
	if (verbose) {
	    tout << "Expected pair, got: " << result << '\n';
	}
	return false;
    }
    if (result.get_element(0).get_element(0).get_string() != "CAB" ||
	result.get_element(0).get_element(1).get_string() != "cab") {
	if (verbose) {
	    tout << "Got: " << result << '\n';
	    }
	return false;
    }
    if (result.get_element(1).get_element(0).get_string() != "ABC" ||
	result.get_element(1).get_element(1).get_string() != "abc") {
	if (verbose) {
	    tout << "Got: " << result << '\n';
	    }
	return false;
    }
    return true;
}

bool
test_ompaditerator1()
{
    OmIndexerBuilder builder;

    OmNodeDescriptor nd = builder.get_node_info("omsplitter");

    OmPadIterator pi = nd.inputs_begin();

    TEST(pi != nd.inputs_end());
    TEST(*pi == "in");
    TEST(pi.get_type() == "*1");
    TEST(pi.get_phys_type() == mt_record);

    pi++;
    TEST(pi == nd.inputs_end());

    OmPadIterator po = nd.outputs_begin();

    TEST(po != nd.outputs_end());
    TEST(*po == "left");
    TEST(po.get_type() == "*1");
    TEST(po.get_phys_type() == mt_record);

    po++;
    TEST(*po == "right");
    TEST(po.get_type() == "*1");
    TEST(po.get_phys_type() == mt_record);

    po++;
    TEST(po == nd.outputs_end());

    return true;
}

bool
test_indexerdesc1()
{
    OmIndexerDesc desc(make_indexerdesc_one_node("omsplitter", "in", "right"));

    TEST(desc.get_output_node() == "only");
    TEST(desc.get_output_pad() == "right");

    OmNodeInstanceIterator ni(desc.nodes_begin());

    TEST(ni != desc.nodes_end());
    TEST(*ni == "only");

    ++ni;
    TEST(ni == desc.nodes_end());

    ni = desc.find_node("only");
    TEST(*ni == "only");

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {"basic1",			&test_basic1},
    {"basic2",			&test_basic2},
    {"basic3",			&test_basic3},
    {"omindexermessage1",	&test_omindexermessage1},
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
    {"ommakepair1",		&test_ommakepair1},
    {"ommakepairs1",		&test_ommakepairs1},
    {"ompaditerator1",		&test_ompaditerator1},
    {"omindexerdesc1",		&test_indexerdesc1},
    // FIXME: add tests for regex nodes
    {0, 0}
};

int main(int argc, char *argv[])
{
    srcdir = test_driver::get_srcdir(argv[0]);

    return test_driver::main(argc, argv, tests);
}
