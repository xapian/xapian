/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <string>

#include "omtypes.h"

class IRDocument;
class PostList;
class DBPostList;
class TermList;
class DBTermList;
class RSet;

class IRDatabase {
    protected:
	IRDatabase * root;
    public:
	IRDatabase() : root(this) { return; }
        virtual ~IRDatabase() { return; }

	void set_root(IRDatabase *db) {root = db;}

	virtual bool term_exists(const termname &) const;
	virtual termid term_name_to_id(const termname &) const = 0;
	virtual termname term_id_to_name(termid) const = 0;
    
	// Close the database
	virtual void close() = 0;

	// Number of docs in the database
	virtual doccount  get_doccount() const = 0;
	// Average length of a document
	virtual doclength get_avlength() const = 0;

	// Throws RangeError if term not in database
	virtual DBPostList * open_post_list(const termname&, RSet *) const = 0;

	// Throws RangeError if docid invalid
	virtual TermList * open_term_list(docid) const = 0;

	// Throws RangeError if docid invalid
	virtual IRDocument * open_document(docid id) const = 0;

	// Introspection methods
	virtual const string get_database_path() const = 0;
};

class IRSingleDatabase : public virtual IRDatabase {
    public:
	virtual void open(const string &pathname, bool readonly) = 0;
};

class IRGroupDatabase : public virtual IRDatabase {
    public:
	virtual void open(om_database_type,
			  const string &pathname,
			  bool readonly) = 0;
};

inline bool
IRDatabase::term_exists(const termname &tname) const
{
    if(term_name_to_id(tname)) return true;
    return false;
}

#endif /* _database_h_ */
