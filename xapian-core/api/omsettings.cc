/* omsettings.cc: "global" settings object
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

#include "config.h"
#include "omlocks.h"
#include "omrefcnt.h"
#include "om/omsettings.h"
#include <map>
#include <string>

//////////////////////////////////////////////////////////////////
// OmSettings ref-counted data
// ===========================

class OmSettingsData : public OmRefCntBase {
    public:
	typedef string key_type;
	typedef string value_type;

	typedef map<key_type, value_type> map_type;

	map_type values;
};

//////////////////////////////////////////////////////////////////
// OmSettings::Internal class
// ================
class OmSettings::Internal {
    private:
	/// Mutex used for controlling access to data.
	OmLock mutex;

	/// The actual data (or ref-counted pointer to it)
	OmRefCntPtr<OmSettingsData> data;
    public:
	/** Create a settings internal object.
	 */
	Internal();

	/** Copy constructor.
	 *  The copies are reference-counted, so copies are relatively
	 *  cheap.  Modifications to a copy don't affect other existing
	 *  copies (the copy is copy-on-write). */
	Internal(const Internal &other);
	/** Assignment as with copy.  */
	void operator=(const Internal &other);


	/** Destructor. */
	~Internal();

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
	string get_value(const string &key) const;
};

////////////////////////////////////////////////////////////////
// OmSettings methods

OmSettings::OmSettings()
	: internal(new OmSettings::Internal())
{
}

OmSettings::OmSettings(const OmSettings &other)
	: internal(new OmSettings::Internal(*other.internal))
{
}

void
OmSettings::operator=(const OmSettings &other)
{
    OmSettings temp(other);

    swap(internal, temp.internal);

    // temp object deleted.
}

OmSettings::~OmSettings()
{
    delete internal;
}

void
OmSettings::set_value(const string &key, const string &value)
{
    internal->set_value(key, value);
}

string
OmSettings::get_value(const string &key) const
{
    return internal->get_value(key);
}

////////////////////////////////////////////////////////////////
// OmSettings::Internal methods

OmSettings::Internal::Internal()
	: data(new OmSettingsData)
{
}

OmSettings::Internal::Internal(const OmSettings::Internal &other)
	: mutex(), data(other.data)
{
}

void
OmSettings::Internal::operator=(const OmSettings::Internal &other)
{
    OmLockSentry sentry(mutex);
    OmSettings::Internal temp(other);
    swap(data, temp.data);
}

OmSettings::Internal::~Internal()
{
}

void
OmSettings::Internal::set_value(const string &key, const string &value)
{
    OmLockSentry sentry(mutex);
    data->values[key] = value;
}

string
OmSettings::Internal::get_value(const string &key) const
{
    OmLockSentry sentry(mutex);

    OmSettingsData::map_type::const_iterator i;
    i = data->values.find(key);

    if (i == data->values.end()) {
	throw OmRangeError(string("Key ") + key + " doesn't exist.");
    }
    return i->second;
}
