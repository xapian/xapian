package com.muscat.om;

/**
 * pick out expand terms which match the specified prefix
 */

public class OmPrefixExpandDecider extends OmExpandDecider { 
    private OmPrefixExpandDecider () { }
    protected long createNativeObject () { return 0; }

    public OmPrefixExpandDecider (String prefix) {
	nativePtr = createNativeObject (prefix);
    }
    protected native long createNativeObject (String prefix);
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }
}
