package com.muscat.om;

public class OmKey {
    public OmKey (String value) { this.value = value; }
    public String value;
    public boolean lessThan (OmKey k) { return value.compareTo (k.value) < 0; }
}
