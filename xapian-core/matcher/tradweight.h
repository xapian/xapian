/* tradweight.h
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

#ifndef OM_HGUARD_TRADWEIGHT_H
#define OM_HGUARD_TRADWEIGHT_H

#include "irweight.h"
#include "om/omtypes.h"

// Traditional weighting scheme
class TradWeight : public IRWeight {
    private:
	mutable om_weight termweight;
	mutable om_doclength lenpart;

	double param_k;
	
	void calc_termweight() const;
	TradWeight(om_weight termweight_,
		   om_doclength lenpart_,
		   double param_k_)
		: termweight(termweight_), lenpart(lenpart_), param_k(param_k_)
		{}
    public:
	IRWeight * create(const OmSettings &opts) const	{
	    return new TradWeight(opts);
	}
	TradWeight(const OmSettings & opts);
	~TradWeight() { }
	om_weight get_sumpart(om_termcount wdf, om_doclength len) const;
	om_weight get_maxpart() const;

	om_weight get_sumextra(om_doclength len) const;
	om_weight get_maxextra() const;

	bool get_sumpart_needs_doclength() const { return (lenpart != 0); }
};

#endif /* OM_HGUARD_TRADWEIGHT_H */
