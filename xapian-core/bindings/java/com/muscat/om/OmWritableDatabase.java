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

    public void begin_session() {
        begin_session(0);
    }

    public native void begin_session(int timeout);
    public native void end_session();

    public native int add_document(OmDocumentContents document);

    public native String get_description();
}
