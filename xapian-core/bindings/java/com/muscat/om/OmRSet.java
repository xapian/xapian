package com.muscat.om;

public class OmRSet extends OmObject {

    public OmRSet () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native int[] get_items ();
    public native void  add_document (int did);
    public native void  remove_document (int did);
}
