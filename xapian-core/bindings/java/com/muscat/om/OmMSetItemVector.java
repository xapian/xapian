package com.muscat.om;
import java.util.*;

/**
 * implementation of OmVector for OmMSetItems
 */

class OmMSetItemVector extends OmVector {

    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }

    OmMSetItemVector (long ptr) { super (ptr); }

    public native int size ();
    public native Object elementAt (int i);
}
