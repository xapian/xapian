/*
 * $Id$
 *
 * Dump a Xapian database to an XML representation that xdb-manage can use
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
#include <string>
#include <xmlmemory.h>
#include <parser.h>

#include "config.h"

OmSettings read_db_options(xmlDocPtr doc, xmlNodePtr current)
{
    if (xmlStrcmp(current->name, (const xmlChar*)"db-options")) {
	throw string("not a db-options element");
    }
    xmlNsPtr ns = xmlSearchNsByHref(doc, current, (const xmlChar*)"http://xapian.org/schemas/dbtools/db-options");
    if (ns==NULL) {
	throw string("namespace on db-options not recognised");
    }
    OmSettings options;
    xmlNodePtr parameter = current->xmlChildrenNode;
    while(parameter) {
	if(parameter->type==XML_ELEMENT_NODE  && 
	   !xmlStrcmp(parameter->name,(const xmlChar*) "param")) {
	    if (!xmlGetProp(parameter, (const xmlChar*) "name") ||
		!xmlGetProp(parameter, (const xmlChar*) "value")) {
		throw string("param element missing required attribute");
	    }
	    xmlChar* type = xmlGetProp(parameter,(const xmlChar*) "type");
	    if(!type || !xmlStrcmp(type,(const xmlChar*) "string")) {
		options.set((char*) xmlGetProp(parameter,(const xmlChar*) "name"), (const char*) xmlGetProp(parameter,(const xmlChar*) "value"));
	    } else if (!xmlStrcmp(type,(const xmlChar*) "boolean")) {
		options.set((char*) xmlGetProp(parameter,(const xmlChar*) "name"), (xmlStrcmp(xmlGetProp (parameter,(const xmlChar*) "value"),(const xmlChar*) "true")==0));
	    } else if (!xmlStrcmp(type,(const xmlChar*) "integer")) {
		int number =strtol((char*) xmlGetProp (parameter,(const xmlChar*) "value"), NULL, 0);
		options.set((char*) xmlGetProp(parameter,(const xmlChar*) "name"), number);
	    } else if (!xmlStrcmp(type,(const xmlChar*) "double")) {
		double number =strtod((char*) xmlGetProp (parameter,(const xmlChar*) "value"), 0);
		options.set((char*) xmlGetProp(parameter,(const xmlChar*) "name"), number);
	    } else {
		throw string("param element had unknown type");
	    }
	}
	parameter = parameter->next;
    }
    return options;
}
