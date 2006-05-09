// Simple test that we can load the xapian module and run a simple test
//
// Copyright (C) 2004,2005,2006 Olly Betts
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
// USA

class SmokeTest {
    public static void Main() {
	try {
	    // Test the version number reporting functions give plausible
	    // results.
	    string v = "";
	    v += Xapian.Xapian.XapianMajorVersion();
	    v += ".";
	    v += Xapian.Xapian.XapianMinorVersion();
	    v += ".";
	    v += Xapian.Xapian.XapianRevision();
	    string v2 = Xapian.Xapian.XapianVersionString();
	    if (v != v2) {
		System.Console.WriteLine("Unexpected version output (" + v + " != " + v2 + ")");
		System.Environment.Exit(1);
	    }

	    Xapian.Stem stem = new Xapian.Stem("english");
	    Xapian.Document doc = new Xapian.Document();
	    doc.SetData("is there anybody out there?");
	    doc.AddTerm("XYzzy");
	    doc.AddPosting(stem.StemWord("is"), 1);
	    doc.AddPosting(stem.StemWord("there"), 2);
	    doc.AddPosting(stem.StemWord("anybody"), 3);
	    doc.AddPosting(stem.StemWord("out"), 4);
	    doc.AddPosting(stem.StemWord("there"), 5);
	    Xapian.WritableDatabase db = Xapian.InMemory.Open();
	    db.AddDocument(doc);
	    if (db.GetDocCount() != 1) {
		System.Environment.Exit(1);
	    }
	    if (doc.TermListCount() != 5) {
		System.Environment.Exit(1);
	    }
	    int count = 0;
	    Xapian.TermIterator i = doc.TermListBegin();
	    while (i != doc.TermListEnd()) {
		++count;
		++i;
	    }
	    if (count != 5) {
		System.Environment.Exit(1);
	    }
	} catch (System.Exception e) {
	     System.Console.WriteLine("Exception: " + e.ToString());
	     System.Environment.Exit(1);
	}
    }
}
