package com.muscat.om;

public class OmStem extends OmObject {

    protected OmStem () { }
    
    public OmStem (String lang) {
	nativePtr = createNativeObject (lang);
    }	
    protected native long createNativeObject (String lang);

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native String stem_word (String word);

    /** Ask for a list of available languages.  An OmStem object is
     *  not required for this operation.
     *
     * this appears to be undefined ATM, which upsets Java 2
     */  
    //public static native String[] get_available_languages();

}
