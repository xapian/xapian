package com.muscat.om;
import java.util.*;

/**
 * to keep to Java conventions, access methods return Object references. However,
 * this class is subclassed to match the actual object type, for efficient implementation
 */

public abstract class OmVector extends OmReturnObject {

    OmVector (long ptr) { super (ptr); }

    protected void finalize () throws Throwable {
	super.finalize ();
    }

    public abstract int size ();                    // native impl
    public abstract Object elementAt (int i);       // native impl

    public Enumeration elements () {
	return new OmEnumeration ();
    }

    class OmEnumeration implements Enumeration {
	int i = 0;
	int sz;
	
	public OmEnumeration () {
	    sz = size ();
	}
	
	public Object nextElement () {
	    return elementAt (i++);
	}
	
	public boolean hasMoreElements () {
	    return (i < sz);
	}
    }
}
