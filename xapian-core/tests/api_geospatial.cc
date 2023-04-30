/** @file
 * @brief Tests of geospatial functionality.
 */
/* Copyright 2008 Lemur Consulting Ltd
 * Copyright 2010,2011 Richard Boulton
 * Copyright 2012,2016 Olly Betts
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
#include "api_geospatial.h"
#include <xapian.h>

#include "apitest.h"
#include "testsuite.h"
#include "testutils.h"

using namespace std;
using namespace Xapian;

// #######################################################################
// # Tests start here

static void
builddb_coords1(Xapian::WritableDatabase &db, const string &)
{
    Xapian::LatLongCoord coord1(10, 10);
    Xapian::LatLongCoord coord2(20, 10);
    Xapian::LatLongCoord coord3(30, 10);

    Xapian::Document doc;
    doc.add_value(0, coord1.serialise());
    db.add_document(doc);

    doc = Xapian::Document();
    doc.add_value(0, coord2.serialise());
    db.add_document(doc);

    doc = Xapian::Document();
    doc.add_value(0, coord3.serialise());
    db.add_document(doc);
}

/// Test behaviour of the LatLongDistancePostingSource
DEFINE_TESTCASE(latlongpostingsource1, backend && !remote && !inmemory) {
    Xapian::Database db = get_database("coords1", builddb_coords1, "");
    Xapian::LatLongCoord coord1(10, 10);
    Xapian::LatLongCoord coord2(20, 10);
    Xapian::LatLongCoord coord3(30, 10);

    Xapian::GreatCircleMetric metric;
    Xapian::LatLongCoords centre;
    centre.append(coord1);
    double coorddist = metric(coord1, coord2);
    TEST_EQUAL_DOUBLE(coorddist, metric(coord2, coord3));

    // Test a search with no range restriction.
    {
	Xapian::LatLongDistancePostingSource ps(0, coord1, metric);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);
	TEST_EQUAL(ps.get_docid(), 1);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist * 2));
	TEST_EQUAL(ps.get_docid(), 3);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with no range restriction and implicit metric.
    {
	Xapian::LatLongDistancePostingSource ps(0, coord1);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);
	TEST_EQUAL(ps.get_docid(), 1);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist * 2));
	TEST_EQUAL(ps.get_docid(), 3);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a tight range restriction
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, metric, coorddist * 0.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a tight range restriction and implicit metric.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, coorddist * 0.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a looser range restriction
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, metric, coorddist);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a looser range restriction and implicit metric.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, coorddist);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a looser range restriction, but not enough to return
    // the next document.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, metric, coorddist * 1.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a looser range restriction, but not enough to return
    // the next document and implicit metric.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, coorddist * 1.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a loose enough range restriction that all docs should
    // be returned.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, metric, coorddist * 2.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist * 2));
	TEST_EQUAL(ps.get_docid(), 3);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }

    // Test a search with a loose enough range restriction that all docs should
    // be returned and implicit metric.
    {
	Xapian::LatLongDistancePostingSource ps(0, centre, coorddist * 2.5);
	ps.init(db);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1.0);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist));
	TEST_EQUAL(ps.get_docid(), 2);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), false);
	TEST_EQUAL_DOUBLE(ps.get_weight(), 1000.0 / (1000.0 + coorddist * 2));
	TEST_EQUAL(ps.get_docid(), 3);

	ps.next(0.0);
	TEST_EQUAL(ps.at_end(), true);
    }
}

// Test various methods of LatLongCoord and LatLongCoords
DEFINE_TESTCASE(latlongcoords1, !backend) {
    LatLongCoord c1(0, 0);
    LatLongCoord c2(1, 0);
    LatLongCoord c3(1, 0);
    LatLongCoord c4(0, 1);

    // Test comparison
    TEST_NOT_EQUAL(c1.get_description(), c2.get_description());
    // Exactly one of these inequalities should be true.
    TEST((c1 < c2) ^ (c2 < c1));
    TEST_EQUAL(c2.get_description(), c3.get_description());
    TEST(!(c2 < c3) && !(c3 < c2));
    TEST_NOT_EQUAL(c3.get_description(), c4.get_description());
    // Exactly one of these inequalities should be true.  This is a regression
    // test for bug found prior to 1.3.0.
    TEST((c3 < c4) ^ (c4 < c3));

    // Test serialisation
    std::string s1 = c1.serialise();
    LatLongCoord c5;
    c4.unserialise(s1);
    TEST(!(c1 < c4 || c4 < c1));
    const char * ptr = s1.data();
    const char * end = ptr + s1.size();
    c5.unserialise(&ptr, end);
    TEST_EQUAL(c1.get_description(), c4.get_description());
    TEST_EQUAL(c1.get_description(), "Xapian::LatLongCoord(0, 0)");
    TEST_EQUAL(ptr, end);

    // Test uninitialised iterator constructor
    LatLongCoordsIterator i1;

    // Test building a set of LatLongCoords
    LatLongCoords g1(c1);
    TEST(!g1.empty());
    TEST_EQUAL(g1.size(), 1);
    TEST_EQUAL(g1.get_description(), "Xapian::LatLongCoords((0, 0))");
    g1.append(c2);
    TEST_EQUAL(g1.size(), 2);
    TEST_EQUAL(g1.get_description(), "Xapian::LatLongCoords((0, 0), (1, 0))");

    // Test iterating through a set of LatLongCoords
    i1 = g1.begin();
    TEST(i1 != g1.end());
    TEST_EQUAL((*i1).serialise(), c1.serialise());
    TEST_EQUAL((*i1).serialise(), c1.serialise());
    ++i1;
    TEST(i1 != g1.end());
    TEST_EQUAL((*i1).serialise(), c2.serialise());
    i1 = g1.begin();
    ++i1;
    TEST_EQUAL((*i1).serialise(), c2.serialise());
    TEST(i1 != g1.end());
    ++i1;
    TEST(i1 == g1.end());

    // Test that duplicates are allowed in the list of coordinates, now.
    g1.append(c3);
    TEST_EQUAL(g1.size(), 3);
    TEST_EQUAL(g1.get_description(), "Xapian::LatLongCoords((0, 0), (1, 0), (1, 0))");

    // Test building an empty LatLongCoords
    LatLongCoords g2;
    TEST(g2.empty());
    TEST_EQUAL(g2.size(), 0);
    TEST_EQUAL(g2.get_description(), "Xapian::LatLongCoords()");
    TEST(g2.begin() == g2.end());
}

// Test various methods of LatLongMetric
DEFINE_TESTCASE(latlongmetric1, !backend) {
    LatLongCoord c1(0, 0);
    LatLongCoord c2(1, 0);
    Xapian::GreatCircleMetric m1;
    double d1 = m1(c1, c2);
    TEST_REL(d1, >, 111226.0);
    TEST_REL(d1, <, 111227.0);

    // Let's make another metric, this time using the radius of mars, so
    // distances should be quite a bit smaller.
    Xapian::GreatCircleMetric m2(3310000);
    double d2 = m2(c1, c2);
    TEST_REL(d2, >, 57770.0);
    TEST_REL(d2, <, 57771.0);

    // Check serialise and unserialise.
    Xapian::Registry registry;
    std::string s1 = m2.serialise();
    const Xapian::LatLongMetric * m3;
    m3 = registry.get_lat_long_metric(m2.name());
    TEST(m3 != NULL);
    m3 = m3->unserialise(s1);
    double d3 = (*m3)(c1, c2);
    TEST_EQUAL_DOUBLE(d2, d3);

    delete m3;
}

// Test LatLongMetric on lists of coords.
DEFINE_TESTCASE(latlongmetric2, !backend) {
    LatLongCoord c1(0, 0);
    LatLongCoord c2(1, 0);
    LatLongCoords cl1(c1);
    LatLongCoords cl2(c2);
    string c2_str = c2.serialise();
    string cl2_str = cl2.serialise();
    TEST_EQUAL(c2_str, cl2_str);

    LatLongCoord c2_check(5, 5);
    c2_check.unserialise(c2_str);
    TEST_EQUAL(c2_check.latitude, c2.latitude);
    TEST_EQUAL(c2_check.longitude, c2.longitude);

    Xapian::GreatCircleMetric m1;
    double d1 = m1(c1, c2);
    double dl1 = m1(cl1, cl2);
    TEST_EQUAL(d1, dl1);
    double d1_str = m1(cl1, c2_str);
    TEST_EQUAL(d1, d1_str);
}

// Test a LatLongDistanceKeyMaker directly.
DEFINE_TESTCASE(latlongkeymaker1, !backend) {
    Xapian::GreatCircleMetric m1(3310000);
    LatLongCoord c1(0, 0);
    LatLongCoord c2(1, 0);
    LatLongCoord c3(2, 0);
    LatLongCoord c4(3, 0);

    LatLongCoords g1(c1);
    g1.append(c2);

    LatLongDistanceKeyMaker keymaker(0, g1, m1);
    Xapian::Document doc1;
    doc1.add_value(0, g1.serialise());
    Xapian::Document doc2;
    doc2.add_value(0, c3.serialise());
    Xapian::Document doc3;
    doc3.add_value(0, c4.serialise());
    Xapian::Document doc4;

    std::string k1 = keymaker(doc1);
    std::string k2 = keymaker(doc2);
    std::string k3 = keymaker(doc3);
    std::string k4 = keymaker(doc4);
    TEST_REL(k1, <, k2);
    TEST_REL(k2, <, k3);
    TEST_REL(k3, <, k4);

    LatLongDistanceKeyMaker keymaker2(0, g1, m1, 0);
    std::string k3b = keymaker2(doc3);
    std::string k4b = keymaker2(doc4);
    TEST_EQUAL(k3, k3b);
    TEST_REL(k3b, >, k4b);
}
