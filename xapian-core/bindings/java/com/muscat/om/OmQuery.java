package com.muscat.om;

public class OmQuery extends OmObject {

    public void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    /** A null query */
    public OmQuery () { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    /** A query consisting of a single term. (This must *not* be an empty string.) */
    public OmQuery (String term) throws OmError { this (term, 1, 0); }
    public OmQuery (String term, int wqf, int pos) throws OmError {
	nativePtr = createNativeObject (term, wqf, pos);
    }
    protected native long createNativeObject (String term, int termcount, int termpos);

    /** A query consiting of two subqueries op'd together */
    public OmQuery (String op, OmQuery left, OmQuery right) {
	nativePtr = createNativeObject (op, left, right);
    }
    protected native long createNativeObject (String op, OmQuery left, OmQuery right);

    /** A query consisting of individual terms op'd together */
    public OmQuery (String op, String[] terms) throws OmError {
	nativePtr = createNativeObject (op, terms);
    }
    protected native long createNativeObject (String op, String[] terms);

    /** get terms in query */
    public native String[] get_terms ();

//    string get_description() const 
//	      Returns a string representing the query. 
//	  om_termcount get_length() const 
//	      Get the length of the query, used by some ranking formulae. 
//	  bool is_bool() const 
//	      Check whether the query is (pure) boolean. 
//	  bool is_defined() const 
//	      Check whether the query is defined. 
//	  OmQuery& operator=(const OmQuery & copyme) 
//	      Assignment. 
//	  bool set_bool(bool isbool_) 
//	      Set whether the query is a pure boolean. 
//	  om_termcount set_length(om_termcount qlen_) 
//	      Set the length of the query. 
//	   ~OmQuery() 
//	      Destructor. 

}
