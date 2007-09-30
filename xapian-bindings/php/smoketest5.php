<?php
/* PHP5 specific tests.
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
$v = Xapian::major_version().'.'.Xapian::minor_version().'.'.Xapian::revision();
$v2 = Xapian::version_string();
if ($v != $v2) {
    print "Unexpected version output ($v != $v2)\n";
    exit(1);
}

$db = Xapian::inmemory_open();

# Check PHP5 handling of Xapian::DocNotFoundError
try {
    $doc2 = Database_get_document($db, 2);
    print "Retrieved non-existent document\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "DocNotFoundError: Docid 2 not found") {
	print "DocNotFoundError Exception string not as expected, got: '$e->getMessage()'\n";
	exit(1);
    }
}

# Check QueryParser parsing error.
try {
    $qp = new XapianQueryParser;
    $qp->parse_query("test AND");
    print "Successfully parsed bad query\n";
    exit(1);
} catch (Exception $e) {
    if ($e->getMessage() !== "QueryParserError: Syntax: <expression> AND <expression>") {
	print "QueryParserError Exception string not as expected, got: '$e->getMessage()'\n";
	exit(1);
    }
}

$op_or = XapianQuery::OP_OR;
$op_phrase = XapianQuery::OP_PHRASE;
$op_xor = XapianQuery::OP_XOR;
$op_elite_set = XapianQuery::OP_ELITE_SET;
$op_mult_weight = XapianQuery::OP_MULT_WEIGHT;

?>
