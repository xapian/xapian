package com.muscat.om;

public class OmMSetItem extends OmReturnObject {
    
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }

    OmMSetItem (long ptr) { super (ptr); }

    /** the document ID */
    public native int get_did();

    /** the document weight */
    public native double get_wt();

    /** the collapse key, if this option is used */
    public native OmKey get_collapse_key();
}

