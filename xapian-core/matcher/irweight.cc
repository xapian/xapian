/* irweight.cc: How to create irweight objects
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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

#include <config.h>
#include "irweight.h"

// Include headers for all built-in weight types
#include "tradweight.h"
#include "bm25weight.h"
#include "boolweight.h"

std::map<std::string, const IRWeight *> IRWeight::custom_weights;

IRWeight *
IRWeight::create_new(const OmSettings & opts)
{
    DEBUGCALL_STATIC(MATCH, IRWeight *, "IRWeight::create_new", opts);
    IRWeight * weight = NULL;

    const std::string wt_type = opts.get("match_weighting_scheme", "bm25");

    if (wt_type.empty() || wt_type.at(0) != 'x') {
	// Create weight of correct type
	if (wt_type == "bm25") {
	    weight = new BM25Weight(opts);
	} else if (wt_type == "trad") {
	    weight = new TradWeight(opts);	
	} else if (wt_type == "bool") {
	    weight = new BoolWeight();
	} else {
	    throw OmInvalidArgumentError("Unknown weighting scheme");
	}	
    } else {
	// handle custom weights
	std::map<std::string, const IRWeight *>::iterator i;
	i = custom_weights.find(wt_type);
	if (i == custom_weights.end()) {
	    // FIXME: is this really the right error?
	    throw OmOpeningError("Unknown custom weighting scheme `" +
				 wt_type + "'");
	}
	weight = i->second->create(opts);
    }

    // Check that we have a weighting object
    Assert(weight != NULL);

    RETURN(weight);
}

void
IRWeight::register_custom(const std::string &wt_type, const IRWeight *wt)
{
    DEBUGCALL_STATIC(MATCH, void, "IRWeight::register_custom", wt_type << ", " << wt);
    Assert(wt);
    if (wt_type.empty() || wt_type[0] != 'x') {
	throw OmInvalidArgumentError("Custom weighting scheme names must start with `x'");
    }

    custom_weights.insert(std::make_pair(wt_type, wt));
}
