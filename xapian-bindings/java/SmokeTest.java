// Simple test that we can use xapian from java
//
// Copyright (C) 2005 Olly Betts
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

import org.xapian.*;

public class SmokeTest {
    public static void main(String args[]) throws Exception {
	try {
	    Stem stem = new Stem("english");
	    Document doc = new Document();
	    doc.setData("is there anybody out there?");
	    doc.addTerm("XYzzy");
	    doc.addPosting(stem.stemWord("is"), 1);
	    doc.addPosting(stem.stemWord("there"), 2);
	    doc.addPosting(stem.stemWord("anybody"), 3);
	    doc.addPosting(stem.stemWord("out"), 4);
	    doc.addPosting(stem.stemWord("there"), 5);
	    WritableDatabase db = Xapian.InMemory.open();
	    db.addDocument(doc);
	    if (db.getDocCount() != 1) {
		System.exit(1);
	    }
	} catch (Exception e) {
	    System.exit(1);
	}
    }
}
