/* indexerxml.cc: Functions dealing with XML input
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

#include "indexerxml.h"
#include "om/omerror.h"
#include "register_core.h"
#include <parser.h>  // libxml
#ifdef HAVE_LIBXML_VALID
#include <valid.h>
#endif
#include <algorithm>
#include <map>

/** Return true if this xml Document is valid */
static bool doc_is_valid(xmlDocPtr doc);

/** Walk the xml tree to make an OmIndexerDesc */
static AutoPtr<OmIndexerDesc> desc_from_tree(xmlDocPtr doc);

AutoPtr<OmIndexerDesc>
desc_from_xml_file(const std::string &filename)
{
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    if (!doc || !doc_is_valid(doc)) {
	throw OmInvalidDataError("Graph definition is invalid");
    }

    return desc_from_tree(doc);
}

AutoPtr<OmIndexerDesc>
desc_from_xml_string(const std::string &xmldesc)
{
    xmlDocPtr doc = xmlParseMemory(const_cast<char *>(xmldesc.c_str()),
				   xmldesc.length());

    if (!doc || !doc_is_valid(doc)) {
	throw OmInvalidDataError("Graph definition is invalid");
    }

    return desc_from_tree(doc);
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

static bool
doc_is_valid(xmlDocPtr doc)
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

typedef std::map<std::string, std::string> attrmap;
typedef std::map<std::string, std::string> typemap;

static std::string
get_prop(xmlNodePtr node, const std::string &prop)
{
    /* CHAR is used because it works with older libxmls, before
     * the xmlChar typedef.
     */
    CHAR *temp = 0;
    std::string retval;
    try {
	temp = xmlGetProp(node, prop.c_str());
	retval = (char *)temp;
	free(temp);
	temp = 0;
    } catch (...) {
	if (temp) free(temp);
	temp = 0;
	throw;
    }
    return retval;
}

#if 0
// FIXME: handle the unicode stuff rather than char *
static attrmap attr_to_map(xmlNodePtr node)
{
    xmlAttrPtr attr = node->properties;
    std::map<std::string, std::string> result;
    while (attr) {
	std::string name = (char *)attr->name;
	CHAR *temp = 0;
	std::string value;
	try {
	    //cerr << "Attr " << node->name << "." << name << "=" << endl;
	    temp = xmlGetProp(node, name.c_str());
	    if (temp) {
		value = (char *)temp;
		//cerr << "\tvalue = " << value << endl;
		free(temp);
		temp = 0;
	    }
#if 0  /* this is there to help in debugging odd attributes */
	    {
		xmlDocPtr doc = node->doc;
		xmlAttributePtr attrDecl;
		if (doc->intSubset) {
		    attrDecl = xmlGetDtdAttrDesc(doc->intSubset, node->name,
						 name.c_str());
		    if (attrDecl) {
			//cerr << "\tintSubset default = "
			//	<< attrDecl->defaultValue << endl;
		    }
		}
		if (doc->extSubset) {
		    attrDecl = xmlGetDtdAttrDesc(doc->extSubset, node->name,
						 name.c_str());
		    if (attrDecl) {
			//cerr << "\tintSubset default = "
			//	<< attrDecl->defaultValue << endl;
		    }
		}
	    }
#endif
	} catch (...) {
	    if (temp) free(temp);
	    throw;
	}
	/*
	 *
	 * xmlNodePtr val = attr->val;
	 * std::string value = (char *)val->content;
	 */

	result[name] = value;

	attr = attr->next;
    }
    return result;
}
#endif /* 0 */

/** Turn the parameters specified in <param> tags into OmSettings
 *  entries.
 *  
 *  @param node		The first <param> node
 *  @param config	The OmSettings object to modify
 *
 *  @return The first non-<param> node found.
 */
xmlNodePtr
get_config_values(xmlNodePtr node, OmSettings &config)
{
    while (node != 0) {
	// skip comments...
	if (node->type == XML_COMMENT_NODE) {
	    node = node->next;
	    continue;
	}
	
	// finish if we're no longer seeing <param ...>
	if (std::string((char *)node->name) != "param") {
	    break;
	}

	std::string type = get_prop(node, "type");
	if (type == "string") {
	    config.set(get_prop(node, "name"),
		       get_prop(node, "value"));
	} else if (type == "list") {
	    xmlNodePtr items = node->childs;
	    std::vector<std::string> values;
	    while (items) {
		if (items->type == XML_COMMENT_NODE) {
		    items = items->next;
		    continue;
		}
		std::string name((char *)items->name);
		if (name != "item") {
		    throw OmInvalidDataError(std::string("Unexpected tag `")
					     + name + "'");
		}
		values.push_back(get_prop(items, "value"));
		items = items->next;
	    }
	    config.set(get_prop(node, "name"),
		       values.begin(),
		       values.end());
	} else {
	    throw OmInvalidDataError(std::string("Invalid <param> type `")
				     + type + "'");
	}
	node = node->next;
    }
    return node;
}

static AutoPtr<OmIndexerDesc>
desc_from_tree(xmlDocPtr doc)
{
    xmlNodePtr root = doc->root;
    //cerr << "intSubset = " << doc->intSubset << endl;
    //cerr << "extSubset = " << doc->extSubset << endl;
    if (!root) {
	throw OmInvalidDataError("Error parsing graph description");
    }
    std::string rootname = (char *)root->name;
    if (rootname != "omindexer") {
	throw OmInvalidDataError(std::string("Root tag was `")
				 + rootname + "', not <omindexer>");
    }

    AutoPtr<OmIndexerDesc> result(new OmIndexerDesc());

    for (xmlNodePtr node = root->childs;
	 node != 0;
	 node = node->next) {

	if (node->type == XML_COMMENT_NODE) {
	    continue;
	}
	std::string type = (char *)node->name;
	if (type == "node") {
	    OmIndexerDesc::NodeInstance ndesc;

	    ndesc.type = get_prop(node, "type");
	    ndesc.id = get_prop(node, "id");

	    xmlNodePtr child = node->childs;

	    child = get_config_values(child, ndesc.param);

	    // translate the inputs
	    while (child != 0) {
		if (child->type == XML_COMMENT_NODE) {
		    child = child->next;
		    continue;
		}

		if (std::string((char *)child->name) != "input") {
		    throw OmInvalidDataError(std::string("<input> tag expected, found ") + std::string((char *)child->name));
		}
		OmIndexerDesc::Connect conn;
		conn.input_name = get_prop(child, "name");
		conn.feeder_node = get_prop(child, "node");
		conn.feeder_out = get_prop(child, "out_name");
		ndesc.input.push_back(conn);

		child = child->next;
	    }
	    result->nodes.push_back(ndesc);
	} else if (type == "output") {
	    result->output_node = get_prop(node, "node");
	    result->output_conn = get_prop(node, "out_name");
	}
    }
    return result;
}
