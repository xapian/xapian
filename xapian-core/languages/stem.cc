/** @file
 *  @brief Implementation of Xapian::Stem API class.
 */
/* Copyright (C) 2007,2008,2010,2011,2012,2015,2018,2019 Olly Betts
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

#include "allsnowballheaders.h"
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

Stem::Stem(Stem &&) = default;

Stem &
Stem::operator=(Stem &&) = default;

Stem::Stem() { }

static StemImplementation*
stem_internal_factory(const std::string& language, bool fallback)
{
    int l = keyword2(tab, language.data(), language.size());
    if (l >= 0) {
	switch (static_cast<sbl_code>(l)) {
	    case ARABIC:
		return new InternalStemArabic;
	    case ARMENIAN:
		return new InternalStemArmenian;
	    case BASQUE:
		return new InternalStemBasque;
	    case CATALAN:
		return new InternalStemCatalan;
	    case DANISH:
		return new InternalStemDanish;
	    case DUTCH:
		return new InternalStemDutch;
	    case EARLYENGLISH:
		return new InternalStemEarlyenglish;
	    case ENGLISH:
		return new InternalStemEnglish;
	    case FINNISH:
		return new InternalStemFinnish;
	    case FRENCH:
		return new InternalStemFrench;
	    case GERMAN:
		return new InternalStemGerman;
	    case GERMAN2:
		return new InternalStemGerman2;
	    case HUNGARIAN:
		return new InternalStemHungarian;
	    case INDONESIAN:
		return new InternalStemIndonesian;
	    case IRISH:
		return new InternalStemIrish;
	    case ITALIAN:
		return new InternalStemItalian;
	    case KRAAIJ_POHLMANN:
		return new InternalStemKraaij_pohlmann;
	    case LITHUANIAN:
		return new InternalStemLithuanian;
	    case LOVINS:
		return new InternalStemLovins;
	    case NEPALI:
		return new InternalStemNepali;
	    case NORWEGIAN:
		return new InternalStemNorwegian;
	    case NONE:
		return NULL;
	    case PORTUGUESE:
		return new InternalStemPortuguese;
	    case PORTER:
		return new InternalStemPorter;
	    case RUSSIAN:
		return new InternalStemRussian;
	    case ROMANIAN:
		return new InternalStemRomanian;
	    case SPANISH:
		return new InternalStemSpanish;
	    case SWEDISH:
		return new InternalStemSwedish;
	    case TAMIL:
		return new InternalStemTamil;
	    case TURKISH:
		return new InternalStemTurkish;
	}
    }
    if (fallback || language.empty())
	return NULL;
    throw Xapian::InvalidArgumentError("Language code " + language + " unknown");
}

Stem::Stem(const std::string& language)
    : internal(stem_internal_factory(language, false)) { }

Stem::Stem(const std::string& language, bool fallback)
    : internal(stem_internal_factory(language, fallback)) { }

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
