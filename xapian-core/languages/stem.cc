/** @file stem.cc
 *  @brief Implementation of Xapian::Stem API class.
 */
/* Copyright (C) 2007,2008,2010,2011,2012,2015 Olly Betts
 * Copyright (C) 2010 Evgeny Sizikov
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <config.h>

#include <xapian/stem.h>

#include <xapian/error.h>

#include "steminternal.h"

#include "keyword.h"
#include "sbl-dispatch.h"

#include <string>

using namespace std;

namespace Xapian {

Stem::Stem(const Stem & o) : internal(o.internal) { }

Stem &
Stem::operator=(const Stem & o)
{
    internal = o.internal;
    return *this;
}

Stem::Stem() { }

Stem::Stem(const std::string &language) {
    int l = keyword(tab, language.data(), language.size());
    if (l >= 0) {
	switch (static_cast<sbl_code>(l)) {
	    case ARABIC:
		internal = new InternalStemArabic;
		return;
	    case ARMENIAN:
		internal = new InternalStemArmenian;
		return;
	    case BASQUE:
		internal = new InternalStemBasque;
		return;
	    case CATALAN:
		internal = new InternalStemCatalan;
		return;
	    case DANISH:
		internal = new InternalStemDanish;
		return;
	    case DUTCH:
		internal = new InternalStemDutch;
		return;
	    case EARLYENGLISH:
		internal = new InternalStemEarlyenglish;
		return;
	    case ENGLISH:
		internal = new InternalStemEnglish;
		return;
	    case FINNISH:
		internal = new InternalStemFinnish;
		return;
	    case FRENCH:
		internal = new InternalStemFrench;
		return;
	    case GERMAN:
		internal = new InternalStemGerman;
		return;
	    case GERMAN2:
		internal = new InternalStemGerman2;
		return;
	    case HUNGARIAN:
		internal = new InternalStemHungarian;
		return;
	    case ITALIAN:
		internal = new InternalStemItalian;
		return;
	    case KRAAIJ_POHLMANN:
		internal = new InternalStemKraaij_pohlmann;
		return;
	    case LOVINS:
		internal = new InternalStemLovins;
		return;
	    case NORWEGIAN:
		internal = new InternalStemNorwegian;
		return;
	    case NONE:
		return;
	    case PORTUGUESE:
		internal = new InternalStemPortuguese;
		return;
	    case PORTER:
		internal = new InternalStemPorter;
		return;
	    case RUSSIAN:
		internal = new InternalStemRussian;
		return;
	    case ROMANIAN:
		internal = new InternalStemRomanian;
		return;
	    case SPANISH:
		internal = new InternalStemSpanish;
		return;
	    case SWEDISH:
		internal = new InternalStemSwedish;
		return;
	    case TURKISH:
		internal = new InternalStemTurkish;
		return;
	}
    }
    if (language.empty())
	return;
    throw Xapian::InvalidArgumentError("Language code " + language + " unknown");
}

Stem::Stem(StemImplementation * p) : internal(p) { }

Stem::~Stem() { }

string
Stem::operator()(const std::string &word) const
{
    if (!internal.get() || word.empty()) return word;
    return internal->operator()(word);
}

string
Stem::get_description() const
{
    string desc = "Xapian::Stem(";
    if (internal.get()) {
	desc += internal->get_description();
	desc += ')';
    } else {
	desc += "none)";
    }
    return desc;
}

}
