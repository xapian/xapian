/* bm25weight.h
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

#ifndef _bm25weight_h_
#define _bm25weight_h_

#include "irweight.h"
#include "omtypes.h"
#include "omassert.h"

// BM25 weighting scheme
class BM25Weight : public virtual IRWeight {
    private:
	mutable om_weight termweight;
	mutable om_doclength lenpart;

	void calc_termweight() const;
    public:
	~BM25Weight() { }
	om_weight get_weight(om_doccount wdf, om_doclength len) const;
	om_weight get_maxweight() const;
};

#endif /* _bm25weight_h_ */
