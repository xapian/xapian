package com.muscat.om;

/**
 * no public Java methods - this is just a wrapper for the C++ class
 */

public class OmLowercaseExpandDecider extends OmExpandDecider { 
    public OmLowercaseExpandDecider () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }
}
