/* termlist.h
 */

#ifndef _termlist_h_
#define _termlist_h_

#include "omtypes.h"

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

#endif /* _termlist_h_ */
