package com.muscat.om;

import java.util.*;

/**
 * This class is the root of the OM classes which can be instantiated from Java.
 * When the Java object is created, a corresponding native object is also instantiated.
 */

public abstract class OmObject {
    protected long nativePtr;             // pointer to the C++ object

    /**
     * when the Java object is garbage collected, delete the native object
     */
    protected void finalize () throws Throwable {
	if (nativePtr != 0) {
	    //_decr_object_count (this.getClass().getName());
	    deleteNativeObject ();
	    nativePtr = 0;
	}
    }
    
    protected abstract void deleteNativeObject ();

    static {
	System.loadLibrary ("omusjava");
    }
}
