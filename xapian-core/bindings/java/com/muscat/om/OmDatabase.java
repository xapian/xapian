package com.muscat.om;

public class OmDatabase extends OmObject {

    public OmDatabase (String type, String[] params) throws OmError { 
	nativePtr = createNativeObject (type, params);
    }
    protected native long createNativeObject (String type, String[] params);

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native String get_description();
}
