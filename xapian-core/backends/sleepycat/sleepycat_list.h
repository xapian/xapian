/* sleepy_list.h: class definition for sleepycat list access routines
 *
 * ----START-LICENCE----
 * -----END-LICENCE-----
 */

#ifndef _sleepy_list_h_
#define _sleepy_list_h_

#include "omassert.h"
#include <db_cxx.h>

#include "omtypes.h"

typedef unsigned int entry_type;   // Able to represent any entry

class SleepyList {
    private:
	Db *db;
	Dbt key;

	bool opened;
	bool modified;

	entry_type length;  // How many ids
	entry_type *ids;    // List of ids
	entry_type *wdfs;   // List of wdfs, 1 for each id
	entry_type *freqs;  // How many things are indexed by this item
	termpos *positions;

	entry_type readentry(char **, char *); // Read entry
    public:
	SleepyList(Db *);
	~SleepyList();

	void open(void *, size_t);
	void add(entry_type id, entry_type wdf);
	void close();
};

#endif /* _sleepy_list_h_ */
