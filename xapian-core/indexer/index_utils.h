/* index_utils.h
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

#ifndef OM_HGUARD_INDEX_UTILS_H
#define OM_HGUARD_INDEX_UTILS_H

#include "om/omtypes.h"
void lowercase_term(om_termname &);
void select_characters(om_termname &term, const std::string & keep);
void get_paragraph(std::istream &input, std::string &para);
void get_a_line(std::istream &input, std::string &line);

#endif /* OM_HGUARD_INDEX_UTILS_H */
