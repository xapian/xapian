package com.muscat.om;

public class OmMSet extends OmReturnObject {
    
    protected native void deleteNativeObject ();
    protected void finalize () throws Throwable {
	super.finalize ();
    }

    OmMSet (long ptr) { super (ptr); }

    /** The index of the first item in the result which was put into the mset. */
    public native int get_firstitem();
    
    /** A list of items comprising the (selected part of the) mset. */
    public native OmVector get_items();

    /** The greatest weight which is attained in the mset. */
    public native double get_max_attained();   

    /** The maximum possible weight in the mset. */
    public native double get_max_possible();
        
    /** A lower bound on the number of documents in the database which have a 
	weight greater than zero. */
    public native int get_mbound();

    /** Return the percentage score for the given item. */
    public int convert_to_percent (OmMSetItem item) {
	return convert_to_percent (item.get_wt());
    }
     
    /** This converts the weight supplied to a percentage score. */
    public native int convert_to_percent (double weight);
}

