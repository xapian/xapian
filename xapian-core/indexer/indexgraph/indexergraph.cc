/* indexergraph.cc: An indexing structure built from an XML definition
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
#include "indexergraph.h"
#include "om/omerror.h"
#include <parser.h>  // libxml
#ifdef HAVE_LIBXML_VALID
#include <valid.h>
#endif
#include <algorithm>

class OmIndexerStartNode : public OmIndexerNode
{
    public:
	static OmIndexerNode *create(const OmSettings &settings) {
	    return new OmIndexerStartNode(settings);
	}

	void set_message(Message msg) {
	    //cout << "Setting message:" << msg << endl;
	    message = msg;
	};
    private:
	OmIndexerStartNode(const OmSettings &settings)
		: OmIndexerNode(settings) {}
	void calculate() {
	    set_output("out", message);
	}

	Message message;
};

OmIndexer::OmIndexer()
{
}

OmIndexer::~OmIndexer()
{
}

auto_ptr<OmIndexer>
OmIndexerBuilder::build_from_file(std::string filename)
{
    xmlDocPtr doc = get_xmltree_from_file(filename);

    auto_ptr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), doc);

    return indexer;
}

auto_ptr<OmIndexer>
OmIndexerBuilder::build_from_string(std::string filename)
{
    xmlDocPtr doc = get_xmltree_from_string(filename);

    auto_ptr<OmIndexer> indexer(new OmIndexer());
    build_graph(indexer.get(), doc);

    return indexer;
}

xmlDocPtr
OmIndexerBuilder::get_xmltree_from_file(const std::string &filename)
{
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    if (!doc || !doc_is_valid(doc)) {
	throw OmInvalidDataError("Graph definition is invalid");
    }

    return doc;
}

xmlDocPtr
OmIndexerBuilder::get_xmltree_from_string(const std::string &xmldesc)
{
    xmlDocPtr doc = xmlParseMemory(const_cast<char *>(xmldesc.c_str()),
				   xmldesc.length());

    if (!doc || !doc_is_valid(doc)) {
	throw OmInvalidDataError("Graph definition is invalid");
    }

    return doc;
}

extern "C" {
static void xml_error_func(void *ctx, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "ERROR: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    throw OmInvalidDataError("xml is not valid");
}

static void xml_warn_func(void *ctx, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "WARNING: ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

} // extern "C"

bool
OmIndexerBuilder::doc_is_valid(xmlDocPtr doc)
{
#ifdef HAVE_LIBXML_VALID
    xmlValidCtxt ctxt;
    ctxt.error = &xml_error_func;
    ctxt.error = &xml_warn_func;

    // Add our predefined "START" id
    xmlAttrPtr attr = xmlNewDocProp(doc, "id", "START");
    xmlAddID(&ctxt, doc, "START", attr);

    return xmlValidateDocument(&ctxt, doc);
#else  // HAVE_LIBXML_VALID
    return true;
#endif
}

OmIndexerNode *
OmIndexerBuilder::make_node(const std::string &type,
			    const OmSettings &config)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(type);
    if (i == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") + type);
    } else {
	return i->second.create(config);
    }
}

typedef std::map<std::string, std::string> attrmap;
typedef std::map<std::string, std::string> typemap;

// FIXME: handle the unicode stuff rather than char *
static attrmap attr_to_map(xmlAttrPtr attr)
{
    std::map<std::string, std::string> result;
    while (attr) {
	std::string name = (char *)attr->name;
	xmlNodePtr val = attr->val;
	std::string value = (char *)val->content;
	//cout << "\tAttr " << name << "=" << value << endl;

	result[name] = value;

	attr = attr->next;
    }
    return result;
}

void
OmIndexerBuilder::build_graph(OmIndexer *indexer, xmlDocPtr doc)
{
    xmlNodePtr root = doc->root;
    if (!root) {
	throw OmInvalidDataError("Error parsing graph description");
    }
    std::string rootname = (char *)root->name;
    if (rootname != "omindexer") {
	throw OmInvalidDataError("Root tag was not <omindexer>");
    }
    //cout << "root name is " << rootname << endl;
    typemap types;
    
    indexer->nodemap["START"] = make_node("START", OmSettings());
    indexer->start = dynamic_cast<OmIndexerStartNode *>(indexer->nodemap["START"]);
    types["START"] = "START";
    
    for (xmlNodePtr node = root->childs;
	 node != 0;
	 node = node->next) {
	std::string type = (char *)node->name;
	// cout << "node name = " << type << endl;
	if (type == "node") {
	    attrmap node_attrs(attr_to_map(node->properties));
	    if (indexer->nodemap.find(node_attrs["id"]) != indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Duplicate node id ")
					 + node_attrs["id"]);
	    }
	    xmlNodePtr child = node->childs;
	    OmSettings config;
	    while (child != 0 &&
		   std::string((char *)child->name) == "param") {
		attrmap param_attrs(attr_to_map(child->properties));
		config.set(param_attrs["name"], param_attrs["value"]);
		child = child->next;
	    }
	    OmIndexerNode *newnode = make_node(node_attrs["type"], config);
	    types[node_attrs["id"]] = node_attrs["type"];
	    indexer->nodemap[node_attrs["id"]] = newnode;
	    // connect the inputs
	    while (child != 0) {
		if (std::string((char *)child->name) != "input") {
		    throw OmInvalidDataError(std::string("<input> tag expected, found ") + std::string((char *)child->name));
		}
		attrmap input_attrs(attr_to_map(child->properties));
		OmIndexer::NodeMap::const_iterator i =
			indexer->nodemap.find(input_attrs["node"]);
		if (i == indexer->nodemap.end()) {
		    throw OmInvalidDataError(std::string("Input node ") +
					     input_attrs["node"] +
					     " not found");
		}
		// typecheck throws on an error
		typecheck(node_attrs["type"], // this node's type
			  input_attrs["name"],// this node's input name
			  types[input_attrs["node"]], // the input node's type
			  input_attrs["out_name"]);  // the input node's output
		newnode->connect_input(input_attrs["name"],
				       i->second,
				       input_attrs["out_name"]);

		child = child->next;
	    }
	} else if (type == "output") {
	    attrmap attrs(attr_to_map(node->properties));
	    OmIndexer::NodeMap::const_iterator i = indexer->nodemap.find(attrs["node"]);
	    if (i == indexer->nodemap.end()) {
		throw OmInvalidDataError(std::string("Unknown output node ") +
					 attrs["node"]);
	    }
	    indexer->final = i->second;
	    indexer->final_out = attrs["out_name"];
	} else {
	    throw OmInvalidDataError(std::string("Unexpected tag ") +
				     type);
	}
    }
}

void
OmIndexerBuilder::typecheck(const std::string &receivertype,
			    const std::string &receiverin,
			    const std::string &sendertype,
			    const std::string &senderout)
{
    OmNodeConnection sendercon = get_outputcon(sendertype, senderout);
    OmNodeConnection receivercon = get_inputcon(receivertype, receiverin);

    // First check that the physical type is compatible
    if (sendercon.phys_type != receivercon.phys_type) {
	if (sendercon.phys_type == mt_record ||
	    receivercon.phys_type == mt_record) {
	    // this is ok - things can be converted (up to a point)
	    // between records and primitives.
	} else {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not physically compatible.");
	}
    } else if (sendercon.type != receivercon.type) {
	if (receivercon.type != "ANY" && sendercon.type != "ANY") {
	    throw OmInvalidDataError(std::string("Types of ") + 
					 sendertype + "[" + senderout + "]" +
					 " and " + receivertype + "[" +
					 receiverin + "] are not compatible.");
	}
	// else at least one of them is "universal"
    }
}

OmNodeConnection
OmIndexerBuilder::get_outputcon(const std::string &nodetype,
				const std::string &output_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodeConnection>::const_iterator i;
    for (i=type->second.outputs.begin(); i!= type->second.outputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == output_name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Invalid output connection ") +
				 nodetype + "[" + output_name + "]");
}

OmNodeConnection
OmIndexerBuilder::get_inputcon(const std::string &nodetype,
			       const std::string &input_name)
{
    std::map<std::string, node_desc>::const_iterator type;
    type = nodetypes.find(nodetype);
    if (type == nodetypes.end()) {
	throw OmInvalidDataError(std::string("Unknown node type ") +
				     nodetype);
    }
    std::vector<OmNodeConnection>::const_iterator i;
    for (i=type->second.inputs.begin(); i!= type->second.inputs.end(); ++i) {
	// FIXME: possibly ought to be a map rather than a vector.
	if (i->name == input_name) {
	    return *i;
	}
    }
    throw OmInvalidDataError(std::string("Invalid input connection ") +
				 nodetype + "[" + input_name + "]");
}

Message
OmIndexer::get_output()
{
    return final->get_output_record(final_out);
}

void
OmIndexer::set_input(Message msg)
{
    start->set_message(msg);
}

void
OmIndexer::set_node_config(const std::string &node_id,
			   const std::string &key,
			   const std::string &value)
{
    NodeMap::iterator i = nodemap.find(node_id);
    if (i == nodemap.end()) {
	throw OmInvalidDataError(std::string("Node id ") + node_id +
				 " doesn't exist");
    }
    i->second->set_config_string(key, value);
}

OmIndexerBuilder::OmIndexerBuilder()
{
    OmNodeDescriptor ndesc("START", &OmIndexerStartNode::create);
    ndesc.add_output("out", "mystr", mt_record);
    register_node_type(ndesc);
}

void
OmIndexerBuilder::register_node_type(const OmNodeDescriptor &ndesc_)
{
    std::map<std::string, node_desc>::const_iterator i;
    i = nodetypes.find(ndesc_.nodename);
    if (i != nodetypes.end()) {
	throw OmInvalidArgumentError(string("Attempt to register node type ")
				     + ndesc_.nodename + ", which already exists.");
    }

    node_desc ndesc;
    ndesc.create = ndesc_.creator;
    std::copy(ndesc_.inputs.begin(), ndesc_.inputs.end(),
	      std::back_inserter(ndesc.inputs));
    std::copy(ndesc_.outputs.begin(), ndesc_.outputs.end(),
	      std::back_inserter(ndesc.outputs));

    nodetypes[ndesc_.nodename] = ndesc;
}

OmNodeDescriptor::OmNodeDescriptor(const std::string &nodename_,
				   OmNodeCreator creator_)
	: nodename(nodename_), creator(creator_)
{
}

void
OmNodeDescriptor::add_input(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    inputs.push_back(OmNodeConnection(name, type, phys_type));
}

void
OmNodeDescriptor::add_output(const std::string &name,
			    const std::string &type,
			    OmIndexerMessageType phys_type)
{
    outputs.push_back(OmNodeConnection(name, type, phys_type));
}
