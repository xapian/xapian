/* database.h
 */

#include <map>
#include <list>
#include <string>
#include <vector>

typedef unsigned int termid;
typedef unsigned int docid;
typedef char *termname;

class RangeError {
    private:
	string msg;
    public:
        RangeError(string error_msg)
        {
	    msg = error_msg;
        }
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

	list<PostListIterator*> postlist;
	list<TermListIterator*> termlist;
};

class DADatabase : public IRDatabase {
    private:
	termid term_name_to_id(termname);
	termname term_id_to_name(termid);

        termid max_termid;
        map<termname, termid> termidmap;
        vector<termname> termidvec;
    public:
        DADatabase();
};

class ProtoDatabase : public IRDatabase {
    private:
    public:
	ProtoDatabase();
}
