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
    public OmQuery (String term, int wqf) throws OmError { this (term, wqf, 0); }
    public OmQuery (String term, int wqf, int pos) throws OmError {
	nativePtr = createNativeObject (term, wqf, pos);
    }
    protected native long createNativeObject (String term, int termcount, int termpos) throws OmError;

    /** A query consiting of two subqueries op'd together */
    public OmQuery (String op, OmQuery left, OmQuery right) throws OmError {
	nativePtr = createNativeObject (op, left, right);
    }
    protected native long createNativeObject (String op, OmQuery left, OmQuery right) throws OmError;

    /** A query consisting of individual terms op'd together (with window
     *  parameter defaulted)
     */
    public OmQuery (String op, String[] terms) throws OmError {
        this(op, terms, 0);
    }
    /** A query consisting of individual terms op'd together */
    public OmQuery (String op, String[] terms, int window) throws OmError {
	nativePtr = createNativeObject (op, terms, window);
    }
    protected native long createNativeObject (String op, String[] terms, int window);

    /** get terms in query */
    public native String[] get_terms ();

    /** Returns a string representing the query */
    public native String get_description();

    /** Get the length of the query, used by some ranking formulae. */
    public native int get_length();

    /** Set whether the query is a pure boolean. */
    public native boolean set_bool(boolean isbool_);

//	  bool is_bool() const 
//	      Check whether the query is (pure) boolean. 
//	  bool is_defined() const 
//	      Check whether the query is defined. 
//	  OmQuery& operator=(const OmQuery & copyme) 
//	      Assignment. 
//	  om_termcount set_length(om_termcount qlen_) 
//	      Set the length of the query. 
//	   ~OmQuery() 
//	      Destructor. 

}
