<?php
/* Simple command-line search program
 *
 * Copyright (C) 2004 James Aylett
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

if (!isset($_SERVER['argv']) or count($_SERVER['argv']) < 3) {
    print "usage: {$_SERVER['argv'][0]} <path to database> <search terms>\n";
    exit;
}

$database = new_Database($_SERVER['argv'][1]);
if (!$database) {
    print "Couldn't open database '{$_SERVER['argv'][1]}'\n";
    exit;
}

$enquire = new_Enquire($database);
$stemmer = new_Stem("english");
$terms = array();
foreach (array_slice($_SERVER['argv'], 2) as $term) {
    array_push($terms, Stem_stem_word($stemmer, strtolower($term)));
}
$query = new_Query(Query_OP_OR, $terms);

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
