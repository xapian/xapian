/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <map>
#include <list>
#include <string>
#include <vector>

#include "omtypes.h"

class OmError {
    private:
        string msg;
    public:
        OmError(string error_msg)
        {
            msg = error_msg;
        }
        string get_msg()
        {
            return msg;
        }
};

class RangeError : public OmError {
    public:
	RangeError(string msg) : OmError(msg) {};
};

class OpeningError : public OmError {
    public:
        OpeningError(string msg) : OmError(msg) {};
};

class PostListIterator {
    private:
    public:
	docid  get_docid();     // Gets current docid
	void   next();          // Moves to next docid
	void   skip_to(docid);  // Moves to next docid >= specified docid
	bool   at_end();        // True if iterator is off the end of the list
	void   close();
};

class TermListIterator {
    private:
    public:
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
    
	void open(string pathname, bool readonly);
	void close();

	// Throws RangeError if termid invalid
	PostListIterator open_post_list(termid);

	// Throws RangeError if docid invalid
	TermListIterator open_term_list(docid);
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
