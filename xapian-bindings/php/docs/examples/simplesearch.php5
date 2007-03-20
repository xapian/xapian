<?php
/* Simple command-line search program
 *
 * Copyright (C) 2004 James Aylett
 * Copyright (C) 2004,2005,2006,2007 Olly Betts
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

if (php_sapi_name() != "cli") {
    print "This example script is written to run under the command line ('cli') version of\nthe PHP interpreter, but you're using the '".php_sapi_name()."' version\n";
    exit;
}

include "php5/xapian.php";

if (!isset($_SERVER['argv']) or count($_SERVER['argv']) < 3) {
    print "usage: {$_SERVER['argv'][0]} <path to database> <search terms>\n";
    exit;
}

$database = new XapianDatabase($_SERVER['argv'][1]);
if (!$database) {
    print "Couldn't open database '{$_SERVER['argv'][1]}'\n";
    exit;
}

$enquire = new XapianEnquire($database);
$stemmer = new XapianStem("english");
$terms = array();
foreach (array_slice($_SERVER['argv'], 2) as $term) {
    array_push($terms, $stemmer->stem_word(strtolower($term)));
}
$query = new XapianQuery(XapianQuery::OP_OR, $terms);

print "Performing query `" . $query->get_description() . "'\n";

$enquire->set_query($query);
$matches = $enquire->get_mset(0, 10);

print $matches->get_matches_estimated() . " results found\n";
$mseti = $matches->begin();
while (!$mseti->equals($matches->end())) {
    print "ID " . $mseti->get_docid() . " " .
	$mseti->get_percent() . "% [" .
	$mseti->get_document()->get_data() . "]\n";
    $mseti->next();
}
?>
