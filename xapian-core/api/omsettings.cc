/* omsettings.cc: "global" settings object
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

	/** Return stored settings as a string for use by
	 *  OmSettings::get_description()
	 */
        string OmSettings::Internal::get_description() const;
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

void
OmSettings::set_value(const string &key, const char *value)
{
    internal->set_value(key, value);
}

void
OmSettings::set_value(const string &key, int value)
{
    char buf[64];
    sprintf(buf, "%d", value);
    internal->set_value(key, string(buf));
}

void
OmSettings::set_value(const string &key, double value)
{
    char buf[64];
    sprintf(buf, "%20g", value);
    internal->set_value(key, string(buf));
}

void
OmSettings::set_value(const string &key, bool value)
{
    internal->set_value(key, value ? "1" : "");
}

void
OmSettings::set_value(const string &key, vector<string>::const_iterator begin,
		      vector<string>::const_iterator end)
{
    string s;
    while (true) {
	s += *begin;
	begin++;
	if (begin == end) break;
	s += '\0';
    }
    internal->set_value(key, s);
}

string
OmSettings::get_value(const string &key) const
{
    return internal->get_value(key);
}

bool
OmSettings::get_value_bool(const string &key) const
{
    string s = internal->get_value(key);
    return !(s.empty() || s == "0");
}

int
OmSettings::get_value_int(const string &key) const
{
    string s = internal->get_value(key);
    int res;
    sscanf(s.c_str(), "%d", &res);
    return res;
}

double
OmSettings::get_value_real(const string &key) const
{
    string s = internal->get_value(key);
    double res;
    sscanf(s.c_str(), "%lf", &res);
    return res;
}

string
OmSettings::get_value(const string &key, string def) const
{
    try {
	return internal->get_value(key);
    }
    catch (const OmRangeError &e) {
	return def;
    }
}

bool
OmSettings::get_value_bool(const string &key, bool def) const
{
    try {
	return get_value_bool(key);
    }
    catch (const OmRangeError &e) {
	return def;
    }
}

int
OmSettings::get_value_int(const string &key, int def) const
{
    try {
	return get_value_int(key);
    }
    catch (const OmRangeError &e) {
	return def;
    }
}

double
OmSettings::get_value_real(const string &key, double def) const
{
    try {
	return get_value_real(key);
    }
    catch (const OmRangeError &e) {
	return def;
    }
}

vector<string>
OmSettings::get_value_vector(const string &key) const
{
    string s = internal->get_value(key);
    string::size_type p = 0, q;
    vector<string> v;
    while (1) {	    
	q = s.find('\0', p);
	v.push_back(s.substr(p, q - p));
	if (q == string::npos) break;
	p = q + 1;
    }
    return v;
}

std::string
OmSettings::get_description() const
{
    DEBUGAPICALL("OmSettings::get_description", "");
    /// \todo display all the settings
    std::string description = "OmSettings(";
    description += internal->get_description();
    description += ')';
    DEBUGAPIRETURN(description);
    return description;
}

////////////////////////////////////////////////////////////////
// OmSettings::Internal methods

OmSettings::Internal::Internal()
	: mutex()
{
    data = new OmSettingsData;
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
    // copy on write...
    if (data->ref_count_get() > 1) {
	data = OmRefCntPtr<OmSettingsData>(new OmSettingsData(*data));
    }
    data->values[key] = value;
}

string
OmSettings::Internal::get_value(const string &key) const
{
    OmLockSentry sentry(mutex);

    OmSettingsData::map_type::const_iterator i;
    i = data->values.find(key);

    if (i == data->values.end()) {
	throw OmRangeError(string("Setting ") + key + " doesn't exist.");
    }
    return i->second;
}

string
OmSettings::Internal::get_description() const
{
    std::string description;
    OmSettingsData::map_type::const_iterator i;
    for (i = data->values.begin(); i != data->values.end(); i++) {
	description += "\"" + i->first + "\"->\"" + i->second + "\" ";	
    }
    return description;
}
