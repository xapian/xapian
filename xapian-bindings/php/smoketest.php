<?php
/* Simple test to ensure that we can load the xapian module and exercise basic
 * functionality successfully.
 *
 * Copyright (C) 2004,2005,2006 Olly Betts
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

$php_version = substr(PHP_VERSION, 0, 1);

include "php$php_version/xapian.php";

# Include PHP version specific code.
include "smoketest$php_version.php";

$stem = new XapianStem("english");
if ($stem->get_description() != "Xapian::Stem(english)") {
    print "Unexpected \$stem->get_description()\n";
    exit(1);
}

$doc = new XapianDocument();
$doc->set_data("a\x00b");
if ($doc->get_data() === "a") {
    print "get_data+set_data truncates at a zero byte\n";
    exit(1);
}
if ($doc->get_data() !== "a\x00b") {
    print "get_data+set_data doesn't transparently handle a zero byte\n";
    exit(1);
}
$doc->set_data("is there anybody out there?");
$doc->add_term("XYzzy");
$doc->add_posting($stem->apply("is"), 1);
$doc->add_posting($stem->apply("there"), 2);
$doc->add_posting($stem->apply("anybody"), 3);
$doc->add_posting($stem->apply("out"), 4);
$doc->add_posting($stem->apply("there"), 5);

// Check virtual function dispatch.
if (substr($db->get_description(), 0, 17) !== "WritableDatabase(") {
    print "Unexpected \$db->get_description()\n";
    exit(1);
}
$db->add_document($doc);
if ($db->get_doccount() != 1) {
    print "Unexpected \$db->get_doccount()\n";
    exit(1);
}

$terms = array("smoke", "test", "terms");
$query = new XapianQuery($op_or, $terms);
if ($query->get_description() != "Xapian::Query((smoke OR test OR terms))") {
    print "Unexpected \$query->get_description()\n";
    exit(1);
}
$query1 = new XapianQuery($op_phrase, array("smoke", "test", "tuple"));
if ($query1->get_description() != "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))") {
    print "Unexpected \$query1->get_description()\n";
    exit(1);
}
$query2 = new XapianQuery($op_xor, array(new XapianQuery("smoke"), $query1, "string"));
if ($query2->get_description() != "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))") {
    print "Unexpected \$query2->get_description()\n";
    exit(1);
}
$subqs = array("a", "b");
$query3 = new XapianQuery($op_or, $subqs);
if ($query3->get_description() != "Xapian::Query((a OR b))") {
    print "Unexpected \$query3->get_description()\n";
    exit(1);
}
$enq = new XapianEnquire($db);
$enq->set_query(new XapianQuery($op_or, "there", "is"));
$mset = $enq->get_mset(0, 10);
if ($mset->size() != 1) {
    print "Unexpected \$mset->size()\n";
    exit(1);
}
$terms = join(" ", $enq->get_matching_terms($mset->get_hit(0)));
if ($terms != "is there") {
    print "Unexpected matching terms: $terms\n";
    exit(1);
}

if ($op_elite_set != 10) {
    print "OP_ELITE_SET is $op_elite_set not 10\n";
    exit(1);
}

# Regression test - overload resolution involving boolean types failed.
$enq->set_sort_by_value(1, TRUE);

# Regression test - fixed in 0.9.10.1.
$oqparser = new XapianQueryParser();
$oquery = $oqparser->parse_query("I like tea");

# Regression test for bug#192 - fixed in 1.0.3.
$enq->set_cutoff(100);

?>
