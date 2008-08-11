/* omega-config.cc: CGI-based configuration for omega.
 *
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include <config.h>

#include <string>
#include <map>
#include <fstream>
#include <utility>
#include <list>
#include <iostream>

#include "safeunistd.h"
#include <parser.h>

using std::string;
using std::list;
using std::pair;
using std::map;

#define xmlT(x) (reinterpret_cast<const xmlChar*>(x))
#define toC(x)  (reinterpret_cast<const char*>(x))

/** This clump of defines is for compatibility across libxml1 and 2.
 *  libxml2 and later versions of libxml1 should have these already.
 *  These should work for earlier libxml1 versions.
 */
#ifndef xmlChildrenNode
#define xmlChildrenNode childs
#define xmlRootNode root
#endif

static list<pair<string, string> >                mappings;
static list<pair<string, pair<string, string> > > actions;
static list<string>                               inclusions;
static list<string>                               exclusions;

static const string cgi_path      = "http://alum/cgi-bin/omindex-config-submit";
static const string config_file   = "/etc/omindex.conf";
static const string template_file = "/var/www/omindex-config.html";

static const unsigned int SPLIT_LENGTH = 70u;

static const xmlChar* XMLTAG_BODY           = xmlT("omindex");
static const xmlChar* XMLTAG_MAPPINGS       = xmlT("mappings");
static const xmlChar* XMLTAG_MAPPINGS_MAP   = xmlT("map");
static const xmlChar* XMLTAG_MAPPINGS_LOCAL = xmlT("local");
static const xmlChar* XMLTAG_MAPPINGS_HREF  = xmlT("href");
static const xmlChar* XMLTAG_INCLUSIONS     = xmlT("inclusions");
static const xmlChar* XMLTAG_INCLUSIONS_INC = xmlT("include");
static const xmlChar* XMLTAG_FILTER         = xmlT("filter");
static const xmlChar* XMLTAG_EXCLUSIONS     = xmlT("exclusions");
static const xmlChar* XMLTAG_EXCLUSIONS_EXC = xmlT("exclude");
static const xmlChar* XMLTAG_ACTIONS        = xmlT("actions");
static const xmlChar* XMLTAG_ACTIONS_ACT    = xmlT("action");
static const xmlChar* XMLTAG_ACTIONS_LOAD   = xmlT("load");
static const xmlChar* XMLTAG_ACTIONS_RUN    = xmlT("run");
static const xmlChar* XMLTAG_ACTIONS_MIME   = xmlT("mime");

typedef void (*ReaderFn)(string[]);
typedef void (*WriterFn)();

#define NUM_PROPERTIES 3

struct reader_writer_info {
    const xmlChar* top_tag;
    const xmlChar* child_tag;
    const xmlChar* properties[NUM_PROPERTIES];
    ReaderFn reader;
    WriterFn writer;
};

static void read_mappings(string data[])
{
    mappings.push_back(pair<string, string>(data[0], data[1]));
}

static void write_mappings()
{

}

static void read_inclusions(string data[])
{
    inclusions.push_back(data[0]);
}

static void write_inclusions()
{

}

static void read_exclusions(string data[])
{
    exclusions.push_back(data[0]);
}

static void write_exclusions()
{

}

static void read_actions(string data[])
{
    actions.push_back(pair<string, pair<string, string> >(data[0],
							  pair<string, string>(data[1], data[2])));
}

static void write_actions()
{

}

