package com.muscat.om;

public class OmData {
    OmData (byte[] value) { this.value = value; }
    public byte[] value;
    public String toString () {
	return new String (value);
    }
}

