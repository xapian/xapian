/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <string>

#include "omtypes.h"

class IRDocument;
class DBPostList;
class TermList;

class IRDatabase {
    protected:
	IRDatabase * root;
    public:
	IRDatabase() : root(this) { return; }
        virtual ~IRDatabase() { return; }

	void set_root(IRDatabase *db) {root = db;}

	virtual termid term_name_to_id(const termname &) const = 0;
	virtual termname term_id_to_name(termid) const = 0;
    
	virtual termid add_term(const termname &) = 0;
	virtual docid add_doc(IRDocument &) = 0;
	virtual void add(termid, docid, termpos) = 0;

        virtual void open(const string &pathname, bool readonly) = 0;
	virtual void close() = 0;

	// Number of docs in the database
	virtual doccount  get_doccount() const = 0;
	// Average length of a document
	virtual doclength get_avlength() const = 0;

	// Throws RangeError if termid invalid
	virtual DBPostList * open_post_list(termid) const = 0;

	// Throws RangeError if docid invalid
	virtual TermList * open_term_list(docid) const = 0;

	// Throws RangeError if docid invalid
	virtual IRDocument * open_document(docid id) const = 0;

	// Introspection methods
	virtual const string get_database_path() const = 0;
};

#endif /* _database_h_ */
