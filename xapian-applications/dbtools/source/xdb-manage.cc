/*
 * $Id$
 *
 * Manage a Xapian database via XML message files
 * Copyright 2001 tangozebra ltd
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

#include <om/om.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string>
#include <vector>
#include <fstream>

#include <xmlmemory.h>
#include <parser.h>

#include "config.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;

static string default_generator="";
static string default_generator_string="";

OmSettings read_db_options(xmlDocPtr, xmlNodePtr); // xdb-options.cc

string fetch_file(const string& filename)
{
    std::ifstream file(filename.c_str());
    string s;
    getline(file, s, char(26) /* ^Z */);
    return s;
}

string make_generator (xmlDocPtr document,xmlNodePtr node)
{
    xmlNodePtr pointer = xmlCopyNode (node, 1);
    xmlDocPtr new_document = xmlNewDoc((const xmlChar*) "1.0");
    if (pointer==NULL||new_document==NULL) {
        throw string ("couldn't create generator (unspecified internal error)");
    }
    xmlDocSetRootElement (new_document, pointer);

    xmlChar*memory;
    int size;
    xmlDocDumpMemory(new_document, &memory, &size);
    string characters = string ((char*)memory);
    //    cout << "constructed document = \""<<characters<<"\""<<endl;
    //    OmIndexerBuilder builder =OmIndexerBuilder();
    //    OmIndexer generator = builder.build_from_string (characters);
    free(memory);
    xmlFreeDoc(new_document);
    return characters;
}

om_docid find_document (OmDatabase*database, xmlNodePtr document)
{
    if (xmlStrcmp(document->name,(const xmlChar*) "doc")) {
	throw string ("expected 'doc' element, but found something else");
    }
    if(xmlChar* id= xmlGetProp (document,(const xmlChar*) "doc-id")) {
	try {
	    om_docid document_identifier= (om_docid)strtol((char*)id, NULL, 0);
	    OmDocument document= database->get_document (document_identifier);
	    return (document_identifier);
	} catch (OmDocNotFoundError & error) {
	    return 0;
	}
    }
    xmlNodePtr element = document->xmlChildrenNode;
    OmQuery query;
    while(element) {
	if(element->type==XML_ELEMENT_NODE) {
	    if(! xmlStrcmp(element->name,(const xmlChar*) "literal-term") && 
	       xmlGetProp(element,(const xmlChar*) "value") &&
	       xmlGetProp(element,(const xmlChar*) "unique") && 
	       ! xmlStrcmp(xmlGetProp(element,(const xmlChar*) "unique"),
			   (const xmlChar*) "yes")) {
		// add to query object
		if(!query.is_empty ()) {
		    query =OmQuery (OmQuery::OP_AND, query,OmQuery (string ((char*) xmlGetProp (element,(const xmlChar*) "value"))));
		}else {
		    query =OmQuery(string ((char*) xmlGetProp (element,(const xmlChar*) "value")));
		}
	    }
	}
	element = element->next;
    }
    if(query.is_empty ()) {
    	return 0;
    }
    OmEnquire *enquire = new OmEnquire(*database);
    enquire->set_query (query);
    OmMSet set = enquire->get_mset (0,2);
    switch(set.size ()) {
	om_docid r;
    case 0:
	delete enquire;
	// 	    cerr << "didn't find_document()" << endl;
	return 0;
	break;
    case 1:    
	r = *set[0];
	// 	    cerr << "find_document(): single " << r << endl;
	delete enquire;
	return r;
	
    default:
	r = *set[0];
	// 	    cerr << "find_document(): eek, multiple" << endl;
	delete enquire;
	return r;// not an error if we duplicate duplicate documents
	// throw string("unique literal terms did not identify a single document!");
    }
    
    delete enquire;
    return 0;
}

typedef struct {
    enum { DOC_ADDED, DOC_IGNORED, DOC_REINDEXED } status;
    om_docid doc_id;
} doc_status;

typedef enum {
    DUPLICATES_REINDEX,
    DUPLICATES_IGNORE,
    DUPLICATES_DUPLICATE
} duplicates_strategy;

