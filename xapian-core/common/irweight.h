/* irweight.h
 */

#ifndef _irweight_h_
#define _irweight_h_

class IRDatabase;

class IRWeight {
    private:
	weight termweight;
    public:
	IRWeight(const IRDatabase *, doccount termfreq);
	weight get_weight() {return termweight;}
};

#endif /* _irweight_h_ */
