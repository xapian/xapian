/** @file letor_internal.h
 * @brief Internals of Xapian::Letor class
 */
/* Copyright (C) 2011 Parth Gupta
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

#ifndef XAPIAN_INCLUDED_LETOR_INTERNAL_H
#define XAPIAN_INCLUDED_LETOR_INTERNAL_H

#include <xapian/letor.h>

#include "ranker.h"
 
#include <string>

using namespace Xapian;
using std::string;

namespace Xapian {

class Letor::Internal : public Xapian::Internal::intrusive_base
{
    friend class Letor;

    Ranker & ranker;
    FeatureManager feature_manager;

public:
    void load_model_file(string model_file_);

    void update_mset(const Xapian::Query & query_, const Xapian::MSet & mset_);
};
    void train(string training_data_file_, string model_file_);

}

#endif // XAPIAN_INCLUDED_LETOR_INTERNAL_H
