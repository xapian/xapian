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
	virtual docid  get_docid() = 0;     // Gets current docid
	virtual void   next() = 0;          // Moves to next docid
	virtual void   skip_to(docid) = 0;  // Moves to next docid >= specified docid
	virtual bool   at_end() = 0;        // True if iterator is off the end of the list
	virtual void   close() = 0;
};

class TermListIterator {
    private:
    public:
	virtual termid get_termid() = 0;    // Gets current termid
	virtual void   next() = 0;          // Moves to next termid
	virtual void   skip_to(termid) = 0; // Moves to next termid >= specified termid
	virtual bool   at_end() = 0;        // True if iterator is off the end of the list
	virtual void   close() = 0;
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
