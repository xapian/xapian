// Simple test that we can load the xapian module and run a simple test
//
// Copyright (C) 2004 Olly Betts
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA

using Xapian;

class SmokeTest {
    public static void Main() {
	try {
	    Stem stem = new Stem("english");
	    Document doc = new Document();
	    doc.set_data("is there anybody out there?");
	    doc.add_term("XYzzy", 1);
	    doc.add_posting(stem.stem_word("is"), 1, 1);
	    doc.add_posting(stem.stem_word("there"), 2, 1);
	    doc.add_posting(stem.stem_word("anybody"), 3, 1);
	    doc.add_posting(stem.stem_word("out"), 4, 1);
	    doc.add_posting(stem.stem_word("there"), 5, 1);
// FIXME: inmemory_open fails to link...
//	    WritableDatabase db = inmemory_open();
//	    db.add_document(doc);
//	    if (db.get_doccount() != 1) {
//		System.Environment.Exit(1);
//	    }
	} catch {
	     // FIXME: give details of the exception
	     System.Console.WriteLine("Exception:");
	     System.Environment.Exit(1);
	}
    }
}
