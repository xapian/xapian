/* database.h
 */

#ifndef _database_h_
#define _database_h_

#include <map>
#include <list>
#include <string>
#include <vector>

#include "omassert.h"

#include "omtypes.h"
#include "error.h"
#include "irweight.h"

class PostList {
    private:
    public:
	virtual doccount get_termfreq() const = 0;// Gets number of docs indexed by this term

	virtual docid  get_docid() const = 0;     // Gets current docid
	virtual weight get_weight() const = 0;    // Gets current weight
        virtual weight get_maxweight() const = 0;    // Gets max weight

        virtual weight recalc_maxweight() = 0; // recalculate weights (used when tree has been autopruned)

	// w_min in the next two functions is simply a hint -
	// documents with a weight less than w_min will be ignored.
	// However, it may be best to return them anyway, if the weight
	// calculation is expensive, since many documents will be thrown
	// away anyway without calculating the weight.
	virtual PostList *next(weight w_min) = 0; // Moves to next docid
	virtual PostList *skip_to(docid, weight w_min);  // Moves to next docid >= specified docid
	virtual bool   at_end() const = 0;        // True if we're off the end of the list

        virtual ~PostList() { return; }
};

// naive implementation for database that can't do any better
inline PostList *
PostList::skip_to(docid id, weight w_min)
{
    Assert(!at_end());
    while (!at_end() && get_docid() < id) {
	PostList *ret = next(w_min);
	if (ret) return ret;
    }
    return NULL;
}

class DBPostList : public virtual PostList {
    protected:
	IRWeight * ir_wt;

	mutable bool weight_initialised;
	mutable weight termweight;
	void calc_termweight() const; // Calculates term weight
    public:
	DBPostList() :
		ir_wt(NULL), weight_initialised(false) {}
	~DBPostList() {
	    if(ir_wt) delete ir_wt;
	}
	void set_termweight(const IRWeight &); // Sets term weight
};

inline void
DBPostList::set_termweight(const IRWeight & wt)
{
    if(ir_wt) delete ir_wt;
    weight_initialised = false;
    ir_wt = new IRWeight(wt);
}

inline void
DBPostList::calc_termweight() const
{
    Assert(ir_wt != NULL);
    termweight = ir_wt->get_weight();
    weight_initialised = true;
}

class TermList {
    private:
    public:
	virtual termid get_termid() const = 0; // Gets current termid
	virtual termcount get_wdf() const = 0; // Get wdf of current term
	virtual doccount get_termfreq() const = 0; // Get num of docs indexed by term
	virtual TermList * next() = 0; // Moves to next termid
	virtual bool   at_end() const = 0; // True if we're off the end of the list

        virtual ~TermList() { return; }
};

// A document in the database
class IRDocument {
    private:
    public:
	// FIXME - holds keys and records
};

class IRDatabase {
    private:
    public:
        virtual ~IRDatabase() { return; }

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
};

#endif /* _database_h_ */
