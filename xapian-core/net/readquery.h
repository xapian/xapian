/* readquery.h: tests for the network matching code.
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

#ifndef OM_HGUARD_NETREADQUERY_H
#define OM_HGUARD_NETREADQUERY_H

#include "om/omtypes.h"
#include <string>
#include "netutils.h"

struct querytok {
    enum etype {
	END,
	NULL_QUERY,
	BOOL_FLAG,
	QUERY_LEN,
	OP_AND,
	OP_OR,
	OP_FILTER,
	OP_ANDMAYBE,
	OP_ANDNOT,
	OP_XOR,
	OP_NEAR,
	OP_PHRASE,
	OP_WEIGHT_CUTOFF,
	OP_PERCENT_CUTOFF,
	TERM,
	OP_BRA,
	OP_KET,
	ERROR
    } type;
    om_termcount qlen;
    om_termname tname;
    om_termcount wqf;
    om_termpos term_pos;
    om_termpos window; // for NEAR and PHRASE
    double cutoff; // for *_CUTOFF

    querytok(etype type_ = ERROR)
	    : type(type_) {}
    querytok(int type_)
	    : type(static_cast<etype>(type_)) {}
};

void qfs_start(std::string text);
querytok qfs_gettok();
void qfs_end();

#endif /* OM_HGUARD_NETREADQUERY_H */