doc_status add_document(OmWritableDatabase* database,xmlNodePtr document,
			string generator_string,
			duplicates_strategy duplicates)
{
    OmDocument new_document;
    int have_document = 0;
    om_termpos position = 0;
    xmlNodePtr element = document->xmlChildrenNode;
    om_docid previous = find_document(database,document); // 0 => not found

    if (previous>0 && duplicates==DUPLICATES_IGNORE) {
	doc_status result;
	result.doc_id = 0;
	result.status = doc_status::DOC_IGNORED;
	return result;
    }

    //    cout << "add_document, top node is '" << element->name << "'" << endl;

    while(element) {
	if(element->type==XML_ELEMENT_NODE && 
	   !xmlStrcmp(element->name,(const xmlChar*) "word-list")) {	
	    if (generator_string=="") {
		if (default_generator_string=="") {
		    default_generator_string = fetch_file(default_generator);
		}
		generator_string = default_generator_string;
	    }

	    if (generator_string=="") {
		throw string("generator was empty - this isn't going to work");
	    }

	    OmIndexerBuilder builder = OmIndexerBuilder();
	    OmIndexer generator = builder.build_from_string(generator_string);

	    xmlChar*offset = xmlGetProp(element,(const xmlChar*) "position-offset");
	    if(offset) {
		position +=strtol((char*) offset, NULL, 0);
	    }

	    xmlNodePtr pointer = element->xmlChildrenNode;
	    xmlNodePtr text= 0;
	    while(pointer) {
		if(pointer->type==XML_TEXT_NODE) {
		    if(text) {		    
			xmlNodePtr temp= xmlTextMerge (text, pointer);
			xmlFreeNode (text);
			if (!temp) {
			    throw string("Couldn't merge text nodes when preparing word list");
			}
			text = temp;
		    }else {
			text = xmlCopyNode(pointer, 0);
			if (!text) {
			    throw string("Couldn't copy initial text node when preparing word list");
			}
		    }
		}
		pointer = pointer->next;
	    }
	    if(text) {
		OmIndexerMessage message =OmIndexerMessage(string ((char*) text->content));
		generator.set_input (message);
		message = generator.get_raw_output ();
		if(message[0].get_string ()=="termlist") {
		    // term list
		    have_document = 1;// use existing one
		    om_termpos highest_position = 0;
		    for(size_t loop = 1; loop<message.get_vector_length ();
			loop ++) {
			OmIndexerMessage term = message [loop];
			om_termname termname =term [0].get_string ();
			for(size_t j= 0; j<term [3].get_vector_length (); 
			    j++) {
			    new_document.add_posting(termname, position + term [3][j].get_int());
			    if((om_termpos)term [3][j].get_int()>highest_position) {
				highest_position =term [3][j].get_int();
			    }
			}
		    }
		    position += highest_position;
		}else {
		    if(have_document) {
		        throw string("indexer produced a document but we already have one (multiple word-list elements?");
		    }else {
			have_document = 1;
			new_document = generator.get_output ();
		    }
		}
		xmlFreeNode (text);		
	    }// else nothing to do
	}
	element = element->next;    
    }
    // take document and add keys, literal terms and data
    // as we find it
    element =document->xmlChildrenNode;
    while(element) {
	if(element->type==XML_ELEMENT_NODE) {
	    if(!xmlStrcmp(element->name,(const xmlChar*) "key")) {
		if (xmlGetProp(element, (const xmlChar*) "number") &&
		    xmlGetProp(element, (const xmlChar*) "value")) {
		    xmlChar*number = xmlGetProp(element,(const xmlChar*) "number");
		    om_keyno key = strtol((char*) number,NULL, 0);
		    new_document.add_key (key, string ((char*)xmlGetProp (element,(const xmlChar*) "value")));
		} else {
		    throw string("key element missing required attribute");
		}
	    } else if(!xmlStrcmp(element->name,(const xmlChar*) "literal-term")) {
		if (xmlGetProp(element, (const xmlChar*) "value")) {
		    int wdf = 1;
		    if (xmlChar*wdf_string = xmlGetProp(element, (const xmlChar*) "wdf")) {
			wdf = strtol((char*)wdf_string, NULL, 0);
		    }
		    if (wdf==0) {
			wdf = 1;
		    }
		    new_document.add_term_nopos (string ((char*) xmlGetProp (element,(const xmlChar*) "value")), wdf);
		    if(xmlChar*pos=xmlGetProp(element,(const xmlChar*) "positions")) {
			string positions = string((char*)pos);
			while (positions!="") {
			    string::size_type it = positions.find(',');
			    if (it==string::npos) {
				it = string::npos;
			    }
			    int position = strtol(positions.c_str(), NULL, 0);
			    new_document.add_posting (string ((char*) xmlGetProp (element,(const xmlChar*) "value")), position, 0);
			    if (it==string::npos) {
				positions = "";
			    } else {
				positions.erase(0, it+1);
			    }
			}
		    }
		} else {
		    throw string("literal-term element missing required attribute 'value'");
		}
	    } else if(!xmlStrcmp(element->name,(const xmlChar*) "data")) {
	      xmlChar*data_format = xmlGetProp (element,(const xmlChar*) "type");
	      if (!data_format || !xmlStrcmp(data_format,(const xmlChar*) "inline")) {
		xmlNodePtr pointer = element->xmlChildrenNode;
		xmlNodePtr text= 0;
		while(pointer) {
		  if(pointer->type==XML_TEXT_NODE) {
		    if(text) {		    
		      xmlNodePtr temp= xmlTextMerge(text, pointer);
		      xmlFreeNode (text);
		      if (!temp) {
			throw string("Couldn't merge text nodes when building inline data");
		      }
		      text = temp;
		    }else {
		      text = xmlCopyNode(pointer, 0);
		      if (!text) {
			throw string("Couldn't copy initial text node when building inline data");
		      }
		    }
		  }
		  pointer = pointer->next;
		}
		if(text) {
		  new_document.set_data (string ((char*)text->content));
		  xmlFreeNode (text);
		}
	      } else if(!xmlStrcmp(data_format,(const xmlChar*) "fields")) {
		xmlNodePtr pointer = element->xmlChildrenNode;
		string data;
		while(pointer) {
		  if(pointer->type==XML_ELEMENT_NODE && 
		     !xmlStrcmp(pointer->name,(const xmlChar*) "field")) {
		    xmlChar*field_name = xmlGetProp (pointer,(const xmlChar*) "name");
		    if (!field_name) {
		      throw string("field element missing required attribute 'name'");
		    }
		    data +=  (char*) field_name;
		    data +="=";
		    xmlChar*field_value = xmlGetProp (pointer,(const xmlChar*) "value");
		    if (!field_value) {
		      throw string("field element missing required attribute 'value'");
		    }
		    data += (char*) field_value;
		    data += "\n";
		  }
		  pointer = pointer->next;
		}
		new_document.set_data (data);
	      } else {
		throw string("unrecognised document data type");
	      }
	    }
	}
	element = element->next;
    }

    doc_status result;
    if (previous>0 && duplicates == DUPLICATES_REINDEX) {
	    result.status = doc_status::DOC_REINDEXED;
	    result.doc_id = previous;
	    database->replace_document(previous, new_document);
    }
    if (previous==0 || duplicates == DUPLICATES_DUPLICATE) {
	result.status = doc_status::DOC_ADDED;
	result.doc_id = database->add_document (new_document);
    }
    return result;
}

