/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

class IRDatabase;

class IRWeight {
    private:
	const IRDatabase *database;
	doccount termfreq;
    public:
	IRWeight(const IRDatabase *db, doccount tf)
		: database(db), termfreq(tf)
	{}
	weight get_weight() const;
};

#endif /* _irweight_h_ */
