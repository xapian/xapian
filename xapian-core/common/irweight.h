/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

class IRDatabase;

class IRWeight {
    private:
	const IRDatabase *database;
	doccount termfreq;

	bool initialised;

	mutable bool weight_calculated;
	mutable weight termweight;
    public:
	IRWeight() : initialised(false) {}
	void set_stats(const IRDatabase *db, doccount tf) {
	    database = db;
	    termfreq = tf;
	    initialised = true;
	}
	void calc_termweight() const;
	weight get_weight(doccount wdf) const;
	weight get_maxweight() const;
};

#endif /* _irweight_h_ */
