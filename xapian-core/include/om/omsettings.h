/* omsettings.h: "global" settings object
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#include <string>

//////////////////////////////////////////////////////////////////
// OmSettings class
// ================

/** This class is used to pass various settings to other OM classes.
 *
 *  The settings available are listed in the documentation.
 */
class OmSettings {
    public:
	class Internal;
	/// Internal implementation
	Internal *internal;

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
	 *  @param value The value to set the option to, as a string.
	 */
	void set(const std::string &key, const std::string &value);

	/** Set an option value.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to, as a char *.
	 */
	void set(const std::string &key, const char *value);
    
	/** Set an option value to an integer.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, int value);

	/** Set an option value to a real number.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, double value);

	/** Set an option value to a boolean.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const std::string &key, bool value);

	/** Get a setting value as a string.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	std::string get(const std::string &key) const;

	/** Get a setting value as a string, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	std::string get(const std::string &key, std::string def) const;

	/** Get a setting value as an integer.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	int get_int(const std::string &key) const;

	/** Get a setting value as an integer, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	int get_int(const std::string &key, int def) const;

	/** Get a setting value as a boolean.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	bool get_bool(const std::string &key) const;

	/** Get a setting value as a boolean, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	bool get_bool(const std::string &key, bool def) const;

	/** Get a setting value as an real number.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
	double get_real(const std::string &key) const;

	/** Get a setting value as an real number, with default value.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 */
	double get_real(const std::string &key, double def) const;

	/** Returns a string representing the database group object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif // OM_HGUARD_OMSETTINGS_H
