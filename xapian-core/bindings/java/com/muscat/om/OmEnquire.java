package com.muscat.om;

public class OmEnquire extends OmObject {

    private OmEnquire () { }
    
    public OmEnquire (OmDatabaseGroup db) {
	nativePtr = createNativeObject (db);
    }
	
    protected native long createNativeObject (OmDatabaseGroup db);
    protected native void deleteNativeObject ();
    public void finalize () throws Throwable {
	super.finalize ();
    }

    /** Get the document data by match set item. */
    public native OmDocument get_doc (OmMSetItem mitem);
            
    /** Get the document data by document id. */
    public native OmDocument get_doc (int did);

    /** Get (a portion of) the match set for the current query. */
    public OmMSet get_mset (int first, int maxitems, OmRSet rset,
    			    OmMatchOptions moptions)
    {
	return get_mset (first, maxitems, rset, moptions, null);
    }
    public OmMSet get_mset (int first, int maxitems) {
	return get_mset (first, maxitems, null, null, null);
    }
    public native OmMSet get_mset (int first, int maxitems, 
				   OmRSet rset, OmMatchOptions moptions, OmMatchDecider mdecider);

    /** Set the query to run. */
    public native void set_query (OmQuery query);

    /** get an expand set of terms */
    public OmESet get_eset (int maxitems, OmRSet rset) {
	return get_eset (maxitems, rset, null, null);
    }
    public native OmESet get_eset (int maxitems, OmRSet rset, 
				   OmExpandOptions eopts, OmExpandDecider edecider);

    /** Get terms which match a given document, by match set item. */
    public String[] get_matching_terms (OmMSetItem mitem) {
	return get_matching_terms (mitem.get_did());
    }

    /** Get terms which match a given document, by document id. */
    public native String[] get_matching_terms (int did);

}
