/*
 * $Id$
 *
 * Dump a Xapian database to an XML representation that xdb-manage can use
 * Copyright 2001 tangozebra ltd
 * Copyright 2002 Ananova Ltd
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
 */

#include <config.h>

#include <om/om.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <xmlmemory.h>
#include <parser.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;

// This clump of defines is for compatibility across libxml1 and 2.
// libxml2 and later versions of libxml1 should have these already.
// These should work for earlier libxml1 versions.
#ifndef xmlChildrenNode
#define xmlChildrenNode childs
#define xmlRootNode root
#endif

OmSettings read_db_options(xmlDocPtr, xmlNodePtr); // xdb-options.cc

int main(int argc, char** argv)
{
    // getopt
    char* optstring = "hv";
    int longindex, getopt_ret;

    struct option longopts[3] = {
	{ "help",		0,			NULL, 'h' },
	{ "version",		0,			NULL, 'v' },
	{ NULL, 0, NULL, 0 }
    };

    while ((getopt_ret = getopt_long(argc, argv, optstring,
                                     longopts, &longindex))!=EOF) {
        switch (getopt_ret) {
        case 'h':
            cout << "Usage: " << argv[0] << " [OPTION] FILE\n\n"
	         << "Dump an Xapian database to an XML file.\n"
                 << "  -h, --help\t\tdisplay this help and exit\n"
                 << "  -v, --version\t\toutput version and exit\n\n"
                 << "Report bugs via the web interface at:\n"
                 << "<http://xapian.org/bugs/>" << endl;
            return 0;
        case 'v':
	    cout << PACKAGE << " " << VERSION << "\n"
		 << "Copyright 2001 tangozebra ltd\n"
		 << "Copyright 2002 Ananova Ltd\n\n"
		 << "This is free software, and may be redistributed under\n"
		 << "the terms of the GNU Public License." << endl;
            return 0;
        case ':': // missing param
            return 1;
        case '?': // unknown option
            return 1;
	}
    }

    if (argc - optind != 1) {
	cerr << "Must specify exactly one file." << endl;
	return 1;
    }

    OmDatabase* database = 0;
    xmlNodePtr options = 0;
    try {
	xmlDocPtr doc = xmlParseFile(argv[optind]);
	if (doc==NULL) {
	    throw string("could't open/parse file");
	}
	
	options = xmlDocGetRootElement(doc);
	if (options==NULL) {
	    xmlFreeDoc(doc);
	    throw string("empty document");
	}
    
	xmlNsPtr ns = xmlSearchNsByHref(doc,options, (const xmlChar*)"http://xapian.org/schemas/dbtools/db-options");
	if (ns== NULL) {
	    xmlFreeDoc(doc);
	    throw string("wrong namespace");
	}
	if(xmlStrcmp(options->name, (const xmlChar *) "db-options")) {
	    xmlFreeDoc(doc);
	    throw string("wrong root element");
	}

	try {
	    OmSettings db_options = read_db_options(doc, options);
	    options = xmlCopyNode(options, 1); // so we can use it later
	    database = new OmDatabase(db_options);
	    xmlFreeDoc(doc);
	} catch (OmError& error) {
	    xmlFreeDoc(doc);
	    throw error.get_msg();
	}
    } catch (string& error) {
	cerr << error << endl;
	return 1;
    }

    // And dump the database ...
    xmlDocPtr output = xmlNewDoc((const xmlChar*) "1.0");
    if (!output) {
	if (database)
	    delete database;
	cerr << "Couldn't create output document" << endl;
	return 1;
    }
    xmlNodePtr output_root = xmlNewDocNode(output, NULL,(const xmlChar*) "db-update", NULL);
    if (!output_root) {
	if (database)
	    delete database;
	xmlFreeDoc(output);
	cerr << "Couldn't create output document root element" << endl;
	return 1;
    }
    xmlNsPtr output_namespace = xmlNewNs(output_root,(const xmlChar*) "http://xapian.org/schemas/dbtools/manage",NULL);
    if (!output_namespace) {
	if (database)
	    delete database;
	xmlFreeDoc(output);
	cerr << "Couldn't set output document namespace" << endl;
	return 1;
    }
    xmlDocSetRootElement(output, output_root);
    xmlSetNs (output_root, output_namespace);

    try {
	// We need a db-options document. This needs to be based on
	// our input document, so thank god we saved that information
	if (options) {
	    if (!xmlAddChild(output_root, options)) {
		throw string("couldn't add options block to dump");
	    }
	} // else, well, we just can't dump it
	
	// Let's have an <add> element
	xmlNodePtr add_node = xmlNewChild (output_root, NULL,(const xmlChar*) "add", NULL);
	if (!add_node) {
	    throw string("couldn't create add node for dump");
	}
	
	// Now we want to actually dump the contents of the database
	om_docid next_doc = 1; // Lowest valid index
	for (om_doccount done_docs=0;
	     done_docs < database->get_doccount(); done_docs++) {
	    bool found = 0;
	    while (found==0) {
		try {
		    OmDocument document = database->get_document(next_doc); 
		    // Give ourselves a <doc> element to play with
		    xmlNodePtr document_node = xmlNewChild (add_node, NULL,(const xmlChar*) "doc", NULL);
		    if (!document_node) {
			throw string("couldn't create node to dump this document");
		    }
		    char number[40];
		    sprintf(number, " doc_id = %i ", next_doc);
		    xmlNodePtr document_comment = xmlNewComment((const xmlChar*)number);
		    if (document_comment) { // don't worry about errors here
			if (!xmlAddChild(document_node, document_comment)) {
			    xmlFreeNode(document_comment);
			}
		    }
		    
		    // (literal) terms
		    for (OmTermIterator term=document.termlist_begin();
			 term!=document.termlist_end();
			 term++) {
			xmlNodePtr term_node = xmlNewChild (document_node, NULL,(const xmlChar*) "literal-term", NULL);
			if (!term_node) {
			    throw string("couldn't create node to dump a term");
			}
			
			sprintf(number, "%i", term.get_wdf());
			if (!xmlSetProp(term_node, (const xmlChar*)"wdf", (const xmlChar*)number) ||
			    !xmlSetProp(term_node, (const xmlChar*)"value", (const xmlChar*)(*term).c_str())) {
			    throw string("couldn't dump wdf and/or value for term");
			}
			string positions = "";
			for (OmPositionListIterator position=term.positionlist_begin();
			     position!=term.positionlist_end();
			     ) {
			    sprintf(number, "%i", *position);
			    positions += string(number);
			    position++;
			    if (position != term.positionlist_end()) {
				positions += ",";
			    }
			}
			if (positions!="") {
			    if (!xmlSetProp(term_node, (const xmlChar*)"positions", (const xmlChar*)positions.c_str())) {
				throw string("couldn't set position list for term");
			    }
			}
		    }
		    
		    for (OmValueIterator value = document.values_begin();
			 value != document.values_end();
			 ++value) {
			xmlNodePtr value_node = xmlNewChild (document_node, NULL,(const xmlChar*) "value", NULL);
			if (!value_node) {
			    throw string("couldn't create node to dump value");
			}
			sprintf(number, "%i", value.get_valueno());
			if (!xmlSetProp(value_node, (const xmlChar*)"number", (const xmlChar*)number) ||
			    !xmlSetProp(value_node, (const xmlChar*)"value", (const xmlChar*)value->c_str())) {
			    throw string("couldn't set number and/or value for value");
			}
		    }
		    
		    // Data -- FIX me: xmlEncodeEntitiesReentrant needs its result freeing
		    xmlChar* xmlstr = xmlEncodeEntitiesReentrant(output, (const xmlChar*)document.get_data().c_str ());
		    if (!xmlstr) {
			throw string("couldn't encode data of document to dump");
		    }
		    xmlNodePtr data_node = xmlNewChild (document_node, NULL,(const xmlChar*) "data", xmlstr);
		    if (!data_node) {
			throw string("couldn't create node to dump document data");
		    }
		    xmlSetProp(data_node, (const xmlChar*)"type", (const xmlChar*)"inline"); // don't care about error, because this is the default
		    
		    found = 1;
		} catch (OmDocNotFoundError& error) {
		    // Do nothing - we just try the next
		    // This will work unless ->get_doccount() lies to us
		}
		next_doc++;
	    }
	}
    } catch (string& s) {
	xmlFreeDoc(output);
	if (database) {
	    delete database;
	}
	cerr << s << endl;
	return 1;
    }
	
#ifdef HAVE_LIBXML2    
    int result = xmlDocDump (stdout, output);
#else
    int result = 0;
    xmlDocDump (stdout, output);
#endif
    if (result==-1) {
	cerr << "Couldn't dump output document" << endl;
    }
    
    xmlFreeDoc(output);
    if (database) {
	delete database;
    }
    return 0;
}
