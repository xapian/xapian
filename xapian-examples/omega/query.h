/* query.h: query functions for omega
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

#ifndef QUERY_H
#define QUERY_H

#include <list>
#include <map>
#include <string>

// Heuristics:
// * If any terms have been removed, it's a "fresh query" so we discard any
//   relevance judgements
// * If all previous terms are there but more have been added then we keep
//   the relevance judgements, but return the first page of hits
//
// NEW_QUERY entirely new query
// SAME_QUERY unchanged query
// EXTENDED_QUERY new query, but based on the old one
typedef enum { NEW_QUERY, SAME_QUERY, EXTENDED_QUERY } querytype;

string html_escape(const string &str);

querytype set_probabilistic(const string&, const string&);
om_doccount do_match();

void add_bterm(const string &);

extern void do_picker(char prefix, const char **opts);

extern om_queryop op;

extern string raw_prob;
extern map<om_docid, bool> ticked;
extern string query_string;
extern map<char, string> filter_map;
extern char *fmtstr;
extern list<om_termname> new_terms_list;

typedef enum { NORMAL, PLUS, MINUS /*, BOOL_FILTER*/ } termtype;

extern void check_term(const string &name, termtype type);

#endif /* QUERY_H */
