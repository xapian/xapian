package com.muscat.om;

public class OmDatabaseGroup extends OmObject {

    public OmDatabaseGroup () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    /** add a new database */
    public void add_database (String type, String single_param) {
	String[] parms = { single_param };
	add_database (type, parms);
    }

    /** add a new database */
    public native void add_database (OmDatabase db);

    /** add a new database */
    public native void add_database (String type, String[] params);
}
