package com.muscat.om;

public class OmESet extends OmReturnObject {
    
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }
    
    OmESet (long ptr) { super (ptr); }

    public native OmVector get_items ();
    
    public native int get_ebound ();
}