static const reader_writer_info reader_writer_info_tab[] = {
    { XMLTAG_MAPPINGS, XMLTAG_MAPPINGS_MAP,
      { XMLTAG_MAPPINGS_LOCAL, XMLTAG_MAPPINGS_HREF, NULL }, read_mappings, write_mappings },
    { XMLTAG_INCLUSIONS, XMLTAG_INCLUSIONS_INC,
      { XMLTAG_FILTER, NULL, NULL }, read_inclusions, write_inclusions },
    { XMLTAG_EXCLUSIONS, XMLTAG_EXCLUSIONS_EXC,
      { XMLTAG_FILTER, NULL, NULL }, read_exclusions, write_exclusions },
    { XMLTAG_ACTIONS, XMLTAG_ACTIONS_ACT,
      { XMLTAG_ACTIONS_MIME, XMLTAG_ACTIONS_RUN, XMLTAG_ACTIONS_LOAD }, read_actions,
                                                                        write_actions },
    { NULL, NULL, { NULL, NULL, NULL }, NULL, NULL }
};

static void read_block(xmlDocPtr doc, xmlNodePtr node, const reader_writer_info* info)
{
    node = node->xmlChildrenNode;

    while (node) {
	if (!xmlIsBlankNode(node)) {
	    if (xmlStrcmp(node->name, info->child_tag)) {
		throw (string("Unexpected node '") + toC(node->name) +
		       string("'"));
	    }

	    if (!node->properties) {
		throw (string("No properties present for node '") + toC(node->name) + string("'"));
	    }
	
	    string data[NUM_PROPERTIES];
	    for (int i = 0; i < NUM_PROPERTIES; i++) {
		data[i] = "";
	    }
	
	    xmlAttr* prop = node->properties;
	    while (prop) {
		for (int i = 0; i < NUM_PROPERTIES; i++) {
		    if (!xmlStrcmp(prop->name, info->properties[i])) {
			if (data[i] != "") {
			    throw (string("Duplicate '") +
				   reinterpret_cast<const char*>(info->properties[i]) +
				   string("' property for node ") +
				   reinterpret_cast<const char*>(node->name) + string("'"));
			}
			data[i] = string(toC(xmlNodeListGetString(doc, prop->children, 1)));
		    }
		}
		prop = prop->next;
	    }
	    for (int i = 0; i < NUM_PROPERTIES; i++) {
		if (data[i] == "" && info->properties[i]) {
		    throw (string("Missing '") +
			   reinterpret_cast<const char*>(info->properties[i]) +
			   string("' property for node ") +
			   reinterpret_cast<const char*>(node->name) +
			   string("'"));
		}
	    }

	    (*(info->reader))(data);
	}

	node = node->next;
    }
}

static void read_config_file()
{
    static map<string, const reader_writer_info*> rw_map;
    if (rw_map.empty()) {
	const reader_writer_info* tabptr = reader_writer_info_tab;
	while (tabptr->top_tag) {
      	    rw_map[string(toC(tabptr->top_tag))] = tabptr;
	    tabptr++;
	}
    }

    xmlDocPtr doc = xmlParseFile(config_file.c_str());
    if (!doc) {
	throw (string("Couldn't open configuration file ") + config_file);
    }

    try {
	xmlNodePtr cur = xmlDocGetRootElement(doc);
	if (!cur) {
	    throw string("Empty configuration file");
	}

	if (xmlStrcmp(cur->name, XMLTAG_BODY)) {
	    throw string("Bad root node in configuration file");
	}

	cur = cur->xmlChildrenNode;
	while (cur) {
	    if (!xmlIsBlankNode(cur)) {
		string tag = string(toC(cur->name));
		map<string, const reader_writer_info*>::const_iterator iter = rw_map.find(tag);

		try {
		    if (iter == rw_map.end()) {
			throw "Unknown tag `" + tag + "'";
		    }
		    
		    read_block(doc, cur, (*iter).second);
		}
		catch (string& msg) {
		    throw (msg + string(" in configuration file"));
		}
	    }

	    cur = cur->next;
	}
    }
    catch (string& msg) {
	xmlFreeDoc(doc);
	throw msg + " " + config_file;
    }

    xmlFreeDoc(doc);
}

static string process_cgi()
{
    return cgi_path;
}

static string process_host()
{
    char buffer[256];
    gethostname(buffer, 256);
    return string(buffer);
}

static string process_configfile()
{
    return config_file;
}

