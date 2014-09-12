<?php
// Run this PHP script using 'make check' in the build tree.

/* Simple test to ensure that we can load the xapian module and exercise basic
 * functionality successfully.
 *
 * Copyright (C) 2004,2005,2006,2007,2009,2011,2012,2013,2014 Olly Betts
 * Copyright (C) 2010 Richard Boulton
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

# Die on any error, warning, notice, etc.
function die_on_error($errno, $errstr, $file, $line) {
    if ($file !== Null) {
	print $file;
	if ($line !== Null) print ":$line";
	print ": ";
    }
    print "$errstr\n";
    exit(1);
}
set_error_handler("die_on_error", -1);

include "xapian.php";

# Test the version number reporting functions give plausible results.
$v = Xapian::major_version().'.'.Xapian::minor_version().'.'.Xapian::revision();
$v2 = Xapian::version_string();
if ($v != $v2) {
    print "Unexpected version output ($v != $v2)\n";
    exit(1);
}

$db = Xapian::inmemory_open();
$db2 = Xapian::inmemory_open();

# Check PHP5 handling of Xapian::DocNotFoundError
try {
    $doc2 = $db->get_document(2);
    print "Retrieved non-existent document\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DocNotFoundError: Docid 2 not found") {
	print "DocNotFoundError Exception string not as expected, got: '{$e->getMessage()}'\n";
	exit(1);
    }
}

# Check QueryParser parsing error.
try {
    $qp = new XapianQueryParser;
    $qp->set_stemmer(new XapianStem("en"));
    $qp->parse_query("test AND");
    print "Successfully parsed bad query\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "QueryParserError: Syntax: <expression> AND <expression>") {
	print "QueryParserError Exception string not as expected, got: '$e->getMessage()'\n";
	exit(1);
    }
}

# Check that open_stub() is wrapped as expected.
try {
    $db = Xapian::auto_open_stub("nosuchdir/nosuchdb");
    print "Opened non-existent stub database\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DatabaseOpeningError: Couldn't open stub database file: nosuchdir/nosuchdb (No such file or directory)") {
	print "DatabaseOpeningError Exception string not as expected, got: '{$e->getMessage()}'\n";
	exit(1);
    }
}

# Check that DB_BACKEND_STUB works as expected.
try {
    $db = new XapianDatabase("nosuchdir/nosuchdb", Xapian::DB_BACKEND_STUB);
    print "Opened non-existent stub database\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DatabaseOpeningError: Couldn't open stub database file: nosuchdir/nosuchdb (No such file or directory)") {
	print "DatabaseOpeningError Exception string not as expected, got: '{$e->getMessage()}'\n";
	exit(1);
    }
}

# Check that open_stub() writable form is wrapped as expected.
try {
    $db = Xapian::auto_open_stub("nosuchdir/nosuchdb", Xapian::DB_OPEN);
    print "Opened non-existent stub database\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DatabaseOpeningError: Couldn't open stub database file: nosuchdir/nosuchdb (No such file or directory)") {
	print "DatabaseOpeningError Exception string not as expected, got: '{$e->getMessage()}'\n";
	exit(1);
    }
}

# Check that DB_BACKEND_STUB works as expected.
try {
    $db = new XapianWritableDatabase("nosuchdir/nosuchdb",
				     Xapian::DB_OPEN|Xapian::DB_BACKEND_STUB);
    print "Opened non-existent stub database\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DatabaseOpeningError: Couldn't open stub database file: nosuchdir/nosuchdb (No such file or directory)") {
	print "DatabaseOpeningError Exception string not as expected, got: '{$e->getMessage()}'\n";
	exit(1);
    }
}

# Regression test for bug#193, fixed in 1.0.3.
$vrp = new XapianNumberValueRangeProcessor(0, '$', true);
$a = '$10';
$b = '20';
$vrp->apply($a, $b);
if (Xapian::sortable_unserialise($a) != 10) {
    print Xapian::sortable_unserialise($a)." != 10\n";
    exit(1);
}
if (Xapian::sortable_unserialise($b) != 20) {
    print Xapian::sortable_unserialise($b)." != 20\n";
    exit(1);
}

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
$query = new XapianQuery(XapianQuery::OP_OR, $terms);
if ($query->get_description() != "Query((smoke OR test OR terms))") {
    print "Unexpected \$query->get_description()\n";
    exit(1);
}
$query1 = new XapianQuery(XapianQuery::OP_PHRASE, array("smoke", "test", "tuple"));
if ($query1->get_description() != "Query((smoke PHRASE 3 test PHRASE 3 tuple))") {
    print "Unexpected \$query1->get_description()\n";
    exit(1);
}
$query1b = new XapianQuery(XapianQuery::OP_NEAR, array("smoke", "test", "tuple"), 4);
if ($query1b->get_description() != "Query((smoke NEAR 4 test NEAR 4 tuple))") {
    print "Unexpected \$query1b->get_description()\n";
    exit(1);
}
$query2 = new XapianQuery(XapianQuery::OP_XOR, array(new XapianQuery("smoke"), $query1, "string"));
if ($query2->get_description() != "Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))") {
    print "Unexpected \$query2->get_description()\n";
    exit(1);
}
$subqs = array("a", "b");
$query3 = new XapianQuery(XapianQuery::OP_OR, $subqs);
if ($query3->get_description() != "Query((a OR b))") {
    print "Unexpected \$query3->get_description()\n";
    exit(1);
}
$enq = new XapianEnquire($db);
$enq->set_query(new XapianQuery(XapianQuery::OP_OR, "there", "is"));
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

# Feature test for MatchDecider
$doc = new XapianDocument();
$doc->set_data("Two");
$doc->add_posting($stem->apply("out"), 1);
$doc->add_posting($stem->apply("outside"), 1);
$doc->add_posting($stem->apply("source"), 2);
$doc->add_value(0, "yes");
$db->add_document($doc);

class testmatchdecider extends XapianMatchDecider {
    function apply($doc) {
	return ($doc->get_value(0) == "yes");
    }
}

if (defined('PHP_VERSION_ID') && PHP_VERSION_ID >= 50400) {
    print "Skipping known failure subclassing Xapian classes in PHP under PHP 5.4+\n";
} else {
    $query = new XapianQuery($stem->apply("out"));
    $enquire = new XapianEnquire($db);
    $enquire->set_query($query);
    $mdecider = new testmatchdecider();
    $mset = $enquire->get_mset(0, 10, null, $mdecider);
    if ($mset->size() != 1) {
	print "Unexpected number of documents returned by match decider (".$mset->size().")\n";
	exit(1);
    }
    if ($mset->get_docid(0) != 2) {
	print "MatchDecider mset has wrong docid in\n";
	exit(1);
    }
}

class testexpanddecider extends XapianExpandDecider {
    function apply($term) {
	return ($term[0] !== 'a');
    }
}

$enquire = new XapianEnquire($db);
$rset = new XapianRSet();
$rset->add_document(1);
$eset = $enquire->get_eset(10, $rset, XapianEnquire::USE_EXACT_TERMFREQ, 1.0, new testexpanddecider());
foreach ($eset->begin() as $t) {
    if ($t[0] === 'a') {
	print "XapianExpandDecider was not used\n";
	exit(1);
    }
}

# Check min_wt argument to get_eset() works (new in 1.2.5).
$eset = $enquire->get_eset(100, $rset, XapianEnquire::USE_EXACT_TERMFREQ);
$min_wt = 0;
foreach ($eset->begin() as $i => $dummy) {
    $min_wt = $i->get_weight();
}
if ($min_wt >= 1.9) {
    print "ESet min weight too high for testcase\n";
    exit(1);
}
$eset = $enquire->get_eset(100, $rset, XapianEnquire::USE_EXACT_TERMFREQ, 1.0, NULL, 1.9);
$min_wt = 0;
foreach ($eset->begin() as $i => $dummy) {
    $min_wt = $i->get_weight();
}
if ($min_wt < 1.9) {
    print "ESet min_wt threshold not applied\n";
    exit(1);
}

if (XapianQuery::OP_ELITE_SET != 10) {
    print "OP_ELITE_SET is XapianQuery::OP_ELITE_SET not 10\n";
    exit(1);
}

# Regression test - overload resolution involving boolean types failed.
$enq->set_sort_by_value(1, TRUE);

# Regression test - fixed in 0.9.10.1.
$oqparser = new XapianQueryParser();
$oquery = $oqparser->parse_query("I like tea");

# Regression test for bug#192 - fixed in 1.0.3.
$enq->set_cutoff(100);

# Check DateValueRangeProcessor works.
function add_vrp_date(&$qp) {
    $vrpdate = new XapianDateValueRangeProcessor(1, 1, 1960);
    $qp->add_valuerangeprocessor($vrpdate);
}
$qp = new XapianQueryParser();
add_vrp_date($qp);
$query = $qp->parse_query('12/03/99..12/04/01');
if ($query->get_description() !== 'Query(0 * VALUE_RANGE 1 19991203 20011204)') {
    print "XapianDateValueRangeProcessor didn't work - result was ".$query->get_description()."\n";
    exit(1);
}

# Feature test for XapianFieldProcessor
class testfieldprocessor extends XapianFieldProcessor {
    function apply($str) {
	if ($str === 'spam') throw new Exception('already spam');
	return new XapianQuery("spam");
    }
}

$tfp = new testfieldprocessor();
$qp->add_prefix('spam', $tfp);
$query = $qp->parse_query('spam:ignored');
if ($query->get_description() !== 'Query(spam)') {
    print "testfieldprocessor didn't work - result was ".$query->get_description()."\n";
    exit(1);
}

try {
    $query = $qp->parse_query('spam:spam');
    print "testfieldprocessor exception not rethrown\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== 'already spam') {
	print "Exception has wrong message\n";
	exit(1);
    }
}

# Test setting and getting metadata
if ($db->get_metadata('Foo') !== '') {
    print "Unexpected value for metadata associated with 'Foo' (expected ''): '".$db->get_metadata('Foo')."'\n";
    exit(1);
}
$db->set_metadata('Foo', 'Foo');
if ($db->get_metadata('Foo') !== 'Foo') {
    print "Unexpected value for metadata associated with 'Foo' (expected 'Foo'): '".$db->get_metadata('Foo')."'\n";
    exit(1);
}

# Test OP_SCALE_WEIGHT and corresponding constructor
$query4 = new XapianQuery(XapianQuery::OP_SCALE_WEIGHT, new XapianQuery('foo'), 5.0);
if ($query4->get_description() != "Query(5 * foo)") {
    print "Unexpected \$query4->get_description()\n";
    exit(1);
}

# Test MultiValueKeyMaker.

$doc = new XapianDocument();
$doc->add_term("foo");
$doc->add_value(0, "ABB");
$db2->add_document($doc);
$doc->add_value(0, "ABC");
$db2->add_document($doc);
$doc->add_value(0, "ABC\0");
$db2->add_document($doc);
$doc->add_value(0, "ABCD");
$db2->add_document($doc);
$doc->add_value(0, "ABC\xff");
$db2->add_document($doc);

$enquire = new XapianEnquire($db2);
$enquire->set_query(new XapianQuery("foo"));

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(5, 4, 3, 2, 1));
}

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0, true);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(1, 2, 3, 4, 5));
}

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0);
    $sorter->add_value(1);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(5, 4, 3, 2, 1));
}

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0, true);
    $sorter->add_value(1);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(1, 2, 3, 4, 5));
}

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0);
    $sorter->add_value(1, true);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(5, 4, 3, 2, 1));
}

{
    $sorter = new XapianMultiValueKeyMaker();
    $sorter->add_value(0, true);
    $sorter->add_value(1, true);
    $enquire->set_sort_by_key($sorter, true);
    $mset = $enquire->get_mset(0, 10);
    mset_expect_order($mset, array(1, 2, 3, 4, 5));
}

# Feature test for ValueSetMatchDecider:
{
    $md = new XapianValueSetMatchDecider(0, true);
    $md->add_value("ABC");
    $doc = new XapianDocument();
    $doc->add_value(0, "ABCD");
    if ($md->apply($doc)) {
	print "Unexpected result from ValueSetMatchDecider->apply(); expected false\n";
	exit(1);
    }

    $doc = new XapianDocument();
    $doc->add_value(0, "ABC");
    if (!$md->apply($doc)) {
	print "Unexpected result from ValueSetMatchDecider->apply(); expected true\n";
	exit(1);
    }

    $mset = $enquire->get_mset(0, 10, 0, null, $md, null);
    mset_expect_order($mset, array(2));

    $md = new XapianValueSetMatchDecider(0, false);
    $md->add_value("ABC");
    $mset = $enquire->get_mset(0, 10, 0, null, $md, null);
    mset_expect_order($mset, array(1, 3, 4, 5));
}

function mset_expect_order($mset, $a) {
    if ($mset->size() != sizeof($a)) {
	print "MSet has ".$mset->size()." entries, expected ".sizeof($a)."\n";
	exit(1);
    }
    for ($j = 0; $j < sizeof($a); ++$j) {
	# PHP4 doesn't cope with: $mset->get_hit($j)->get_docid();
	$hit = $mset->get_hit($j);
	if ($hit->get_docid() != $a[$j]) {
	    print "Expected MSet[$j] to be $a[$j], got ".$hit->get_docid()."\n";
	    exit(1);
	}
    }
}

# Feature tests for Query "term" constructor optional arguments:
$query_wqf = new XapianQuery('wqf', 3);
if ($query_wqf->get_description() != 'Query(wqf#3)') {
    print "Unexpected \$query_wqf->get_description():\n";
    print $query_wqf->get_description() . "\n";
    exit(1);
}

$query = new XapianQuery(XapianQuery::OP_VALUE_GE, 0, "100");
if ($query->get_description() != 'Query(VALUE_GE 0 100)') {
    print "Unexpected \$query->get_description():\n";
    print $query->get_description() . "\n";
    exit(1);
}

$query = XapianQuery::MatchAll();
if ($query->get_description() != 'Query(<alldocuments>)') {
    print "Unexpected \$query->get_description():\n";
    print $query->get_description() . "\n";
    exit(1);
}

$query = XapianQuery::MatchNothing();
if ($query->get_description() != 'Query()') {
    print "Unexpected \$query->get_description():\n";
    print $query->get_description() . "\n";
    exit(1);
}

# Test access to matchspy values:
{
    $matchspy = new XapianValueCountMatchSpy(0);
    $enquire->add_matchspy($matchspy);
    $enquire->get_mset(0, 10);
    $values = array();
    foreach ($matchspy->values_begin() as $k => $term) {
	$values[$term] = $k->get_termfreq();
    }
    $expected = array(
        "ABB" => 1,
	"ABC" => 1,
	"ABC\0" => 1,
	"ABCD" => 1,
	"ABC\xff" => 1,
    );
    if ($values != $expected) {
        print "Unexpected matchspy values():\n";
	var_dump($values);
	var_dump($expected);
	print "\n";
	exit(1);
    }
}

# Regression test for SWIG bug - it was generating "return $r;" in wrapper
# functions which didn't set $r.
$indexer = new XapianTermGenerator();
$doc = new XapianDocument();

$indexer->set_document($doc);
$indexer->index_text("I ask nothing in return");
$indexer->index_text_without_positions("Tea time");
$indexer->index_text("Return in time");

$s = '';
foreach ($doc->termlist_begin() as $term) {
    $s .= $term . ' ';
}
if ($s !== 'ask i in nothing return tea time ') {
    print "PHP Iterator wrapping of TermIterator doesn't work ($s)\n";
    exit(1);
}

$s = '';
foreach ($doc->termlist_begin() as $k => $term) {
    $s .= $term . ':' . $k->get_wdf() . ' ';
}
if ($s !== 'ask:1 i:1 in:2 nothing:1 return:2 tea:1 time:2 ') {
    print "PHP Iterator wrapping of TermIterator keys doesn't work ($s)\n";
    exit(1);
}

# Test GeoSpatial API
$coord = new XapianLatLongCoord();
$coord = new XapianLatLongCoord(-41.288889, 174.777222);

define('COORD_SLOT', 2);
$metric = new XapianGreatCircleMetric();
$range = 42.0;

$centre = new XapianLatLongCoords($coord);
$query = new XapianQuery(new XapianLatLongDistancePostingSource(COORD_SLOT, $centre, $metric, $range));

$db = Xapian::inmemory_open();
$coords = new XapianLatLongCoords();
$coords->append(new XapianLatLongCoord(40.6048, -74.4427));
$doc = new XapianDocument();
$doc->add_term("coffee");
$doc->add_value(COORD_SLOT, $coords->serialise());
$db->add_document($doc);

$centre = new XapianLatLongCoords();
$centre->append(new XapianLatLongCoord(40.6048, -74.4427));

$ps = new XapianLatLongDistancePostingSource(COORD_SLOT, $centre, $metric, $range);
$q = new XapianQuery("coffee");
$q = new XapianQuery(XapianQuery::OP_AND, $q, new XapianQuery($ps));

$enq = new XapianEnquire($db);
$enq->set_query($q);
$mset = $enq->get_mset(0, 10);
if ($mset->size() != 1) {
    print "Expected one result with XapianLatLongDistancePostingSource, got ";
    print $mset->size() . "\n";
    exit(1);
}

$s='';
foreach ($db->allterms_begin() as $k => $term) {
    $s .= "($term:{$k->get_termfreq()})";
}
if ($s !== '(coffee:1)') {
    print "PHP Iterator iteration of allterms doesn't work ($s)\n";
    exit(1);
}

?>
