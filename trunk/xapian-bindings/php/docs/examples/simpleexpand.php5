<?php
/* Simple example PHP5 script demonstrating query expansion.
 *
 * Copyright (C) 2007 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

if (php_sapi_name() != "cli") {
    print "This example script is written to run under the command line ('cli') version of\n";
    print "the PHP interpreter, but you're using the '".php_sapi_name()."' version\n";
    exit(1);
}

include "xapian.php";

if ($argc < 3) {
    print "Usage: {$argv[0]} PATH_TO_DATABASE QUERY [-- [DOCID...]]\n";
    exit(1);
}

try {
    // Open the database for searching.
    $database = new XapianDatabase($argv[1]);

    // Start an enquire session.
    $enquire = new XapianEnquire($database);

    // Combine command line arguments up to "--" with spaces between
    // them, so that simple queries don't have to be quoted at the shell
    // level.
    $args = array_slice($argv, 2);
    $separator = array_search("--", $args);

    if ($separator === FALSE) {
	$separator = count($args);
    }

    $query_string = join(" ", array_slice($args, 0, $separator));

    $rset = new XapianRSet();
    foreach (array_slice($args, $separator + 1) as $docid) {
	$rset->add_document(intval($docid));
    }

    $qp = new XapianQueryParser();
    $stemmer = new XapianStem("english");
    $qp->set_stemmer($stemmer);
    $qp->set_database($database);
    $qp->set_stemming_strategy(XapianQueryParser::STEM_SOME);
    $query = $qp->parse_query($query_string);
    print "Parsed query is: {$query->get_description()}\n";

    // Find the top 10 results for the query.
    $enquire->set_query($query);
    $matches = $enquire->get_mset(0, 10, $rset);

    // Display the results.
    print "{$matches->get_matches_estimated()} results found:\n";

    $i = $matches->begin();
    while (!$i->equals($matches->end())) {
	$n = $i->get_rank() + 1;
	$data = $i->get_document()->get_data();
	print "$n: {$i->get_percent()}% docid={$i->get_docid()} [$data]\n\n";
	$i->next();
    }

    // If no relevant docids were given, invent an RSet containing the top 5
    // matches (or all the matches if there are less than 5).
    if ($rset->is_empty()) {
	$c = 5;
	$i = $matches->begin();
	while ($c-- && !$i->equals($matches->end())) {
	    $rset->add_document($i->get_docid());
	    $i->next();
	}
    }

    // Generate an ESet containing terms that the user might want to add to
    // the query.
    $eset = $enquire->get_eset(10, $rset);

    // List the terms.
    for ($t = $eset->begin(); !$t->equals($eset->end()); $t->next()) {
	print "{$t->get_term()}: weight = {$t->get_weight()}\n";
    }
} catch (Exception $e) {
    print $e->getMessage() . "\n";
    exit(1);
}
?>
