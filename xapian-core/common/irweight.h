/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

class IRDatabase;
class RSet;

class IRWeight {
    private:
	const IRDatabase *database;
	doccount termfreq;
	const RSet * rset;

	bool initialised;

	mutable bool weight_calculated;
	mutable weight termweight;
	mutable doclength lenpart;
    public:
	IRWeight() : initialised(false), weight_calculated(false) { return; }
	void set_stats(const IRDatabase *db,
		       doccount tf,
		       const RSet *rset_new = NULL) {
	    database = db;
	    termfreq = tf;
	    rset = rset_new;
	    initialised = true;
	}
	void calc_termweight() const;
	weight get_weight(doccount wdf, doclength len) const;
	weight get_maxweight() const;
};

#endif /* _irweight_h_ */
