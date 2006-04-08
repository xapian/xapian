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

dl("xapian.so");
$stem = new_Stem("english");
if (Stem_get_description($stem) != "Xapian::Stem(english)") exit(1);
$doc = new_Document();
Document_set_data($doc, "is there anybody out there?");
Document_add_term($doc, "XYzzy");
Document_add_posting($doc, Stem_stem_word($stem, "is"), 1);
Document_add_posting($doc, Stem_stem_word($stem, "there"), 2);
Document_add_posting($doc, Stem_stem_word($stem, "anybody"), 3);
Document_add_posting($doc, Stem_stem_word($stem, "out"), 4);
Document_add_posting($doc, Stem_stem_word($stem, "there"), 5);
$db = inmemory_open();
// Check virtual function dispatch.
if (WritableDatabase_get_description($db) != Database_get_description($db))
    exit(1);
WritableDatabase_add_document($db, $doc);
if (Database_get_doccount($db) != 1) exit(1);
$terms = array("smoke", "test", "terms");
$query = new_Query(Query_OP_OR, $terms);
if (Query_get_description($query) != "Xapian::Query((smoke OR test OR terms))") exit(1);
$query1 = new_Query(Query_OP_PHRASE, array("smoke", "test", "tuple"));
if (Query_get_description($query1) != "Xapian::Query((smoke PHRASE 3 test PHRASE 3 tuple))") exit(1);
$query2 = new_Query(Query_OP_XOR, array(new_Query("smoke"), $query1, "string"));
if (Query_get_description($query2) != "Xapian::Query((smoke XOR (smoke PHRASE 3 test PHRASE 3 tuple) XOR string))") exit(1);
$subqs = array("a", "b");
$query3 = new_Query(Query_OP_OR, $subqs);
if (Query_get_description($query3) != "Xapian::Query((a OR b))") exit(1);
$enq = new_Enquire($db);
Enquire_set_query($enq, new_Query(Query_OP_OR, "there", "is"));
$mset = Enquire_get_mset($enq, 0, 10);
if (MSet_size($mset) != 1) exit(1);
$terms = join(" ", Enquire_get_matching_terms($enq, MSet_get_hit($mset, 0)));
if ($terms != "is there") exit(1);

# Check PHP4 handling of Xapian::DocNotFoundError
$old_error_reporting = error_reporting();
if ($old_error_reporting & E_WARNING)
    error_reporting($old_error_reporting ^ E_WARNING);
$doc2 = Database_get_document($db, 2);
if ($doc2 != null) {
    exit(1);
}
if ($old_error_reporting & E_WARNING)
    error_reporting($old_error_reporting);

# Regression test - overload resolution involving boolean types failed.
Enquire_set_sort_by_value($enq, 1, TRUE);

?>
