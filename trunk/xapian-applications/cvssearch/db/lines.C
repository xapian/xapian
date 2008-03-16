#include <xapian.h>
#include "lines.h"
#include "util.h"

lines::lines(const string & r, const string & p, const string & m)
    :
    root(r),
    package(p),
    message(m)
{
    static string cvsdata = get_cvsdata();
    path = cvsdata + "/" + root + "/src/";
    stemmer = new Xapian::Stem("english");
    current_fn = "";
    line_no = 0;  
    file_count = 0;
    in_code = 0;
}

lines::~lines() 
{
    delete stemmer;
    if ( in_code != 0 ) {
        delete in_code;
    }
}


void
lines::extractSymbols( const string& s, set <string> & symbols, list<string>& symbol_list, bool do_qualified_classes) {
    string current = "";
    bool foundBlank = false;
    for ( string::const_iterator i = s.begin(); i != s.end(); i++ ) {
        char c = *i;
        
        if ( blankChar(c) ) {
            if ( current != "" ) {
                foundBlank = true;
            }
            continue;
        }
        
        if ( current == "" ) {
            if ( okFirstChar(c) ) {
                current = c;
            }
        } else {
            // already started something
            if (! okSubChar(c) && ( !do_qualified_classes || c != ':' ) ) {
                if ( c == '(' ) {
                    assert( current != "" );
                    current += "()";
                    //cerr << "... found " << current << endl;
		    if ( !do_qualified_classes || (current.find("::") != -1 && current.find("()") == -1 )) {
		      symbols.insert(current);
		      symbol_list.push_back(current);
		    }
                    current = "";
                    foundBlank = false;
                } else {
                    // identifier ended
                    //cerr << "... found " << current << endl;
                    assert( current != "" );

		    if ( !do_qualified_classes || (current.find("::") != -1 && current.find("()") == -1 ) ) {
		      symbols.insert(current);
		      symbol_list.push_back(current);
		    }

                    current = "";
                    foundBlank = false;
                }
            } else { // okay subsequent character
                if ( foundBlank ) {
                    assert( current != "" );

		    if ( !do_qualified_classes || (current.find("::") != -1 && current.find("()") == -1 ) ) {
		      symbols.insert(current);
		      symbol_list.push_back(current);
		    }
		    
                    current = "";	  
                    foundBlank = false;
                }
                current += c;
            }
        }
    } 
    if ( current != "" ) {
        //    cerr << "...found " << current << endl;
      if ( !do_qualified_classes || (current.find("::") != -1 && current.find("()") == -1) ) {
        symbols.insert(current);
	symbol_list.push_back(current);
      }
    }
}

void
lines::stemWords( const list<string>& words, list<string>& term_list )
{
    for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
	string term = *i;
        lowercase_term(term);
        term = stemmer->stem_word(term);
        terms.insert(term);
        term_list.push_back(term);
    }
}

void
lines::updateRevisionComments( map< string, list<string> >& rcw ) {
    map< string, list<string > >::iterator i;
    for( i = revision_comment_words.begin(); i != revision_comment_words.end(); i++ ) {
        if ( rcw[i->first].empty() ) {
            rcw[ i->first ] = i->second;
        }
    }
}

#if SHOW_WARNINGS
#warning "doesn't handle all upper case yet"
#endif

const list<string>
lines::getCodeSymbolTerms() {
    // ----------------------------------------
    // computed here, since may not be required
    // by some apps
    // ----------------------------------------
    list<string> code_terms;
    
    for( list<string>::iterator s = symbol_list.begin(); s != symbol_list.end(); s++ ) {
        //cerr << "** symbol " << (*s) << endl;
        string w = "";
        for( string::const_iterator c = s->begin(); c != s->end(); c++ ) {
            
            if ( (*c) == '(' || (*c) == ')' ) {
                continue;
            }
            
            if ( ((*c) >= 'A' && (*c) <= 'Z') || (*c) == '_' || isdigit(*c) ) {
                if ( w != "" ) {
                    lowercase_string(w);
		    //                    w = stemmer->stem_word(w);
                    //		  cerr << "........inserting " << w << endl;
                    code_terms.push_back(w);
                    w = "";
                }
            }
            if ( (*c) != '_' && !isdigit(*c) ) {
                w += (*c);
            }
      
        }
        if ( w != "" ) {
            lowercase_string(w);
            //w = stemmer->stem_word(w);
           // 	      cerr << "........inserting " << w << endl;
	    code_terms.push_back(w);
        }
    }
    list<string> stemmed;
    stemWords( code_terms, stemmed );
    return stemmed;
}

