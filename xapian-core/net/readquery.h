/* readquery.h: decode a serialised query
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
 * Copyright 2003 Olly Betts
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

#include <xapian/types.h>
#include <string>
#include "netutils.h"

using namespace std;

struct querytok {
    enum etype {
	END,
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
	OP_ELITE_SET,
	TERM,
	OP_BRA,
	ERROR
    } type;
    Xapian::termcount qlen;
    string tname;
    Xapian::termcount wqf;
    Xapian::termpos term_pos;
    Xapian::termpos window; // for NEAR and PHRASE
    Xapian::termcount elite_set_size;
    double cutoff; // for *_CUTOFF

    querytok(etype type_ = ERROR)
	    : type(type_) {}
    querytok(int type_)
	    : type(static_cast<etype>(type_)) {}
};

void qfs_start(string text);
querytok qfs_gettok();
void qfs_end();

#endif /* OM_HGUARD_NETREADQUERY_H */

