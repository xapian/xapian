/* boolweight.h : Boolean weighting scheme (everything gets 0)
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

#ifndef OM_HGUARD_BOOLWEIGHT_H
#define OM_HGUARD_BOOLWEIGHT_H

#include "irweight.h"
#include "om/omtypes.h"

// Boolean weighting scheme (everything gets 0)
class BoolWeight : public IRWeight {
    private:
    public:
	~BoolWeight() { }
	om_weight get_sumpart(om_termcount wdf, om_doclength len) const { return 0; }
	om_weight get_maxpart() const { return 0; }

	om_weight get_sumextra(om_doclength len) const { return 0; }
	om_weight get_maxextra() const { return 0; }
};

#endif /* OM_HGUARD_BOOLWEIGHT_H */
