#ifndef __LINES_H__
#define __LINES_H__

#include <vector>
#include <set>
#include <list>
#include <map>
#include <string>
#include <fstream>
using namespace std;

namespace Xapian {
class Stem;
}
class lines
{
protected:
    // ----------------------------------------
    // Given a line of source code, pick up all
    // classes and functions in that line
    // and insert them into the symbols set.
    // ----------------------------------------
    static void extractSymbols(const string & s, set <string> & symbols, list<string>& symbol_list, bool do_qualified_classes );

    // ----------------------------------------
    // takes a list of words, lower cases and 
    // stems each word, and puts the result
    // in another list
    // ----------------------------------------
    void stemWords(const list<string>& words, list<string>& term_list);

    string current_fn;
    string data;
    string codelinedata;
    string codeline;
    list<string> term_list;
    set <string> symbols;
    set <string> qualified_classes; // example: KParts::ReadOnlyPart
    list <string> qualified_class_list;
    list<string> symbol_list;
    set <string> terms;
    unsigned int line_no;
    map< string, list<string> > revision_comment_words;
    map< string, string >       revision_comment_string;

    Xapian::Stem *stemmer;
    ifstream *in_code;
    unsigned int file_count;
    string path;
    string root;
    string package;
    string message;

public:
    lines(const string & root, const string & package, const string & message);
    virtual ~lines();

    // ----------------------------------------
    // Reads next line entry. 
    // Moves on to next file when previous file done
    //
    // Returns false when there are no more files
    // to read lines from
    // ----------------------------------------
    virtual bool readNextLine() = 0;



    // ----------------------------------------
    // functions above need implementation
    // by subclass.
    // 
    // functions below are defined in this class
    // ----------------------------------------

    // -----------------------------------------
    // Suppose a line is associated with revision 
    // comments C1, C2, C3 for revisions R1, R2, R3.
    //
    // then the map is updated as follows:
    //
    // R1 -> list of lowercased, stemmed C1 words
    // R2 -> list of lowercased, stemmed C2 words
    // R3 -> list of lowercased, stemmed C3 words
    //
    // Observe that the map is not cleared. That is,
    // calling this method multiple times just keeps 
    // making the map bigger (so that we can build
    // a map for all revisions of interest)
    // ----------------------------------------
    void updateRevisionComments( map< string, list<string> >& revision_comment_words);
    
    // ----------------------------------------
    // Gets a *set* of all the words in the 
    // combined string C1 & C2 & C3.  The words 
    // in this set are lowercased and stemmed.
    // ----------------------------------------
    const set<string> & getCommentTerms()    const { return terms;}

    // ----------------------------------------
    // Code symbols (functions/classes) from 
    // line of code just read.
    // ----------------------------------------
    const set<string> & getCodeSymbols()     const { return symbols;}

    const set<string> & getQualifiedClasses() const { return qualified_classes; }

    // ----------------------------------------
    // getTermList returns a *list* of all words
    // in the combined string  C1 & C2 & C3 
    // (in that order). The order and frequency 
    // of words is preserved in this case.  
    // Again, the words are lowercased and stemmed.
    // ----------------------------------------
    const list<string>& getTermList()        const { return term_list;}

    const list<string>& getQualifiedClassList() const { return qualified_class_list; }

    // ----------------------------------------
    // Suppose a line is associated with revision
    // comments C1, C2, C3 for revisions R1, R2, R3.
    //
    // We return a map of the form:
    //
    // R1 -> list of lowercased, stemmed C1 words
    // R2 -> list of lowercased, stemmed C2 words
    // R3 -> list of lowercased, stemmed C3 words
    //
    // Observe this contains information for the
    // line just read only.
    // ----------------------------------------
    const map< string, list<string> > & getRevisionCommentWords() const { return revision_comment_words;}

    // ----------------------------------------
    // Like above, but we have a map of the form:
    //
    // R1 -> C1 string
    // R2 -> C2 string
    // R3 -> C3 string
    // ----------------------------------------
    const map< string, string >       & getRevisionCommentString() const { return revision_comment_string;}
    
    // ----------------------------------------
    // Gets the current file name.
    // ----------------------------------------
    const string      & getCurrentFile()     const { return current_fn;}

    // ----------------------------------------
    // This is the data string returned by the 
    // cvssearch command
    // it looks something like this:
    // 65 1074:root0 kdenetwork_kmail 63:1.71 1.52
    // ----------------------------------------
    const string      & getData()            const { return data;}

    // ----------------------------------------
    // This is the data string that may be used
    // in the future to do grep searches by 
    // using a single file per application
    //
    // It looks a lot like data from previous 
    // member getData() but also includes 
    // the line content afterwards. 
    // The idea is to just grep it and look at 
    // the data preceding each line in the grep 
    // results.
    // ----------------------------------------
    const string      & getCodeLineData()    const { return codelinedata;}

    // ----------------------------------------
    // Gets the actual contents of the line 
    // just read.
    // ----------------------------------------
    const string      & getCodeLine()        const { return codeline;}

    // ----------------------------------------
    // Looks at all the functions/classes 
    // associated with the line just read, and 
    // returns the set of all words in all 
    // these functions/classes.
    //
    // For example, if the symbols are:  
    // startTimer() and TimerEvent, 
    // it returns the set { start, timer, event}.
    // 
    // its computed here, since may not be 
    // required by some apps, so the output is
    // not a reference.
    // ----------------------------------------
    const list<string>   getCodeSymbolTerms();

    // ----------------------------------------
    // Gets the number of line just read from 
    // current file.
    // ----------------------------------------
    unsigned int        getLineNumber() const { return line_no; }
};

#endif
