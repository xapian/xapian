/* indextest.cc: test of the OpenMuscat indexing system
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

#include "config.h"
#include <iostream>
#include <string>
using std::cout;
using std::endl;

#include "om/om.h"
#include "om/omindexerbuilder.h"
#include "testsuite.h"

bool test_basic1()
{
    OmIndexerBuilder builder;

    builder.build_from_string("<?xml version=\"1.0\"?><omindexer/>");

    return true;
}

// ##################################################################
// # End of actual tests                                            #
// ##################################################################

/// The lists of tests to perform
test_desc tests[] = {
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
