/** @file handler_libebook.cc
 * @brief @brief Extract text and metadata using libe-book.
 */
/* Copyright (C) 2011 Olly Betts
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
#include "handler.h"

#include <xapian.h>
#include <string.h>
#include <unordered_map>
#include <memory>

#include <librevenge-0.0/librevenge-generators/librevenge-generators.h>
#include <librevenge-0.0/librevenge-stream/librevenge-stream.h>
#include <iostream>
#include <libe-book/libe-book.h>

using libebook::EBOOKDocument;
using namespace std;
using namespace librevenge;

static
char * str_copy(const char * text){
    int len = strlen(text);
    char * line = new char[len];
    strcpy(line,text);
    return line;
}

static
void clear_text(string & str,const char * text){
    int len = strlen(text);
    for(int i=0;i<len;++i)
	if (!isspace(text[i]) || (i && !isspace(text[i - 1])))
	    str.push_back(text[i]);
}

bool
extract(const string & filename,
	string & dump,
	string & title,
	string & keywords,
	string & author,
	string & pages)
{
    try {
	shared_ptr<RVNGInputStream> input;
	RVNGString str_dump;
	const char * InputFile = filename.c_str();

	if (RVNGDirectoryStream::isDirectory(InputFile))
	    input.reset(new RVNGDirectoryStream(InputFile));
	else
	    input.reset(new RVNGFileStream(InputFile));

	EBOOKDocument::Type type = EBOOKDocument::TYPE_UNKNOWN;
	EBOOKDocument::Confidence confidence =
				EBOOKDocument::isSupported(input.get(), &type);

	if (EBOOKDocument::CONFIDENCE_SUPPORTED_PART == confidence){
	    input.reset(RVNGDirectoryStream::createForParent(InputFile));
	    confidence = EBOOKDocument::isSupported(input.get(), &type);
	}

	if ((EBOOKDocument::CONFIDENCE_EXCELLENT != confidence) &&
	    (EBOOKDocument::CONFIDENCE_WEAK != confidence))
	    return false;

	RVNGTextTextGenerator metadata(str_dump, true);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &metadata, type))
	    return false;

	unordered_map<string,char *> hash;
	char * str = str_copy(str_dump.cstr());
	char * pch = strtok(str,"\n");
	while (pch != NULL){
	    int l = strlen(pch);
	    for(int i=0;i<l;i++)if(pch[i]==' '){
		pch[i]='\0';
		hash[pch] = &pch[i+1];
		break;
	    }
	    pch = strtok (NULL, "\n");
	}
	unordered_map<string,char *>::const_iterator it;
	unordered_map<string,char *>::const_iterator lost = hash.end();

	// Extract Author if it possible
	if((it=hash.find("dc:creator"))!=lost)
	    clear_text(author,it->second);
	else if((it=hash.find("meta:initial-creator"))!=lost)
	    clear_text(author,it->second);

	// Extract Title if it possible
	if((it=hash.find("dc:title"))!=lost)
	    clear_text(title,it->second);

	// Extract Keywords if it possible
	if((it=hash.find("dc:subject"))!=lost)
	    clear_text(keywords,it->second);
	if((it=hash.find("dc:publisher"))!=lost)
	    clear_text(keywords,it->second);
	if((it=hash.find("dcterms:available"))!=lost)
	    clear_text(keywords,it->second);
	pages = "";

	// Extract Dump if it possible
	RVNGTextTextGenerator content(str_dump, false);

	if (EBOOKDocument::RESULT_OK !=
	    EBOOKDocument::parse(input.get(), &content, type))
	    return false;
	clear_text(dump,str_dump.cstr());
    } catch (...) {
	cerr << "Libe-book threw an exception" << endl;
	return false;
    }
    return true;
}
