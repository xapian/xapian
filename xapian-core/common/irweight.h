/* irweight.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#ifndef _irweight_h_
#define _irweight_h_

#include "omtypes.h"
#include "omassert.h"

class IRDatabase;
class RSet;

// Abstract base class for weighting schemes
class IRWeight {
    protected:
	const IRDatabase *root;
	doccount termfreq;
	termname tname;
	const RSet * rset;

	bool initialised;
	mutable bool weight_calculated;

	virtual void calc_termweight() const = 0;
    public:
	IRWeight() : initialised(false), weight_calculated(false) { return; }
	virtual ~IRWeight() { return; };
	virtual void set_stats(const IRDatabase *,
			       doccount,
			       termname,
			       const RSet *);
	virtual weight get_weight(doccount wdf, doclength len) const = 0;
	virtual weight get_maxweight() const = 0;
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
IRWeight::set_stats(const IRDatabase *root_new,
		    doccount termfreq_new,
		    termname tname_new,
		    const RSet *rset_new = NULL) {
    // Can set stats several times, but can't set them after we've used them
    Assert(!weight_calculated);

    root = root_new;
    termfreq = termfreq_new;
    tname = tname_new;
    rset = rset_new;
    initialised = true;
}

#endif /* _irweight_h_ */
