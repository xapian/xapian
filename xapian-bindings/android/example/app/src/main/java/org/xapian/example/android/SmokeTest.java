package org.xapian.example.android;// Simple test that we can use xapian from java
//
// Copyright (C) 2005,2006,2007,2008,2011,2016 Olly Betts
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

import android.util.Log;

import org.xapian.*;

// FIXME: need to sort out throwing wrapped Xapian::Error subclasses
//import org.xapian.errors.*;

// FIXME: "implements" not "extends" in JNI Java API
class MyMatchDecider extends MatchDecider {
    public boolean accept(Document d) {
	// NB It's not normally appropriate to call getData() in a MatchDecider
	// but we do it here to make sure we don't get an empty document.
/*	try { */
	    return d.getData().length() == 0;
/*
	} catch (XapianError e) {
	    return true;
	}
*/
    }
}

// FIXME: "implements" not "extends" in JNI Java API
class MyExpandDecider extends ExpandDecider {
    public boolean accept(String s) { return s.charAt(0) != 'a'; }
}

public class SmokeTest {
	public static String TAG="SmokeTest";
    public static void main(String[] args) throws Exception {
	TermGenerator termGenerator = new TermGenerator();
	termGenerator.setFlags(TermGenerator.FLAG_SPELLING);
	try {
	    // Test the version number reporting functions give plausible
	    // results.
	    String v = "";
	    v += Version.major();
	    v += ".";
	    v += Version.minor();
	    v += ".";
	    v += Version.revision();
	    String v2 = Version.string();
	    if (!v.equals(v2)) {
		Log.d(TAG,"Unexpected version output (" + v + " != " + v2 + ")");
		return;
	    }

	    Stem stem = new Stem("english");
	    if (!stem.toString().equals("Xapian::Stem(english)")) {
		Log.d(TAG,"Unexpected stem.toString()");
		return;
	    }
	    Document doc = new Document();
	    doc.setData("a\000b");
	    String s = doc.getData();
	    if (s.equals("a")) {
		Log.d(TAG,"getData+setData truncates at a zero byte");
		return;
	    }
	    if (!s.equals("a\000b")) {
		Log.d(TAG,"getData+setData doesn't transparently handle a zero byte");
		return;
	    }
	    doc.setData("is there anybody out there?");
	    doc.addTerm("XYzzy");
// apply was stemWord() in the JNI bindings
	    doc.addPosting(stem.apply("is"), 1);
	    doc.addPosting(stem.apply("there"), 2);
	    doc.addPosting(stem.apply("anybody"), 3);
	    doc.addPosting(stem.apply("out"), 4);
	    doc.addPosting(stem.apply("there"), 5);
	    WritableDatabase db = new WritableDatabase("", Xapian.DB_BACKEND_INMEMORY);
	    db.addDocument(doc);
	    if (db.getDocCount() != 1) {
		Log.d(TAG,"Unexpected db.getDocCount()");
		return;
	    }

            if (!Query.MatchAll.toString().equals("Query(<alldocuments>)")) {
		Log.d(TAG,"Unexpected Query.MatchAll.toString()");
		return;
            }

            if (!Query.MatchNothing.toString().equals("Query()")) {
		Log.d(TAG,"Unexpected Query.MatchNothing.toString()");
		return;
            }

	    String[] terms = { "smoke", "test", "terms" };
	    Query query = new Query(Query.OP_OR, terms);
	    if (!query.toString().equals("Query((smoke OR test OR terms))")) {
		Log.d(TAG,"Unexpected query.toString()");
		return;
	    }
	    Query[] queries = { new Query("smoke"), query, new Query("string") };
	    Query query2 = new Query(Query.OP_XOR, queries);
	    if (!query2.toString().equals("Query((smoke XOR (smoke OR test OR terms) XOR string))")) {
		Log.d(TAG,"Unexpected query2.toString()");
		return;
	    }
	    String[] subqs = { "a", "b" };
	    Query query3 = new Query(Query.OP_OR, subqs);
	    if (!query3.toString().equals("Query((a OR b))")) {
		Log.d(TAG,"Unexpected query3.toString()");
		return;
	    }
	    Enquire enq = new Enquire(db);
	    enq.setQuery(new Query(Query.OP_OR, "there", "is"));
	    MSet mset = enq.getMSet(0, 10);
	    if (mset.size() != 1) {
		Log.d(TAG,"Unexpected mset.size()");
		return;
	    }
	    MSetIterator m_itor = mset.begin();
	    Document m_doc = null;
	    long m_id;
	    while(m_itor.hasNext()) {
		m_id = m_itor.next();
		if(m_itor.hasNext()) {
		    m_doc = mset.getDocument(m_id);
		}
	    }

	    // Only one doc exists in this mset
	    if(m_doc != null && m_doc.getDocId() != 0) {
		Log.d(TAG,"Unexpected docid");
		    return;
	    }

	    String term_str = "";
	    TermIterator itor = enq.getMatchingTermsBegin(mset.getElement(0));
	    while (itor.hasNext()) {
		term_str += itor.next();
		if (itor.hasNext())
		    term_str += ' ';
	    }
	    if (!term_str.equals("is there")) {
		Log.d(TAG,"Unexpected term_str");
		return;
	    }
/* FIXME:dc: Fails since Xapian::Error is still unmapped
	    boolean ok = false;
	    try {
		Database db_fail = new Database("NOsuChdaTabASe");
		// Ignore the return value.
		db_fail.getDocCount();
	    } catch (DatabaseOpeningError e) {
		ok = true;
	    }
	    if (!ok) {
		Log.d(TAG,"Managed to open non-existent database");
		return;
	    }
*/
/*
	    if (Query.OP_ELITE_SET != 10) {
		Log.d(TAG,"OP_ELITE_SET is " + Query.OP_ELITE_SET + " not 10");
		return;
	    }
*/
	    RSet rset = new RSet();
	    rset.addDocument(1);
	    ESet eset = enq.getESet(10, rset, new MyExpandDecider());
	    // FIXME: temporary simple check
	    if (0 == eset.size()) {
		Log.d(TAG,"ESet.size() was 0");
		return;
	    }

	    int count = 0;
	    for(ESetIterator eit = eset.begin(); eit.hasNext(); ) {
	    // for (int i = 0; i < eset.size(); i++) {
		if (eit.getTerm().charAt(0) == 'a') {
		    Log.d(TAG,"MyExpandDecider wasn't used");
		    return;
		}
		++count;
		eit.next();
	    }
	    if (count != eset.size()) {
		Log.d(TAG,"ESet.size() mismatched number of terms returned by ESetIterator");
		Log.d(TAG,count + " " + eset.size());
		return;
	    }

/*
	    MSet mset2 = enq.getMSet(0, 10, null, new MyMatchDecider());
	    if (mset2.size() > 0) {
		Log.d(TAG,"MyMatchDecider wasn't used");
		return;
	    }
*/

	    if (!enq.getQuery().toString().equals("Query((there OR is))")) {
		Log.d(TAG,"Enquire::getQuery() returned the wrong query: " + enq.getQuery().toString());
		return;
	    }
        Log.d(TAG,"All tests ran successfully");
	} catch (Exception e) {
	    Log.d(TAG,"Caught unexpected exception " + e.toString());
	    return;
	}
    }
}
