/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <map>
#include <list>
#include <string>
#include <vector>

typedef unsigned int termid;
typedef unsigned int docid;
typedef char *termname;

class Error {
    private:
	string msg;
    public:
        Error(string error_msg)
        {
	    msg = error_msg;
        }
	string get_msg()
	{
	    return msg;
	}
};

class RangeError : public Error {
    public:
	RangeError(string msg) : Error(msg) {};
};

class OpeningError : public Error {
    public:
        OpeningError(string msg) : Error(msg) {};
};

class PostListIterator {
    private:
	
    public:
	PostListIterator();
	void   open(termid);    // Throws RangeError if termid invalid
	docid  get_docid();     // Gets current docid
	void   next();          // Moves to next docid
	void   skip_to(docid);  // Moves to next docid >= specified docid
	bool   at_end();        // True if iterator is off the end of the list
	void   close();
};

class TermListIterator {
    private:
    public:
	TermListIterator();
	void   open(docid);     // Throws InvalidIDException if docid invalid
	termid get_termid();    // Gets current termid
	void   next();          // Moves to next termid
	void   skip_to(termid); // Moves to next termid >= specified termid
	bool   at_end();        // True if iterator is off the end of the list
	void   close();
};

class IRDatabase {
    private:
    public:
	termid term_name_to_id(termname);
	termname term_id_to_name(termid);
    
	void open(string);
	void close();
};

class DADatabase : public IRDatabase {
    private:
        termid max_termid;
        map<termname, termid> termidmap;
        vector<termname> termidvec;
    public:
	termid term_name_to_id(termname);
	termname term_id_to_name(termid);

        DADatabase();
};

#endif /* _database_h_ */
