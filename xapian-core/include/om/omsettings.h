/* omsettings.h: "global" settings object
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 Dialog Corporation
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

#ifndef OM_HGUARD_OMSETTINGS_H
#define OM_HGUARD_OMSETTINGS_H

//////////////////////////////////////////////////////////////////
// OmSettings class
// ================

/** This class is used to pass various settings to the match
 *  process.  Settings can be set per session.
 */
class OmSettings {
    private:
	class Internal;
	
	/// Internal implementation
	Internal *internal;

    public:
	/** Create a settings object.
	 */
	OmSettings();

	// Maybe add method/constructor to read settings from file?

	/** Copy constructor.
	 *  The copies are reference-counted, so copies are relatively
	 *  cheap.  Modifications to a copy don't affect other existing
	 *  copies (the copy is copy-on-write). */
	OmSettings(const OmSettings &other);
	/** Assignment operator.  This should be cheap. */
	void operator=(const OmSettings &other);

	/** Destructor. */
	~OmSettings();

	/** Set an option value.
	 *  
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set_value(const string &key, const string &value);

	/** Get a setting value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	string get_value(const string &key);
};

#endif // OM_HGUARD_OMSETTINGS_H
