#include "lines.h"
#include "util.h"

lines::lines(const string & src_path, const string & r, const string & p, const string & m)
    :path(src_path),
     root(r),
     package(p),
     message(m)
{
    stemmer = new OmStem("english");
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
lines::extractSymbols( const string& s, set <string> & symbols) {
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
            if (! okSubChar(c) ) {
                if ( c == '(' ) {
                    assert( current != "" );
                    current += "()";
                    //cerr << "... found " << current << endl;
                    symbols.insert(current);
                    current = "";
                    foundBlank = false;
                } else {
                    // identifier ended
                    //cerr << "... found " << current << endl;
                    assert( current != "" );
                    symbols.insert(current);
                    current = "";
                    foundBlank = false;
                }
            } else { // okay subsequent character
                if ( foundBlank ) {
                    assert( current != "" );
                    symbols.insert(current);
                    current = "";	  
                    foundBlank = false;
                }
                current += c;
            }
        }
    } 
    if ( current != "" ) {
        //    cerr << "...found " << current << endl;
        symbols.insert(current);
    }
}

void
lines::stemWords( const list<string>& words, list<string>& term_list )
{
    for( list<string>::const_iterator i = words.begin(); i != words.end(); i++ ) {
        string word = *i;
        om_termname term = word;
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

const set<string>
lines::getCodeSymbolTerms() {
    // ----------------------------------------
    // computed here, since may not be required
    // by some apps
    // ----------------------------------------
    set<string> code_terms;
    
    for( set<string>::iterator s = symbols.begin(); s != symbols.end(); s++ ) {
        string w = "";
        for( string::const_iterator c = s->begin(); c != s->end(); c++ ) {
            
            if ( (*c) == '(' || (*c) == ')' ) {
                continue;
            }
            
            if ( ((*c) >= 'A' && (*c) <= 'Z') || (*c) == '_' ) {
                if ( w != "" ) {
                    lowercase_string(w);
                    w = stemmer->stem_word(w);
                    //		  cerr << "........inserting " << w << endl;
                    code_terms.insert(w);
                    w = "";
                }
            }
            if ( (*c) != '_' ) {
                w += (*c);
            }
      
        }
        if ( w != "" ) {
            lowercase_string(w);
            w = stemmer->stem_word(w);
            //	      cerr << "........inserting " << w << endl;
            code_terms.insert(w);
        }
    }
    return code_terms;
}

