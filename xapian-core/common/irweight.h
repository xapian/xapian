/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

class IRDatabase;

#include <vector>
class RSetItem {
    public:
	docid did;
};

class RSet {
    public:
	vector<RSetItem> documents;
};

class IRWeight {
    private:
	const IRDatabase *database;
	doccount termfreq;

	bool initialised;

	mutable bool weight_calculated;
	mutable weight termweight;
	mutable doclength lenpart;
    public:
	IRWeight() : initialised(false), weight_calculated(false) { return; }
	void set_stats(const IRDatabase *db, doccount tf, const RSet rset) {
	    database = db;
	    termfreq = tf;
	    initialised = true;
	}
	void set_stats(const IRDatabase *db, doccount tf) {
	    database = db;
	    termfreq = tf;
	    initialised = true;
	}
	void calc_termweight() const;
	weight get_weight(doccount wdf, doclength len) const;
	weight get_maxweight() const;
};

#endif /* _irweight_h_ */
