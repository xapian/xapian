package com.muscat.om;

public class OmESetItem extends OmReturnObject {
    
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }

    OmESetItem (long ptr) { super (ptr); }

    public native String get_tname ();
    public native double get_wt ();
}
