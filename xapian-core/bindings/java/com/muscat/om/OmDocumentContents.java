package com.muscat.om;

import java.util.*;

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

    /** Set the data to be stored along with this document in the index. */
    public native void set_data(String data);

    /** Add a "key" number keyno with value */
    public native void add_key(int keyno, String value);

    /** Add an occurrence of a term to the document. */
    public void add_posting(String tname) {
        add_posting(tname, 0);
    }

    /** Add an occurrence of a term to the document. */
    public native void add_posting(String tname, int tpos);

    /** Get the description of this object. */
    public native String get_description();
}
