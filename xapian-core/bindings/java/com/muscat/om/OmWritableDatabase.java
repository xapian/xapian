package com.muscat.om;

public class OmWritableDatabase extends OmDatabase {

    public OmWritableDatabase (String type, String[] params) throws OmError { 
	nativePtr = createNativeObject (type, params);
    }
    protected native long createNativeObject (String type, String[] params);

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public void lock() {
        lock(0);
    }

    public native void lock(int timeout);
    public native void unlock();

//    public native int add_document(OmDocumentContents document);

    public native String get_description();
}
