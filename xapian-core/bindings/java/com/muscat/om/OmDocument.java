package com.muscat.om;

public class OmDocument extends OmReturnObject {
    
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }

    OmDocument (long ptr) { super (ptr); }

    public native OmKey get_key (int keyno);
    public native OmData get_data ();
}

