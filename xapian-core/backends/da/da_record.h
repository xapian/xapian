/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_record_h_
#define _da_record_h_

#include "irdocument.h"
#include "daread.h"

class DADocument : public virtual IRDocument {
    friend class DADatabase;
    private:
	struct record * rec;
	DADocument(struct record *);

	// Stop copying
	DADocument(const DADocument &);
	DADocument & operator = (const DADocument &);
    public:
	~DADocument();

	IRKey get_key(keyno) const;
	IRData get_data() const;
};

#endif /* _da_record_h_ */
