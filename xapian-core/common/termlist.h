/* termlist.h
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#ifndef OM_HGUARD_TERMLIST_H
#define OM_HGUARD_TERMLIST_H

#include "om/omtypes.h"
#include "om/omerror.h"
#include "omrefcnt.h"
#include "expandweight.h"

/** Abstract base class for termlists. */
class TermList : public OmRefCntBase
{
    private:
	/// Copying is not allowed.
	TermList(const TermList &);

	/// Assignment is not allowed.
	void operator=(const TermList &);
    public:
	/// Standard constructor for base class.
	TermList() {}

	/// Standard destructor for base class.
	virtual ~TermList() { return; }

	// Gets size of termlist
	virtual om_termcount get_approx_size() const = 0;

	// Gets weighting info for current term
	virtual OmExpandBits get_weighting() const = 0;

	// Gets current termname
	virtual const om_termname get_termname() const = 0;

	// Get wdf of current term
	virtual om_termcount get_wdf() const = 0;

	// Get num of docs indexed by term
	virtual om_doccount get_termfreq() const = 0;

	/** next() causes the TermList to move to the next term in the list.
	 *  It must be called before any other methods.
	 *  If next() returns a non-zero pointer P, then the original
	 *  termlist should be deleted, and the original pointer replaced
	 *  with P.
	 *  In LeafTermList, next() will always return 0.
	 */
	virtual TermList * next() = 0;

	// True if we're off the end of the list
	virtual bool   at_end() const = 0;
};

/** Base class for termlists which are at the leaves of the termlist tree,
 *  and thus generate weighting information, rather than merging weighting
 *  information from sub termlists.
 */
class LeafTermList : public TermList
{
    protected:
	const OmExpandWeight * wt;
    public:
	LeafTermList() : wt(NULL) { return; }
	~LeafTermList() { return; }

	// Sets term weight
	virtual void set_weighting(const OmExpandWeight * wt_) { wt = wt_; }
};

#endif /* OM_HGUARD_TERMLIST_H */
