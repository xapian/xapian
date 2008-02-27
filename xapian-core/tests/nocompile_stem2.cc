/** @file nocompile_stem2.cc
 *  @brief Check that it's not possible to instantiate an instance of a
 *  StemSnowball subclass on the stack.
 */
/* Copyright 2008 Lemur Consulting Ltd
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

#include <config.h>
#include <xapian.h>

class StemSnowballSubclass : public Xapian::StemSnowball {
    protected:
	// All user derived subclasses have to make their destructor
	// restricted, so that the class can't be instantiated on the stack.
	~StemSnowballSubclass() {}
    public:
	StemSnowballSubclass(const std::string & language)
		: Xapian::StemSnowball(language) {}

	std::string operator()(const std::string &word) const { return "!2" + Xapian::StemSnowball::operator()(word); }
	std::string get_description() const { return "StemSubclass"; }
};

int main()
{
    StemSnowballSubclass stem("en");
}
