<?php
/* Simple test that we can load the xapian module and run a simple test
 *
 * Copyright (C) 2004 Olly Betts
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
