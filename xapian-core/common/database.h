/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <string>
#include "omtypes.h"

#include "database_builder.h"

class IRDocument;
class PostList;
class DBPostList;
class TermList;
class DBTermList;
class RSet;

class IRDatabase {
    // Class which can create IRDatabases.
    // All classes derived from IRDatabase must also have DatabaseBuilder as
    // a friend, so that they can be constructed in a unified way.
    friend class DatabaseBuilder;
    private:
    	// Open method - called only by DatabaseBuilder
	virtual void open(const DatabaseBuilderParams &) = 0;

	// Note: No close method needed - just delete the object
    protected:
    	// Constructor - called only by derived classes and by DatabaseBuilder
	IRDatabase() : root(this) { return; }

	// Root database - used to calculate collection statistics
	IRDatabase * root;
    public:
        virtual ~IRDatabase() { return; }

	void set_root(IRDatabase *db) {root = db;}

	// Number of docs in the database
	virtual doccount  get_doccount() const = 0;
	// Average length of a document
	virtual doclength get_avlength() const = 0;

	// Number of docs indexed by a given term
	virtual doccount get_termfreq(const termname &) const = 0;

	// Whether a given term is in the database
	virtual bool term_exists(const termname &) const = 0;

	// Throws RangeError if term not in database
	virtual DBPostList * open_post_list(const termname&, RSet *) const = 0;

	// Throws RangeError if docid invalid
	virtual TermList * open_term_list(docid) const = 0;

	// Throws RangeError if docid invalid
	virtual IRDocument * open_document(docid id) const = 0;

	// Introspection methods
	virtual const string get_database_path() const = 0;
};

#endif /* _database_h_ */