void process_file(char* filename, bool auto_create)
{

  xmlDocPtr output = xmlNewDoc((const xmlChar*) "1.0");
  if (!output) {
      throw string("Couldn't create output document");
  }
  xmlNodePtr output_root = xmlNewDocNode(output, NULL,(const xmlChar*) "db-updated", NULL);
  if (!output_root) {
      xmlFreeDoc(output);
      throw string("Couldn't create output document root element");
  }
  xmlNsPtr output_namespace = xmlNewNs(output_root,(const xmlChar*) "http://xapian.org/schemas/dbtools/manage-results",NULL);
  if (!output_namespace) {
      xmlFreeDoc(output);
      throw string("Couldn't set output document namespace");
  }
  xmlDocSetRootElement(output, output_root);
  xmlSetNs (output_root, output_namespace);

  OmWritableDatabase* database= 0;
  xmlDocPtr doc = 0;

  try {
    // Read the input document
    doc = xmlParseFile(filename);
    if (doc==NULL) {
      throw string("could't open/parse file");
    }
    
    xmlNodePtr top = xmlDocGetRootElement(doc);
    if (top==NULL) {
      throw string("empty document");
    }
    
    xmlNsPtr ns = xmlSearchNsByHref(doc,top, (const xmlChar*)"http://xapian.org/schemas/dbtools/manage");
    if (ns== NULL) {
      throw string("wrong namespace");
    }
    if(xmlStrcmp(top->name, (const xmlChar *) "db-update")) {
      throw string("wrong root element");
    }
    
    xmlNodePtr current = top->xmlChildrenNode;
    OmSettings options;
    
    while(current && current->type!=XML_ELEMENT_NODE) {
      current = current->next;
    }
    
    // Open the database
    if(current && !xmlStrcmp(current->name, (const xmlChar*)"db-options")) {
      try {	
	options = read_db_options(doc, current);
	if (auto_create) {
	  bool d_ow = false;
	  bool d_cr = false;
	  try {
	    d_ow = options.get_bool("database_overwrite");
	  } catch (OmRangeError&) {
	  }
	  try {
	    d_cr = options.get_bool("database_create");
	  } catch (OmRangeError&) {
	  }
	  options.set("database_overwrite", false);
	  options.set("database_create", true);
	  try {
	    OmWritableDatabase db(options);
	  } catch (OmDatabaseCreateError&) {
	    // If it didn't exist, it should do by now ...
	  }
	  options.set("database_overwrite", d_ow);
	  options.set("database_create", d_cr);
	}
	database = new OmWritableDatabase(options);
      } catch (OmError& error) {
	throw error.get_msg();
      }
    } else {
      throw string("no db-options element");
    }
    current = current->next;

    // Perform actions in the input document
    while(current) {
      if(current->type==XML_ELEMENT_NODE) {      
	if(!xmlStrcmp(current->name,(const xmlChar*) "add")) {
	  duplicates_strategy duplicates = DUPLICATES_IGNORE;
	  xmlChar*strategy;
	  if((strategy= xmlGetProp(current,(const xmlChar*) "duplicates"))!=0) {
	    if(!xmlStrcmp(strategy,(const xmlChar*) "reindex")) {  
	      duplicates = DUPLICATES_REINDEX;
	    }else if (!xmlStrcmp(strategy,(const xmlChar*) "duplicate")) {
	      duplicates = DUPLICATES_DUPLICATE;
	    }
	  }
	  // 	  cerr << "duplicate strategy = " << duplicates << endl;
	  
	  xmlNodePtr  output_node = xmlNewChild (output_root, NULL,(const xmlChar*) "added", NULL);
	  if (output_node==NULL) {
	    throw string("Couldn't create output element for this 'add'");
	  }
	  xmlNodePtr document = current->xmlChildrenNode;
	  while(document && document->type!=XML_ELEMENT_NODE) {
	    document = document->next;
	  }
	  string generator = "";
	  
	  try {
	    while(document) {
	      if(document->type==XML_ELEMENT_NODE) {
		if(! xmlStrcmp(document->name,(const xmlChar*) "generator")) {
		  // you _are_ allowed to change generator partway
		  // through an <add>, but we don't advise it
		  try {
		    generator = make_generator  (doc,document->xmlChildrenNode);
		  } catch (OmError & error) {
		    throw string("Couldn't create generator: " + error.get_msg());
		  }
		} else if (!xmlStrcmp(document->name, (const xmlChar*)"doc")) {
		  xmlNodePtr  output_document = xmlNewChild (output_node, NULL,(const xmlChar*) "doc", NULL);
		  if (output_document==NULL) {
		    throw string("Couldn't create output element for this 'doc'");
		  }
		  try {
		    doc_status status = add_document (database, document, generator, duplicates);
		    if (status.doc_id > 0) {
		      char  string_id[20];
		      sprintf (string_id, "%.19i", status.doc_id);
		      if ((xmlSetProp (output_document,(const xmlChar*) "doc-id", (const xmlChar*) string ( string_id).c_str()))==NULL) {
			throw string("Couldn't set doc-id property of output element for this 'doc'");
		      }
		    }
		    switch (status.status) {
		    case doc_status::DOC_ADDED:
		      break; // nothing special
		    case doc_status::DOC_IGNORED:
		      if ((xmlSetProp(output_document,(const xmlChar*)"ignored", (const xmlChar*) "yes"))==NULL) {
			throw string("Couldn't set ignored property of output element for this 'doc'");
		      }
		      break;
		    case doc_status::DOC_REINDEXED:
		      if ((xmlSetProp(output_document,(const xmlChar*)"reindexed", (const xmlChar*) "yes"))==NULL) {
			throw string("Couldn't set reindexed property of output element for this 'doc'");
		      }
		      break;
		    }
		  } catch (string& s) {
		    if ((xmlSetProp(output_document, (const xmlChar*) "error", (const xmlChar*) s.c_str()))==NULL) {
		      throw string(s + " (and couldn't set error property for this 'doc'");
		    }
		  } catch (OmError &e) {
		    if ((xmlSetProp (output_document,(const xmlChar*) "error", (const xmlChar*) e.get_msg().c_str()))==NULL) {
		      throw string(e.get_msg() + " (and couldn't set error property for this 'doc'");
		    }
		  }
		}
	      } // else not a node
	      document = document->next;
	    }
	  } catch (string &s) {
	    if ((xmlSetProp (output_node,(const xmlChar*) "error",
			     (const xmlChar*) s.c_str()))==NULL) {
	      throw s;
	    }
	  }
	} else if (! xmlStrcmp(current->name,(const xmlChar*) "delete")) {
	  xmlNodePtr  output_node = xmlNewChild (output_root, NULL,(const xmlChar*) "deleted", NULL);
	  if (output_node==NULL) {
	    throw string("couldn't create output element for 'delete'");
	  }
	  xmlNodePtr document = current->xmlChildrenNode;
	  try {
	    while(document) {
	      if(document->type==XML_ELEMENT_NODE) {
		xmlNodePtr  output_document = xmlNewChild (output_node, NULL,(const xmlChar*) "doc", NULL);
		if (output_document==NULL) {
		  throw string("couldn't create output element for this 'doc'");
		}
		om_docid id=0;
		try {
		  id = find_document (database,document);
		} catch (OmError&error) {
		  if ((xmlSetProp(output_document,(const xmlChar*) "error",(const xmlChar*) error.get_msg().c_str()))==NULL) {
		    throw string(error.get_msg() + " (and couldn't set error property of output element for this 'doc'");
		  }
		  document = document->next;
		  continue;
		} catch (string&error) {
		  if ((xmlSetProp(output_document,(const xmlChar*) "error",(const xmlChar*) error.c_str()))==NULL) {
		    throw string(error+ " (and couldn't set error property of output element for this 'doc'");
		  }
		  document = document->next;
		  continue;
		}
		if(id==0) {
		  if ((xmlSetProp(output_document,(const xmlChar*) "error",(const xmlChar*) "document not found"))==NULL) {
		    throw string("document not found (and couldn't set error property of output element for this 'doc'");
		  }
		}else {
		  try {
		    database->delete_document (id);
		    
		    char  string_id[20];
		    sprintf (string_id, "%.19i", id);
		    if ((xmlSetProp (output_document,(const xmlChar*) "doc-id", (const xmlChar*) string (string_id).c_str()))==NULL) {
		      throw string("couldn't set doc-id property of output element");
		    }
		  } catch (OmError&error) {
		    if ((xmlSetProp (output_document,(const xmlChar*) "error",
				     (const xmlChar*) error.get_msg().c_str()))==NULL) {
		      throw string(error.get_msg() + " (and couldn't set error property of output element for this 'doc'");
		    }
		  }
		}
	      }
	      document = document->next;
	    }
	  } catch (string& s) {
	    if ((xmlSetProp(output_node, (const xmlChar*)"error", (const xmlChar*)s.c_str()))==NULL) {
	      throw string(s + " (and couldn't set error property of 'deleted' element)");
	    }
	  }
	}
      }
      current = current->next;
    }
  } catch (string& s) {
    // Don't need to catch OmError -- always handled above
    if(database) {
      delete database;
    }
    if (doc) {
      xmlFreeDoc(doc);
    }
    bool handled = true;
    if ((xmlSetProp(output_root, (const xmlChar*)"error", (const xmlChar*)s.c_str()))==NULL) {
      handled = false;
    }
#ifdef HAVE_LIBXML2    
    int result = xmlDocDump (stdout, output);
#else
    int result = 0;
    xmlDocDump (stdout, output);
#endif
    xmlFreeDoc(output);
    if (result==-1) {
      throw string(s +" (and couldn't dump output document)");
    } else if (!handled) {
      throw s;
    } else {
      return;
    }
  }

  if(database) {
    delete database;
  }
  if (doc) {
    xmlFreeDoc(doc);
  }
#ifdef HAVE_LIBXML2    
  int s = xmlDocDump (stdout, output);
#else
  int s = 0;
  xmlDocDump (stdout, output);
#endif
  xmlFreeDoc(output);
  if (s==-1) {
    throw string("couldn't dump output document");
  }
  return;
}

