/* omparsequery.h: parser for omega query strings
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef OM_HGUARD_PARSEQUERY_H
#define OM_HGUARD_PARSEQUERY_H

#include "om/omquery.h"

#include <string>

class OmQueryParser {
    public:
	OmQueryParser(const std::string lang_ = "english",
		      bool stem_all_ = false) {
	    set_stemming_options(lang_, stem_all_);
	}
	void set_stemming_options(const std::string &lang_,
				  bool stem_all_ = false);
	OmQuery parse_query(const std::string &raw_query);
};

#endif /* OM_HGUARD_PARSEQUERY_H */
