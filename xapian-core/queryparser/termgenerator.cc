/** @file termgenerator.cc
 * @brief TermGenerator class implementation
 */
/* Copyright (C) 2007,2012 Olly Betts
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

#include <config.h>

#include <xapian/termgenerator.h>
#include <xapian/types.h>
#include <xapian/unicode.h>

#include "termgenerator_internal.h"

#include "str.h"

using namespace std;
using namespace Xapian;

TermGenerator::TermGenerator(const TermGenerator & o) : internal(o.internal) { }

TermGenerator &
TermGenerator::operator=(const TermGenerator & o) {
    internal = o.internal;
    return *this;
}

TermGenerator::TermGenerator() : internal(new TermGenerator::Internal) { }

TermGenerator::~TermGenerator() { }

void
TermGenerator::set_stemmer(const Xapian::Stem & stemmer)
{
    internal->stemmer = stemmer;
}

void
TermGenerator::set_stopper(const Xapian::Stopper * stopper)
{
    internal->stopper = stopper;
}

void
TermGenerator::set_document(const Xapian::Document & doc)
{
    internal->doc = doc;
    internal->termpos = 0;
}

const Xapian::Document &
TermGenerator::get_document() const
{
    return internal->doc;
}

void
TermGenerator::set_database(const Xapian::WritableDatabase &db)
{
    internal->db = db;
}

TermGenerator::flags
TermGenerator::set_flags(flags toggle, flags mask)
{
    TermGenerator::flags old_flags = internal->flags;
    internal->flags = flags((old_flags & mask) ^ toggle);
    return old_flags;
}

void
TermGenerator::set_stemming_strategy(stem_strategy strategy)
{
    internal->strategy = strategy;
}

void
TermGenerator::set_stopper_strategy(stop_strategy strategy)
{
    internal->stop_mode = strategy;
}

void
TermGenerator::set_max_word_length(unsigned max_word_length)
{
    internal->max_word_length = max_word_length;
}

void
TermGenerator::index_text(const Xapian::Utf8Iterator & itor,
			  Xapian::termcount weight,
			  const string & prefix)
{
    internal->index_text(itor, weight, prefix, true);
}

void
TermGenerator::index_text_without_positions(const Xapian::Utf8Iterator & itor,
					    Xapian::termcount weight,
					    const string & prefix)
{
    internal->index_text(itor, weight, prefix, false);
}

void
TermGenerator::increase_termpos(Xapian::termcount delta)
{
    internal->termpos += delta;
}

Xapian::termcount
TermGenerator::get_termpos() const
{
    return internal->termpos;
}

void
TermGenerator::set_termpos(Xapian::termcount termpos)
{
    internal->termpos = termpos;
}

string
TermGenerator::get_description() const
{
    string s("Xapian::TermGenerator(stem=");
    s += internal->stemmer.get_description();
    if (internal->stopper.get()) {
	s += ", stopper set";
    }
    s += ", doc=";
    s += internal->doc.get_description();
    s += ", termpos=";
    s += str(internal->termpos);
    s += ")";
    return s;
}
