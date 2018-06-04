/** @file mathtermgenerator.cc
 * @brief MathTermGenerator class implementation
 */
/* Copyright (C) 2018 Guruprasad Hegde
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

#include <xapian/mathtermgenerator.h>
#include <xapian/types.h>
#include <xapian/unicode.h>

#include "mathtermgenerator_internal.h"

#include "str.h"

using namespace std;
using namespace Xapian;

MathTermGenerator::MathTermGenerator(const MathTermGenerator & o)
    : internal(o.internal) { }

MathTermGenerator &
MathTermGenerator::operator=(const MathTermGenerator & o)
{
    internal = o.internal;
    return *this;
}

MathTermGenerator::MathTermGenerator(MathTermGenerator &&) = default;

MathTermGenerator &
MathTermGenerator::operator=(MathTermGenerator &&) = default;

MathTermGenerator::MathTermGenerator()
    : internal(new MathTermGenerator::Internal) { }

MathTermGenerator::~MathTermGenerator() { }

void
MathTermGenerator::set_document(const Xapian::Document & doc)
{
    internal->doc = doc;
}

const Xapian::Document &
MathTermGenerator::get_document() const
{
    return internal->doc;
}

void
MathTermGenerator::index_math(const char * expr)
{
    internal->index_math(expr);

vector<string>
MathTermGenerator::get_symbol_pair_list(const char * expr)
{
    return internal->get_symbol_pair_list(expr);
}

vector<string>
MathTermGenerator::get_labels_list()
{
    return internal->get_labels_list();
}

string
MathTermGenerator::get_description() const
{
    string s("Xapian::MathTermGenerator()");
    return s;
}
