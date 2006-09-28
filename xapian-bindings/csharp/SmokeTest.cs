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

// The Portable.NET compiler has a bug which prevents it finding static
// member functions such as Xapian.Version.Major():
//
// http://savannah.gnu.org/bugs/?func=detailitem&item_id=12231
//
// The workaround is to add an explicit "using Xapian;" here:
using Xapian;

class SmokeTest {
    public static void Main() {
	try {
	    // Test the version number reporting functions give plausible
	    // results.
	    string v = "";
	    v += Xapian.Version.Major();
	    v += ".";
	    v += Xapian.Version.Minor();
	    v += ".";
	    v += Xapian.Version.Revision();
	    string v2 = Xapian.Version.String();
	    if (v != v2) {
		System.Console.WriteLine("Unexpected version output (" + v + " != " + v2 + ")");
		System.Environment.Exit(1);
	    }

	    Xapian.Stem stem = new Xapian.Stem("english");
	    Xapian.Document doc = new Xapian.Document();
	    // Currently SWIG doesn't generate zero-byte clean code for
	    // transferring string between C# and C++.
	    /*
	    doc.SetData("a\0b");
	    if (doc.GetData() == "a") {
		System.Console.WriteLine("GetData+SetData truncates at a zero byte");
		System.Environment.Exit(1);
	    }
	    if (doc.GetData() != "a\0b") {
		System.Console.WriteLine("GetData+SetData doesn't transparently handle a zero byte");
		System.Environment.Exit(1);
	    }
	    */
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

	    // Check exception handling for Xapian::DocNotFoundError.
	    try {
		Xapian.Document doc2 = db.GetDocument(2);
		System.Console.WriteLine("Retrieved non-existent document: " + doc2.ToString());
		System.Environment.Exit(1);
	    } catch (System.Exception e) {
		// We expect DocNotFoundError
		if (e.Message.Substring(0, 16) != "DocNotFoundError") {
                    System.Console.WriteLine("Unexpected exception from accessing non-existent document: " + e.Message);
		    System.Environment.Exit(1);
		}
	    }

	    // Check that OP_ELITE_SET works (up to 0.9.6 it had the wrong
	    // value in C#).
	    try {
		string[] terms = { "hello", "world" };
		Xapian.Query foo2 = new Xapian.Query(Xapian.Query.op.OP_OR, terms, 1);
		Xapian.Query foo = new Xapian.Query(Xapian.Query.op.OP_ELITE_SET, terms, 1);
		System.Console.WriteLine(foo.GetDescription());
	    } catch (System.Exception e) {
		System.Console.WriteLine("Using OP_ELITE_SET causes an exception");
		System.Environment.Exit(1);
	    }
	} catch (System.Exception e) {
	     System.Console.WriteLine("Exception: " + e.ToString());
	     System.Environment.Exit(1);
	}
    }
}
