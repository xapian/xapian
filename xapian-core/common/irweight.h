/* irweight.h
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

#ifndef OM_HGUARD_IRWEIGHT_H
#define OM_HGUARD_IRWEIGHT_H

#include "om/omtypes.h"
#include "om/omsettings.h"
#include "omdebug.h"

class Database;
class RSet;
class StatsSource;

/// Abstract base class for weighting schemes
class IRWeight {
    private:
	IRWeight(const IRWeight &);
	void operator=(IRWeight &);

    protected:
	const StatsSource *stats;

	om_doclength querysize;
	om_termcount wqf;
	om_termname tname;

	bool initialised;
	mutable bool weight_calculated;

	static map<string, const IRWeight *> custom_weights;
    public:
	IRWeight() : initialised(false), weight_calculated(false) { return; }
	virtual ~IRWeight() { return; }

	static IRWeight *create(const string &wt_type, const OmSettings & opts);

	/// Register a custom weight object
	static void register_custom(const string &wt_type, const IRWeight *wt);

	/// Return a clone of this weight object.
	virtual IRWeight * clone() const = 0;

	/** Initialise the weight object with the neccessary stats, or
	 *  places to get them from.
	 *
	 *  @param stats_    Object to ask for collection statistics.
	 *  @param querysize_ Query size.
	 *  @param wqf_      Within query frequency of term this object is
	 *		     associated with.
	 *  @param tname_    Term which this object is associated with.
	 */
	virtual void set_stats(const StatsSource * stats_,
			       om_doclength querysize_, om_termcount wqf_,
			       om_termname tname_);

	/** Get a weight which is part of the sum over terms being performed.
	 *  This returns a weight for a given term and document.  These
	 *  weights are summed to give a total weight for the document.
	 *
	 *  @param wdf the within document frequency of the term.
	 *  @param len the (unnormalised) document length.
	 */
	virtual om_weight get_sumpart(om_termcount wdf,
				      om_doclength len) const = 0;

	/** Gets the maximum value that get_sumpart() may return.  This
	 *  is used in optimising searches, by having the postlist tree
	 *  decay appropriately when parts of it can have limited, or no,
	 *  further effect.
	 */
	virtual om_weight get_maxpart() const = 0;

	/** Get an extra weight for a document to add to the sum calculated
	 *  over the query terms.
	 *  This returns a weight for a given document, and is used by some
	 *  weighting schemes to account for influence such as document
	 *  length.
	 *
	 *  @param len the (unnormalised) document length.
	 */
	virtual om_weight get_sumextra(om_doclength len) const = 0;

	/** Gets the maximum value that get_sumextra() may return.  This
	 *  is used in optimising searches.
	 */
	virtual om_weight get_maxextra() const = 0;

	/// return false if the weight object doesn't need doclength
	virtual bool get_sumpart_needs_doclength() const { return true; }
};

///////////////////////////////
// Inline method definitions //
///////////////////////////////

inline void
IRWeight::set_stats(const StatsSource * stats_, om_doclength querysize_,
		    om_termcount wqf_, om_termname tname_) {
    // Can set stats several times, but can't set them after we've used them
    Assert(!weight_calculated);

    stats = stats_;
    querysize = querysize_;
    wqf = wqf_;
    tname = tname_;
    initialised = true;
}

#endif /* OM_HGUARD_IRWEIGHT_H */
