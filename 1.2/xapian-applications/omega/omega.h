/* omega.h: Main header for omega
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Lemur Consulting Ltd
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2006,2007,2008 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OMEGA_INCLUDED_OMEGA_H
#define OMEGA_INCLUDED_OMEGA_H

#include <xapian.h>

#define PROGRAM_NAME "omega"

#include "configfile.h"

#include <map>

using namespace std;

const char filter_sep = '-';
// Any choice of character for filter_sep could conceivably lead to
// false positives, but the situation is contrived, and just means that if
// someone changed a filter, the first page wouldn't be forced.
// That's hardly the end of the world...

extern string query_string;

extern string dbname;
extern string fmtname;
extern string filters;

extern Xapian::Database db;
extern Xapian::Enquire * enquire;
extern Xapian::RSet rset;

extern Xapian::docid topdoc;
extern Xapian::docid hits_per_page;
extern Xapian::docid min_hits;

extern int threshold;

extern Xapian::valueno sort_key;
extern bool sort_ascending;
extern bool sort_after;
extern Xapian::Enquire::docid_order docid_order;

extern Xapian::valueno collapse_key;
extern bool collapse;

extern map<string, string> option;

extern string date_start, date_end, date_span;

extern const string default_dbname;

extern bool set_content_type, suppress_http_headers;

#endif // OMEGA_INCLUDED_OMEGA_H