static string process_mimetypes()
{
    list<pair<string, pair<string, string> > >::iterator posn = actions.begin();
    string ret = "";
    while (posn != actions.end()) {
	pair<string, pair<string, string> > p = *posn++;
	ret += p.first + ": load \"" + p.second.first + "\" run \"" + p.second.second + "\"\n";
    }

    return ret;
}

static string process_inclusions_or_exclusions(list<string>& l)
{
    list<string>::iterator posn = l.begin();
    string ret = "";
    while (posn != l.end()) {
	ret += *posn++;
	if (posn != l.end()) {
	    if (ret.length() > SPLIT_LENGTH) {
		ret += "\n";
	    }
	    else {
		ret += ";";
	    }
	}
    }

    return ret;
}

static string process_inclusions()
{
    return process_inclusions_or_exclusions(inclusions);
}

static string process_exclusions()
{
    return process_inclusions_or_exclusions(exclusions);
}

static string process_mappings()
{
    list<pair<string, string> >::iterator posn = mappings.begin();
    string ret = "";
    while (posn != mappings.end()) {
	pair<string, string> p = *posn++;
	ret += p.first + " = " + p.second + "\n";
    }

    return ret;
}

typedef string (*ProcessorFn)();

struct func_info {
    const char* text;
    ProcessorFn processor;
};

static const func_info func_info_tab[] = {
    { "cgi",        process_cgi },
    { "mimetypes",  process_mimetypes },
    { "filesinc",   process_inclusions },
    { "filesexc",   process_exclusions },
    { "mappings",   process_mappings },
    { "configfile", process_configfile },
    { "host",       process_host },
    { NULL,         NULL }
};

static const char TRIGGER_CHAR = '$';

static string make_substitutions(const string& input)
{
    static map<string, ProcessorFn> func_map;
    if (func_map.empty()) {
	const func_info* tabptr = func_info_tab;
	while (tabptr->text) {
      	    func_map[string(tabptr->text)] = tabptr->processor;
	    tabptr++;
	}
    }

    string result;
    string::size_type current_pos = 0;
    string::size_type next_pos;
    while ((next_pos = input.find(TRIGGER_CHAR, current_pos)) != string::npos) {
	result += input.substr(current_pos, next_pos - current_pos);
	next_pos++;
	if (next_pos < input.size()) {
	    char lit = '\0';	
	    for (const char* special_chars = "$$({)}.,"; *special_chars && !lit; special_chars++) {
		if (input[next_pos] == *special_chars++) {
		    lit = *special_chars;
		}
	    }
	    
	    if (lit) {
		result += lit;
		current_pos = next_pos + 1;
	    }
	    else {
		if ((current_pos = input.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
							   "abcdefghijklmnopqrstuvwxyz", next_pos))
		    == string::npos) {
		    current_pos = input.size();
		}

		string var = input.substr(next_pos, current_pos - next_pos);
		map<string, ProcessorFn>::const_iterator iter = func_map.find(var);
		if (iter == func_map.end()) {
		    throw "Unknown function `" + var + "'";
		}
		
		result += ((*iter).second)();
	    }
	}
    }

    result += input.substr(current_pos);

    return result;
}

static void process_template(const string& tpl)
{    
    string fmt;
    std::ifstream str(tpl.c_str());
    if (!str.is_open()) {
	throw (string("Couldn't open format template `") + tpl + '\'');
    }

    while (!getline(str, fmt).eof()) {
	std::cout << make_substitutions(fmt) << std::endl;
    }

    str.close();
}

int main(int argc, char* argv[])
{
    std::cout << "Content-type: text/html" << std::endl << std::endl;

    try {
	read_config_file();
	process_template(template_file);
    }
    catch (string& msg) {
	std::cout << "<html><head><title>Internal Error</title></head></body>" << std::endl;
	std::cout << "<h1>Internal Error</h1><p>" << msg << "</p></body></html>" << std::endl;
    }

    return 0;
}
