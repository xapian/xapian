/* da_database.h: C++ class definition for DA access routines */

#ifndef _da_record_h_
#define _da_record_h_

#include "da_database.h"
#include "daread.h"

class DARecord {
    friend class DADatabase;
    private:
	struct record * rec;
	char *pos;
	DARecord(struct record *);
    public:
	~DARecord();
};

#endif /* _da_record_h_ */
