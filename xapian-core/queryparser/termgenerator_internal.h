/** @file
 * @brief TermGenerator class internals
 */
/* Copyright (C) 2007,2012,2016,2024 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
#define XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H

#include "xapian/intrusive_ptr.h"
#include <xapian/database.h>
#include <xapian/document.h>
#include <xapian/termgenerator.h>
#include <xapian/queryparser.h> // For Xapian::Stopper
#include <xapian/stem.h>

namespace Xapian {

class Stopper;

class TermGenerator::Internal : public Xapian::Internal::intrusive_base {
    friend class TermGenerator;
    Stem stemmer;
    stem_strategy strategy = STEM_SOME;
    Xapian::Internal::opt_intrusive_ptr<const Stopper> stopper;
    stop_strategy stop_mode = STOP_STEMMED;
    Document doc;
    termpos cur_pos = 0;
    termpos pos_limit = termpos(-1);
    TermGenerator::flags flags = 0;
    unsigned max_word_length = 64;
    WritableDatabase db;

  public:
    Internal() { }

    void index_text(Utf8Iterator itor,
		    termcount weight,
		    std::string_view prefix,
		    bool with_positions);
};

}

#endif // XAPIAN_INCLUDED_TERMGENERATOR_INTERNAL_H
