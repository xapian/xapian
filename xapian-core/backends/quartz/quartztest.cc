/* quartztest.cc: test of the Quartz Database
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

#include "testsuite.h"
#include "testutils.h"
#include "om/omerror.h"

#include "quartz_database.h"
#include "quartz_db_table.h"
/// Test making and playing with a quartz_db_table
static bool test_dbtable1()
{
    QuartzDbTable table1(true);
    QuartzDbTable table2(false);

    QuartzRevisionNumber initialrev1 = table1.get_revision_number();
    QuartzRevisionNumber initialrev2 = table2.get_revision_number();

    TEST_EQUAL(initialrev1, table1.get_revision_number());
    TEST_EQUAL(initialrev2, table2.get_revision_number());

    std::map<QuartzDbKey, QuartzDbTag *> newentries;

    TEST_EXCEPTION(OmInvalidOperationError, table1.set_entries(newentries));
    table2.set_entries(newentries);

    TEST_EQUAL(initialrev1, table1.get_revision_number());
    TEST_NOT_EQUAL(initialrev2, table2.get_revision_number());

    return true;
}

/// Test opening of a quartz database
static bool test_open1()
{
    OmSettings settings;
    settings.set("quartz_dir", "foo");

    QuartzDatabase database(settings, true);
    return true;
}

// ================================
// ========= END OF TESTS =========
// ================================

/// The lists of tests to perform
test_desc tests[] = {
    {"quartzdbtable1",		test_dbtable1},
    {"quartzopen1",		test_open1},
    {0, 0}
};

int main(int argc, char *argv[])
{
    return test_driver::main(argc, argv, tests);
}
