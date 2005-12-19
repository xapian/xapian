<?php
/* Simple test that we can load the xapian module and run a simple test
 *
 * Copyright (C) 2004,2005 Olly Betts
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

dl("xapian.so");
$stem = new_Stem("english");
$doc = new_Document();
Document_set_data($doc, "is there anybody out there?");
Document_add_term($doc, "XYzzy");
Document_add_posting($doc, Stem_stem_word($stem, "is"), 1);
Document_add_posting($doc, Stem_stem_word($stem, "there"), 2);
Document_add_posting($doc, Stem_stem_word($stem, "anybody"), 3);
Document_add_posting($doc, Stem_stem_word($stem, "out"), 4);
Document_add_posting($doc, Stem_stem_word($stem, "there"), 5);
$db = inmemory_open();
WritableDatabase_add_document($db, $doc);
if (Database_get_doccount($db) != 1) exit(1);
$terms = array("smoke", "test", "terms")
$query = new_Query(Query_OP_OR, $terms)
$query1 = new_Query(Query_OP_PHRASE, array("smoke", "test", "tuple"))
$query2 = new_Query(Query_OP_XOR, array($query, $query1, "string"))
$subqs = array("a", "b")
$query3 = new_Query(Query_OP_OR, $subqs)
