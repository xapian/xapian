package com.muscat.om;

public class OmExpandOptions extends OmObject {

    public OmExpandOptions () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    /** This sets whether terms which are already in the query will
     *  be returned by the match.  By default, such terms will not
     *  be returned.  A value of true will allow query terms to be
     *  placed in the ESet.
     */
    public native void use_query_terms (boolean allow_query_terms);
}
