package com.muscat.om;

public class OmMatchOptions extends OmObject {
    
    public OmMatchOptions () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native void set_collapse_key (int key_);

    /** Set no key to collapse on.  This is the default. */
    public native void set_no_collapse ();

    /** Set direction of sorting.  This applies only to documents which 
     *  have the same weight, which will only ever occur with some
     *  weighting schemes.
     */
    public native void set_sort_forward (boolean forward_);
    
    /** Set a percentage cutoff for the match.  Only documents
     *  with a percent weight of at least this percentage will
     *  be returned in the mset.  (If the intention is to return
     *  only matches which contain all the terms in the query,
     *  then consider using OM_MOP_AND instead of OM_MOP_OR in
     *  the query.)  The percentage must be between 0 and 100, or
     *  an OmInvalidArgumentError will be thrown.
     */
    public native void set_percentage_cutoff (int percent_);

    public native void set_max_or_terms (int n);
}
