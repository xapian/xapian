/* omsettings.h: "global" settings object
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

#ifndef OM_HGUARD_OMSETTINGS_H
#define OM_HGUARD_OMSETTINGS_H

#include <string>
#include <vector>

//////////////////////////////////////////////////////////////////
// OmSettings class
// ================

/** This class is used to pass various settings to other OM classes.
 *
 *  The settings available are:
 *
 *  - backend : database backend type - current backends are: auto, da, db,
 *    inmemory, remote, quartz, and sleepycat
 *
 *  auto pseudo-backend:
 *
 *  - auto_dir : directory for auto pseudo-backend to look for database in
 *
 *  sleepycat backend:
 *
 *  - sleepycat_dir : directory containing sleepycat database
 *
 *  muscat36 backends:
 *
 *  - m36_key_file : keys for DA or DB file
 *  - m36_record_file : DA record file
 *  - m36_term_file : DA term file
 *  - m36_db_file : DB file
 *  - m36_db_cache_size : size of DB file cache in blocks (default 30)
 *  - m36_heavyduty : true for 3 byte offset form, false for older 2 byte form.
 *    Only used by da backend - db backend autodetects. (default true)
 *
 *  remote backend:
 *
 *  - remote_type : "prog" or "tcp"
 *  - remote_program : the name of the program to run to act as a server
 *   			(when remote_type="prog")
 *  - remote_args : the arguments to the program named in remote_program
 *   			(when remote_type="prog")
 *  - remote_server : the name of the host running a tcp server
 *   			(when remote_type="tcp")
 *  - remote_port : the port on which the tcp server is running
 *   			(when remote_type="tcp")
 *  - remote_timeout : The timeout in milliseconds used before assuming that
 *                     the remote server has failed.  If this timeout is
 *                     reached for any operation, then an OmNetworkTimeout
 *                     exception will be thrown.  The default if not
 *                     specified is 10000ms (ie 10 seconds)
 *
 *  quartz backend:
 *
 *  - quartz_dir : directory containing sleepycat database
 *
 *  match options:
 *
 *  - match_collapse_key : key number to collapse on - duplicates mset
 *    entries will be removed based on a key (default -1 => no collapsing)
 *
 *  - match_sort_forward : If true, documents with the same weight will
 *    be returned in ascending document order; if false, they will be
 *    returned in descending order.flag to sort forward (default true)
 *
 *  - match_percent_cutoff : Minimum percentage score for returned
 *    documents. If a document has a lower percentage score than this, it
 *    will not appear in the mset.  If your intention is to return only
 *    matches which contain all the terms in the query, then consider using
 *    OmQuery::OP_AND instead of OmQuery::OP_OR in the query). (default -1 => no
 *    cut-off)
 *
 *  - match_max_or_terms : Maximum number of terms which will be used if
 *    the query contains a large number of terms which are ORed together.
 *    Only the terms with the match_max_or_terms highest termweights will be
 *    used.  Parts of the query which do not involve terms ORed together will
 *    be unaffected by this option.  An example use of this setting is to
 *    set a query which represents a document - only the elite set of terms
 *    which best distinguish that document to be used to find similar
 *    documents, resulting in a performance improvement.
 *    (default 0 => no limit)
 *
 *  expand options:
 * 
 *  - expand_use_query_terms : If false, terms already in the query will be
 *    not be returned in the ESet; if true, they can be. (default false)
 *
 *  - expand_use_exact_termfreq : If true then term frequencies will be
 *    calculated exactly; if true, an approximation may be used which can
 *    greatly improve efficiency. The approximation only applies when
 *    multiple databases are searched together. (default false)
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
	void set(const std::string &key, const std::string &value);

	/** Set an option value.
	 *
	 *  @param key   The name of the option as a char *.
	 *
	 *  @param value The value to set the option to.
	 */
	void set(const string &key, const char *value);
    
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

	/** Set an option value to a vector of strings.
	 *
	 *  @param key   The name of the option as a string.
	 *
	 *  @param begin Iterator pointing to start of vector.
	 *
	 *  @param end   Iterator pointing to end of vector.
	 */
	void set(const string &key, vector<string>::const_iterator begin,
		 vector<string>::const_iterator end);

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
	std::string get(const std::string &key, string def) const;

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

	/** Get a setting value as a vector of strings.
	 *
	 *  @param key	 The key corresponding to the value to retrieve.
	 *
	 *  @exception   OmRangeError will be thrown for an invalid key.
	 */
        vector<string> get_vector(const string &key) const;

	/** Returns a string representing the database group object.
	 *  Introspection method.
	 */
	std::string get_description() const;
};

#endif // OM_HGUARD_OMSETTINGS_H
