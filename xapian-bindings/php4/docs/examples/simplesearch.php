<?php
/* $Id$
 * Simple command-line search program
 *
 * ----START-LICENCE----
 * Copyright 2004 James Aylett
 * Copyright 2004 Olly Betts
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

define('MAX_PROB_TERM_LENGTH', 64);

if (!isset($_SERVER['argv']) or count($_SERVER['argv']) < 3) {
    print "usage: {$_SERVER['argv'][0]} <path to database> <search terms>\n";
    exit;
}

$database = open($_SERVER['argv'][1]);
if (!$database) {
    print "Died! :-(\n";
    exit;
}

$enquire = new_Enquire($database);
$stemmer = new_Stem("english");
$query = NULL;
foreach (array_slice($_SERVER['argv'], 2) as $term) {
    $nextquery = new_Query(Stem_stem_word($stemmer, strtolower($term)));
    if ($query === NULL) {
	$query = $nextquery;
    } else {
	$query = new_Query_from_query_pair(Query_OP_OR, $query, $nextquery);
    }
}

print "Performing query `" . Query_get_description($query) . "'\n";

Enquire_set_query($enquire, $query);
$matches = Enquire_get_mset($enquire, 0, 10);

print MSet_get_matches_estimated($matches) . " results found\n";
$mseti = MSet_begin($matches);
while (! MSetIterator_equals($mseti, MSet_end($matches))) {
    print "ID " . MSetIterator_get_docid($mseti) . " " .
	MSetIterator_get_percent($mseti) . "% [" .
	Document_get_data(MSetIterator_get_document($mseti)) . "]\n";
    MSetIterator_next($mseti);
}
?>
