package com.muscat.om;

/**
 * This class is the root of the OM classes which can be instantiated from Java.
 * When the Java object is created, a corresponding native object is also instantiated.
 */

public abstract class OmReturnObject {
    protected long nativePtr;            // pointer to the C++ object

    private OmReturnObject () { }        // prevent public construction

    /**
     * called from C++ - ptr points to the native object
     */
    OmReturnObject (long ptr) {
	nativePtr = ptr;
    }

    /**
     * when the Java object is garbage collected, delete the native object
     */
    protected void finalize () throws Throwable {
	if (nativePtr != 0) {
	    deleteNativeObject ();
	}
    }
    
    abstract protected void deleteNativeObject ();
}
