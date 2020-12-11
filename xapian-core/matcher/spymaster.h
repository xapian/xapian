/** @file
 * @brief Class for managing MatchSpy objects during the match
 */
/* Copyright 2017 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_SPYMASTER_H
#define XAPIAN_INCLUDED_SPYMASTER_H

#include <xapian/intrusive_ptr.h>
#include <xapian/matchspy.h>

#include <vector>

class SpyMaster {
    typedef Xapian::Internal::opt_intrusive_ptr<Xapian::MatchSpy> opt_ptr_spy;

    /// The MatchSpy objects to apply.
    const std::vector<opt_ptr_spy>* spies;

  public:
    explicit SpyMaster(const std::vector<opt_ptr_spy>* spies_)
	: spies(spies_->empty() ? NULL : spies_)
    {}

    operator bool() const { return spies != NULL; }

    void operator()(const Xapian::Document& doc,
		    double weight) {
	if (spies != NULL) {
	    for (auto spy : *spies) {
		(*spy)(doc, weight);
	    }
	}
    }
};

#endif // XAPIAN_INCLUDED_SPYMASTER_H