int main(int argc, char** argv)
{
    // getopt
    char* optstring = "hvcg:";
    int longindex, getopt_ret;
    bool auto_create = false;

#ifdef HAVE_GETOPT_LONG
    struct option longopts[5];
    longopts[0].name = "help";
    longopts[0].has_arg = 0;
    longopts[0].flag = NULL;
    longopts[0].val = 'h';
    longopts[1].name = "version";
    longopts[1].has_arg = 0;
    longopts[1].flag = NULL;
    longopts[1].val = 'v';
    longopts[2].name = "auto-create";
    longopts[2].has_arg = 0;
    longopts[2].flag = NULL;
    longopts[2].val = 'c';
    longopts[3].name = "generator";
    longopts[3].has_arg = required_argument;
    longopts[3].flag = 0;
    longopts[3].val = 'g';
    longopts[4].name = 0;
    longopts[4].has_arg = 0;
    longopts[4].flag = 0;
    longopts[4].val = 0;

    while ((getopt_ret = getopt_long(argc, argv, optstring,
                                     longopts, &longindex))!=EOF) {
#else
    while ((getopt_ret = getopt(argc, argv, optstring))!=EOF) {
#endif
        switch (getopt_ret) {
        case 'h':
            cout << "Usage: " << argv[0] << " [OPTION] FILE" << endl
	         << endl << "Manage an Xapian database via XML files." << endl
#ifdef HAVE_GETOPT_LONG
                 << "  -c, --auto-create\tcreate database if necessary" << endl
                 << "  -g, --generator\tdefault indexgraph generator file" << endl
                 << "  -h, --help\t\tdisplay this help and exit" << endl
                 << "  -v --version\t\toutput version and exit" << endl << endl
#else
                 << "  -c\t\tcreate database if necessary" << endl
                 << "  -g\t\tdefault indexgraph generator file" << endl
                 << "  -h\t\tdisplay this help and exit" << endl
                 << "  -v\t\toutput version and exit" << endl << endl
#endif
                 << "Report bugs via the web interface at:" << endl
                 << "<http://xapian.org/bugs/>" << endl;
            return 0;
            break;
        case 'v':
            cout << PACKAGE << " " << VERSION << endl;
            cout << "Copyright 2001 tangozebra ltd" << endl << endl;
            cout << "This is free software, and may be redistributed under" << endl;
            cout << "the terms of the GNU Public License." << endl;
            return 0;
            break;
	case 'c':
	    auto_create = true;
	    break;
	case 'g':
	    default_generator = string(optarg);
	    break;
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
    try {
	process_file(argv[optind], auto_create);
    } catch (string& s) {
	cerr << "Error while processing '" << argv[optind] << "': " << s << endl;
	return 2;
    }
    return 0;
}
