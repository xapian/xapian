/* indexnasty.cc based on simpleindex.cc and omindex.cc
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Sam Liddicott
 * Copyright 2001 Ananova Ltd
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

/*

File format is multiple segments separated by blank lines, each segment
representing a record.

A segment consists of multiple single line fields with a header,

<header>=<data>

A head may optionally be prefixed with !,~ or *, followed by an optional ':' 
terminated prefix, followed by am optional field name; 

i.e.
  [[!|~|*][prefix:]][fieldname]=

Prefix and fieldname may be null.
If ~ is specified then the data is stemmed for terms.
If a * is specified then the prefixed data must be unique and if a record
already exists with this it is overwritten
If a ! is specified
If a prefix is specified then terms are prefixed.
If neither * or ~ or a prefix or a : are specified the field is not used for terms

If a fieldname is specified then the data is stored as a line in the document
data field.

e.g.

~P:P1=Stem terms with prefix P, also store in field P1
Q:Q1=Don't stem terms but give prefix Q, also store in field Q1
~:R1=Stem terms, no prefix and sore as R1
:S1=No Stem terms, no prefix and store as S1
~T:=Stem terms, prefix as T, no store
U:=No stem terms , prefix U, no store
~:=Stem terms, no prefix, no store
:=No stem terms, no prefix, no store
X=No terms, but still store
*Q0:q0=23
~:Text=Jack and Jill went up the hill

*/

#include <memory>
#include <fstream>
#include <om/om.h>
#include <string>
using std::string;

static bool IndexNastyFile(string Filepath, OmWritableDatabase &database,
			   OmStem &stemmer);

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we require one or more parameters.
    if (argc < 2) {
	std::cout << "Usage: " << argv[0] <<
		" <path to database> [<filename>]..." <<
		std::endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Make the database
	OmSettings settings;
	settings.set("backend", "quartz");
	settings.set("quartz_dir", argv[1]);
	OmWritableDatabase database(settings);

	OmStem stemmer("english"); 

	// Read file/s
        om_docid docid;
	if (argc==2) docid=IndexNastyFile("",database,stemmer);
        else for (int i=2; i < argc; i++) {
          docid=IndexNastyFile(argv[i],database,stemmer);
        }

    }
    catch (OmError &error) {
	std::cout << "Exception: "  << error.get_msg() << std::endl;
    }
}

static void
lowercase_term(om_termname &term)
{
    om_termname::iterator i = term.begin();
    while (i != term.end()) {
        *i = tolower(*i);
        i++;
    }
} 

inline static bool
p_alpha(unsigned int c)
{
    return ((c | 32) - 'a') <= ('z' - 'a');
}

inline static bool
p_notalnum(unsigned int c)
{
    return !isalnum(c);
}

inline static bool
p_notplusminus(unsigned int c)
{
    return c != '+' && c != '-';
}

static om_termpos
index_text(const string &s, OmDocument &doc, OmStem &stemmer, om_termpos pos)
{
    std::string::const_iterator i, j = s.begin(), k;
    while ((i = find_if(j, s.end(), p_alpha)) != s.end()) {
        j = find_if(i, s.end(), p_notalnum);
        k = find_if(j, s.end(), p_notplusminus);
        if (k == s.end() || !isalnum(*k)) j = k;
        om_termname term = s.substr(i - s.begin(), j - i);
        lowercase_term(term);
        if (isupper(*i) || isdigit(*i)) {
	    doc.add_posting(term, pos);
        }
 
        term = stemmer.stem_word(term);
        doc.add_posting(term, pos++);
        i = j + 1;
    }
    return pos;
}                           

static bool
IndexNastyFile(string Filepath, OmWritableDatabase &database, OmStem &stemmer)
{
  if (Filepath.empty()) Filepath = "/dev/fd/0";

  std::ifstream stream(Filepath.c_str());

  if (!stream) {
    std::cout << "Can't open file " << Filepath << std::endl;
    return false;
  }

  // Make the document
  om_docid docid=0;
  int pending=0;
#define MAX_PENDING 10

  while (!stream.eof()) {
    OmDocument newdocument;
    docid=0;
    string data="";
    int wordcount=0;
    string line;
    string uniqueterm="";

    while (getline(stream,line) && !line.empty()) {
      bool stem, term, unique;
      string prefix="";
      string field="";
      string header;
      string key="";
      int index=0;
      int cursor = line.find('=', index);

      if (cursor != string::npos) {
        int hindex=0;
        header=line.substr(index,cursor-index);
        if (header.empty()) docid=atoi(line.substr(cursor+1).c_str());
        // if header has no : but has ~ then stem no prefix no store
        // if header has no : but has not ~ then just store 

        // scan up to ! if present to get key
	int hcursor = header.find('!', hindex);
	if (hcursor != string::npos) {
          key=header.substr(hindex,hcursor-hindex);
          header=header.substr(hcursor + 1);
        }

        if (stem=(header[hindex]=='~')) hindex++;
        if (unique=(header[hindex]=='*')) hindex++;

        // now scan up to colon to get terms prefix
	hcursor = header.find(':', hindex);
	term = hcursor != string::npos;
        if (term) {
          prefix = header.substr(hindex, hcursor-hindex);
          field = header.substr(hcursor + 1);
        } else { // No ':' 
          if (term=(stem || unique)) prefix=header.substr(hindex,header.length()-hindex);
          else field=header;
        }

        cursor++; // skip past '='
        string text=line.substr(cursor);

	if (!field.empty() && !text.empty()) data+=field+'='+text+'\n';
	if (!key.empty() && !text.empty()) newdocument.add_key(atoi(key.c_str()),text);

        // now index field if required
        if (term) {
          if (stem) index_text(text, newdocument, stemmer, wordcount);
          else {
            if (unique) { // note unique field - only one per document tho!
	      uniqueterm=prefix+text;
	    }
	    if (!text.empty()) newdocument.add_posting(prefix+text,wordcount++);
	  }
        }
      } else std::cout << "Bad line: " << cursor << " " << line << std::endl;
    }

    if (!data.empty()) {
      // Put the data in the document
      newdocument.set_data(data);

      // Add the document to the database
      if (!uniqueterm.empty() && (database.term_exists(uniqueterm))) {
        try { // nicked from omindex.cc
          OmEnquire enq(database);
	  enq.set_query(OmQuery(uniqueterm));
	  OmMSet mset = enq.get_mset(0, 1);
	  try {
	    database.replace_document(*mset[0], newdocument);
	    docid=*mset[0];
	  } catch (...) {
            docid=database.add_document(newdocument);
	  }
        } catch (...) {
          docid=database.add_document(newdocument);
        }
      } else {
        docid=database.add_document(newdocument);
      }
    } // end of if !data.empty()

//    if (docid) {
//      try {
//        database.replace_document(docid,newdocument);
//      } catch (...) {
//        docid=database.add_document(newdocument);
//      }
//    }
//    else 

//    if (++pending>MAX_PENDING) {
//      pending=0;
//      database.flush();
//      std::cout << "Flushed" << std::endl;
//    }

    std::cout << docid << " " << uniqueterm << std::endl;
  }

  return true;
}
