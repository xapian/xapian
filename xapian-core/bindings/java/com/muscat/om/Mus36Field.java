package com.muscat.om;

import java.util.*;

/**
 * a class representing fields in Muscat 3.6, and able to parse their binary
 * representation 
 */

public class Mus36Field {
    public static final byte TYPE_GROUP  = 1;
    public static final byte TYPE_STRING = 2;
    public static final byte TYPE_INT    = 3;
    public static final byte TYPE_BINARY = 4;
    
    private static final int SH_8  = 256;
    private static final int SH_16 = 65536;
    private static final int SH_24 = 16777216;

    private byte      type;
    private byte      code;
    private byte      flags;
    private int       dataOffset;
    private Vector    groupValue;
    private String    stringValue;
    private int       intValue;
    private byte[]    binaryValue;
    private Hashtable firstFields;

    public byte   getType ()        { return type; }
    public byte   getCode ()        { return code; }

    public Vector getGroupValue ()  { 
	if (type != TYPE_GROUP)
	    throw new RuntimeException ("field is not GROUP type");	
	return groupValue; 
    }

    public String getStringValue () { 
	switch (type) {
	case TYPE_STRING:
	    return stringValue;
	case TYPE_INT:
	    return "" + intValue;
	case TYPE_BINARY:
	    return "" + binaryValue;
	default:
	    return "(TYPE_GROUP)";
	}
    }
    
    public int getIntValue () { 
	if (type != TYPE_INT)
	    throw new RuntimeException ("field is not INT type");	
	return intValue; 
    }

    public byte[] getBinaryValue () {
	if (type != TYPE_BINARY)
	    throw new RuntimeException ("field is not BINARY type");	
	return binaryValue; 
    }	

    public Mus36Field getSubField (int n)  { 
	if (type != TYPE_GROUP)
	    throw new RuntimeException ("field is not GROUP type");
	
	return (Mus36Field) groupValue.elementAt (n); 
    }

    /**
     * get the first field in this group with the supplied code (faster than scanning)
     */
    public Mus36Field getFirstFieldWithCode (byte code) {
	if (type != TYPE_GROUP)
	    throw new RuntimeException ("field is not GROUP type");	

	return (Mus36Field) firstFields.get (new Byte (code));
    }    
    
    /**
     * create a field tree from the binary representation
     */
    public Mus36Field (byte[] rep, boolean flimsy) {	
	// as OM cuts off the first field, re-invent it.
	type   = TYPE_GROUP;
	flags  = 0;
	code   = 0;
	dataOffset = 0;

	groupValue = new Vector ();
	firstFields = new Hashtable ();
	int c = 0;
	int lim = rep.length - 1;
	
	while (c < lim) {
	    int l = getLength (rep, c, flimsy);
	    Mus36Field f = new Mus36Field (rep, c, l, flimsy);
	    groupValue.addElement (f);
	    Byte key = new Byte (f.code);
	    if (! firstFields.containsKey (key)) firstFields.put (key, f);

	    c += l;
	}	    
    }

    private Mus36Field (byte[] rep, int c, int length, boolean flimsy) {
	int off = flimsy ? 0 : 1;
	int lim = c + length - 1;

	//this.length = length;
	type       = rep[c+2+off];
	flags      = rep[c+3+off];
	code       = rep[c+4+off];
	dataOffset = c+5+off;
	
	switch (type) {
	case TYPE_GROUP:
	    groupValue = new Vector ();
	    firstFields = new Hashtable ();
	    int c2 = c + off + 5;
	    int l2 = getLength (rep, c2, flimsy);

	    while (c2 < lim) {
		Mus36Field f = new Mus36Field (rep, c2, l2, flimsy);
		groupValue.addElement (f);
		Byte key = new Byte (f.code);
		if (! firstFields.containsKey (key)) firstFields.put (key, f);

		c2 += l2;
		l2 = getLength (rep, c2, flimsy);
	    }	    
	    break;
	    
	case TYPE_STRING:
	    stringValue = new String (rep, c+off+5, length-(5+off));
	    break;

	case TYPE_INT:
	    int at = c+5+off;
	    intValue =
		(ubv (rep[at++]) * SH_24) +
		(ubv (rep[at++]) * SH_16) +
		(ubv (rep[at++]) * SH_8) +
		(ubv (rep[at]));
	    break;

	case TYPE_BINARY:
	    binaryValue = new byte [length-(5+off)];
	    System.arraycopy (rep, c+off+5, binaryValue, 0, length-(5+off));
	    break;

	default:
	    throw new RuntimeException ("unknown field type " + type);
	}
    }

    // bleah! no "unsigned" though

    private static int ubv (byte b) {
	int r = (int) b;
	return r >= 0 ? r : 256 + r;
    }

    private static int getLength (byte[] rep, int c, boolean flimsy) {
	return ubv(rep[c]) + (ubv(rep[c+1]) * SH_8) + (flimsy ? 0 : (ubv(rep[c+2]) * SH_16));
    }

    public String toString () {
	StringBuffer sb = new StringBuffer ();
	dump (sb, 0);
	return sb.toString ();
    }

    private void dump (StringBuffer sb, int lev) {
	for (int i =0; i < lev; i++)
	    sb.append ("  ");
	
	sb.append ("[code=0x" + Integer.toHexString ((int) code) + ", off=" + dataOffset + "] ");

	switch (type) {
	case TYPE_STRING:
	    sb.append ('"' + stringValue + '"');
	    break;
	case TYPE_INT:
	    sb.append (intValue);
	    break;
	case TYPE_BINARY:
	    for (int i = 0; i < binaryValue.length; i++) {
		sb.append (Integer.toHexString (ubv (binaryValue[i])));
		sb.append (',');
	    }
	    break;
	case TYPE_GROUP:
	    sb.append ("(group)\n");
	    for (Enumeration e = groupValue.elements(); e.hasMoreElements(); )
		((Mus36Field) e.nextElement ()).dump (sb, lev + 1);
	    break;
	default:
	    sb.append ("[unknown field type]");
	}
	
	sb.append ('\n');
    }
}
