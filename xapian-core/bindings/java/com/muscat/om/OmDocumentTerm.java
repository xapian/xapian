package com.muscat.om;

public class OmDocumentContents extends OmObject {

    public OmDocumentContents () { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    /* The data making up the document. */

    /** The data to be stored along with this document in the index. */
    String data;

    
}
