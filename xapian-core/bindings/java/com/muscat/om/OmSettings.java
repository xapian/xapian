/* OmSettings.java: "global" settings object
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */
package com.muscat.om;

public class OmSettings extends OmObject {
    
    public OmSettings () throws OmError { 
	nativePtr = createNativeObject ();
    }
    protected native long createNativeObject ();

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native void set(String key, String value);
    public native void set(String key, int value);
    public native void set(String key, double value);
    public native void set(String key, boolean value);
    public native void set(String key, String[] value);

    public native String get(String key);
    public native String get(String key, String def);
    public native int get_int(String key);
    public native int get_int(String key, int def);
    public native boolean get_bool(String key);
    public native boolean get_bool(String key, boolean def);
    public native double get_real(String key);
    public native double get_real(String key, double def);
    public native String[] get_vector(String key);

    public native String get_description();
};
