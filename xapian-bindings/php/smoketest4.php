<?php
/* PHP4 specific tests.
 *
 * Copyright (C) 2006,2007 Olly Betts
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

# Test the version number reporting functions give plausible results.
$v = xapian_major_version().'.'.xapian_minor_version().'.'.xapian_revision();
$v2 = xapian_version_string();
if ($v != $v2) {
    print "Unexpected version output ($v != $v2)\n";
    exit(1);
}

$db = inmemory_open();

function my_errhandler($type, $str) {
    global $last_exception;
    if ($type == E_WARNING) $last_exception = $str;
}

$old_errhandler = set_error_handler("my_errhandler");
# Check PHP4 handling of Xapian::DocNotFoundError
$old_error_reporting = error_reporting();
if ($old_error_reporting & E_WARNING)
    error_reporting($old_error_reporting ^ E_WARNING);
$doc2 = $db->get_document(2);
if ($doc2 != null) {
    print "Retrieved non-existent document\n";
    exit(1);
}
if ($last_exception !== "DocNotFoundError: Docid 2 not found") {
    print "Exception string not as expected, got: '$last_exception'\n";
    exit(1);
}
# Check QueryParser parsing error.
$qp = new XapianQueryParser;
$qp->parse_query("test AND");
if ($last_exception !== "QueryParserError: Syntax: <expression> AND <expression>") {
    print "Exception string not as expected, got: '$last_exception'\n";
    exit(1);
}
if ($old_error_reporting & E_WARNING)
    error_reporting($old_error_reporting);
set_error_handler($old_errhandler);

# Regression test for bug#193, fixed in 1.0.3.
$vrp = new XapianNumberValueRangeProcessor(0, '$', true);
$a = '$10';
$b = '20';
$vrp->apply($a, $b);
if (xapian_sortable_unserialise($a) != 10) {
    print Xapian::sortable_unserialise($a)." != 10\n";
    exit(1);
}
if (xapian_sortable_unserialise($b) != 20) {
    print Xapian::sortable_unserialise($b)." != 20\n";
    exit(1);
}

$op_or = XapianQuery_OP_OR;
$op_phrase = XapianQuery_OP_PHRASE;
$op_xor = XapianQuery_OP_XOR;
$op_elite_set = XapianQuery_OP_ELITE_SET;
$op_scale_weight = XapianQuery_OP_SCALE_WEIGHT;

?>
