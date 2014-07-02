/** @file arabic-normalizer-constants.h
 * @brief Contains the constants used in Arabic Normalizer like letters,
 * letter groupings, romanisation mappings
 */
/* Copyright (C) 2014 Assem Chelli
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_ARABICNORMALIZERCONSTANTS_H
#define XAPIAN_INCLUDED_ARABICNORMALIZERCONSTANTS_H

#include <cstring>
#include <map>

// Specific punctuation marks
#define ARABIC_COMMA 0x060C
#define ARABIC_SEMICOLON 0x061B
#define ARABIC_QUESTION 0x061F

// Hamza forms
#define ARABIC_HAMZA 0x0621
#define ARABIC_ALEF_MADDA 0x0622
#define ARABIC_ALEF_HAMZA_ABOVE 0x0623
#define ARABIC_WAW_HAMZA 0x0624
#define ARABIC_ALEF_HAMZA_BELOW 0x0625
#define ARABIC_YEH_HAMZA 0x0626

// Letters
#define ARABIC_ALEF 0x0627
#define ARABIC_BEH 0x0628
#define ARABIC_TEH_MARBUTA 0x0629
#define ARABIC_TEH 0x062a
#define ARABIC_THEH 0x062b
#define ARABIC_JEEM 0x062c
#define ARABIC_HAH 0x062d
#define ARABIC_KHAH 0x062e
#define ARABIC_DAL 0x062f
#define ARABIC_THAL 0x0630
#define ARABIC_REH 0x0631
#define ARABIC_ZAIN 0x0632
#define ARABIC_SEEN 0x0633
#define ARABIC_SHEEN 0x0634
#define ARABIC_SAD 0x0635
#define ARABIC_DAD 0x0636
#define ARABIC_TAH 0x0637
#define ARABIC_ZAH 0x0638
#define ARABIC_AIN 0x0639
#define ARABIC_GHAIN 0x063a
#define ARABIC_TATWEEL 0x0640
#define ARABIC_FEH 0x0641
#define ARABIC_QAF 0x0642
#define ARABIC_KAF 0x0643
#define ARABIC_LAM 0x0644
#define ARABIC_MEEM 0x0645
#define ARABIC_NOON 0x0646
#define ARABIC_HEH 0x0647
#define ARABIC_WAW 0x0648
#define ARABIC_ALEF_MAKSURA 0x0649
#define ARABIC_YEH 0x064a
#define ARABIC_MADDA_ABOVE 0x0653
#define ARABIC_HAMZA_ABOVE 0x0654
#define ARABIC_HAMZA_BELOW 0x0655

// Hindu–Arabic numerals
#define ARABIC_ZERO 0x0660
#define ARABIC_ONE 0x0661
#define ARABIC_TWO 0x0662
#define ARABIC_THREE 0x0663
#define ARABIC_FOUR 0x0664
#define ARABIC_FIVE 0x0665
#define ARABIC_SIX 0x0666
#define ARABIC_SEVEN 0x0667
#define ARABIC_EIGHT 0x0668
#define ARABIC_NINE 0x0669
#define ARABIC_PERCENT 0x066a
#define ARABIC_DECIMAL 0x066b
#define ARABIC_THOUSANDS 0x066c

#define ARABIC_STAR 0x066d
#define ARABIC_MINI_ALEF 0x0670
#define ARABIC_ALEF_WASLA 0x0671
#define ARABIC_FULL_STOP 0x06d4
#define ARABIC_BYTE_ORDER_MARK 0xfeff

// Diacritics
#define ARABIC_FATHATAN 0x064b
#define ARABIC_DAMMATAN 0x064c
#define ARABIC_KASRATAN 0x064d
#define ARABIC_FATHA 0x064e
#define ARABIC_DAMMA 0x064f
#define ARABIC_KASRA 0x0650
#define ARABIC_SHADDA 0x0651
#define ARABIC_SUKUN 0x0652

// Small Letters
#define ARABIC_SMALL_ALEF 0x0670
#define ARABIC_SMALL_WAW 0x06E5
#define ARABIC_SMALL_YEH 0x06E6

// Ligatures
#define ARABIC_LAM_ALEF 0xfefb
#define ARABIC_LAM_ALEF_HAMZA_ABOVE 0xfef7
#define ARABIC_LAM_ALEF_HAMZA_BELOW 0xfef9
#define ARABIC_LAM_ALEF_MADDA_ABOVE 0xfef5


// groups
static const unsigned ARABIC_LETTERS[] = {
    ARABIC_ALEF,
    ARABIC_BEH,
    ARABIC_TEH,
    ARABIC_TEH_MARBUTA,
    ARABIC_THEH,
    ARABIC_JEEM,
    ARABIC_HAH,
    ARABIC_KHAH,
    ARABIC_DAL,
    ARABIC_THAL,
    ARABIC_REH,
    ARABIC_ZAIN,
    ARABIC_SEEN,
    ARABIC_SHEEN,
    ARABIC_SAD,
    ARABIC_DAD,
    ARABIC_TAH,
    ARABIC_ZAH,
    ARABIC_AIN,
    ARABIC_GHAIN,
    ARABIC_FEH,
    ARABIC_QAF,
    ARABIC_KAF,
    ARABIC_LAM,
    ARABIC_MEEM,
    ARABIC_NOON,
    ARABIC_HEH,
    ARABIC_WAW,
    ARABIC_YEH,
    ARABIC_HAMZA,
    ARABIC_ALEF_MADDA,
    ARABIC_ALEF_HAMZA_ABOVE,
    ARABIC_WAW_HAMZA,
    ARABIC_ALEF_HAMZA_BELOW,
    ARABIC_YEH_HAMZA
};

static const unsigned ARABIC_TASHKEEL[] = {
    ARABIC_FATHATAN,
    ARABIC_DAMMATAN,
    ARABIC_KASRATAN,
    ARABIC_FATHA,
    ARABIC_DAMMA,
    ARABIC_KASRA,
    ARABIC_SUKUN,
    ARABIC_SHADDA,
};

static const unsigned ARABIC_LIGUATURES[] = {
    ARABIC_LAM_ALEF,
    ARABIC_LAM_ALEF_HAMZA_ABOVE,
    ARABIC_LAM_ALEF_HAMZA_BELOW,
    ARABIC_LAM_ALEF_MADDA_ABOVE
};

static const unsigned ARABIC_HAMZAT[] = {
    ARABIC_HAMZA,
    ARABIC_WAW_HAMZA,
    ARABIC_YEH_HAMZA,
    ARABIC_HAMZA_ABOVE,
    ARABIC_HAMZA_BELOW,
    ARABIC_ALEF_HAMZA_BELOW,
    ARABIC_ALEF_HAMZA_ABOVE
};

static const unsigned ARABIC_ALEFAT[] = {
    ARABIC_ALEF,
    ARABIC_ALEF_MADDA,
    ARABIC_SMALL_ALEF,
    ARABIC_ALEF_WASLA,
    ARABIC_ALEF_MAKSURA
};

static const unsigned ARABIC_YAHLIKE[] = {
    ARABIC_YEH,
    ARABIC_YEH_HAMZA,
    ARABIC_ALEF_MAKSURA,
    ARABIC_SMALL_YEH
};

static const unsigned ARABIC_TEHLIKE[] = {
    ARABIC_TEH,
    ARABIC_TEH_MARBUTA
};

static const unsigned ARABIC_WAWLIKE[] = {
    ARABIC_WAW,
    ARABIC_WAW_HAMZA,
    ARABIC_SMALL_WAW
};

// orders
static const unsigned ARABIC_ALPHABETIC_ORDER[28] = {
    ARABIC_ALEF, // 1
    ARABIC_BEH, // 2
    ARABIC_TEH, // 3
    ARABIC_THEH, // 4
    ARABIC_JEEM, // 5
    ARABIC_HAH, // 6
    ARABIC_KHAH, // 7
    ARABIC_DAL, // 8
    ARABIC_THAL, // 9
    ARABIC_REH, // 10
    ARABIC_ZAIN, // 11
    ARABIC_SEEN, // 12
    ARABIC_SHEEN, // 13
    ARABIC_SAD, // 14
    ARABIC_DAD, // 15
    ARABIC_TAH, // 16
    ARABIC_ZAH, // 17
    ARABIC_AIN, // 18
    ARABIC_GHAIN, // 19
    ARABIC_FEH, // 20
    ARABIC_QAF, // 21
    ARABIC_KAF, // 22
    ARABIC_LAM, // 23
    ARABIC_MEEM, // 24
    ARABIC_NOON, // 25
    ARABIC_HEH, // 26
    ARABIC_WAW, // 27
    ARABIC_YEH // 28
};

std::pair<unsigned, unsigned> shaping_map_data[] = {
    std::make_pair(0xfe83,  ARABIC_ALEF_HAMZA_ABOVE),
    std::make_pair(0xfe87, ARABIC_ALEF_HAMZA_BELOW),
    std::make_pair(0xfe8b, ARABIC_YEH_HAMZA),
    std::make_pair(0xfe8f, ARABIC_BEH),
    std::make_pair(0xfe93, ARABIC_TEH_MARBUTA),
    std::make_pair(0xfe97, ARABIC_TEH),
    std::make_pair(0xfe9b, ARABIC_THEH),
    std::make_pair(0xfe9f, ARABIC_JEEM),
    std::make_pair(0xfea3, ARABIC_HAH),
    std::make_pair(0xfea7, ARABIC_KHAH),
    std::make_pair(0xfeab, ARABIC_THAL),
    std::make_pair(0xfeaf, ARABIC_ZAIN),
    std::make_pair(0xfeb3, ARABIC_SEEN),
    std::make_pair(0xfeb7, ARABIC_SHEEN),
    std::make_pair(0xfebb, ARABIC_SAD),
    std::make_pair(0xfebf, ARABIC_DAD),
    std::make_pair(0xfec3, ARABIC_TAH),
    std::make_pair(0xfec7, ARABIC_ZAH),
    std::make_pair(0xfecb, ARABIC_AIN),
    std::make_pair(0xfecf, ARABIC_GHAIN),
    std::make_pair(0xfed3, ARABIC_FEH),
    std::make_pair(0xfed7, ARABIC_QAF),
    std::make_pair(0xfedb, ARABIC_KAF),
    std::make_pair(0xfedf, ARABIC_LAM),
    std::make_pair(0xfee3, ARABIC_MEEM),
    std::make_pair(0xfee7, ARABIC_NOON),
    std::make_pair(0xfeeb, ARABIC_HEH),
    std::make_pair(0xfeef, ARABIC_ALEF_MAKSURA),
    std::make_pair(0xfef3, ARABIC_YEH),
    std::make_pair(0xfe80, ARABIC_HAMZA),
    std::make_pair(0xfe84, ARABIC_ALEF_HAMZA_ABOVE),
    std::make_pair(0xfe88, ARABIC_ALEF_HAMZA_BELOW),
    std::make_pair(0xfe8c, ARABIC_YEH_HAMZA),
    std::make_pair(0xfe90, ARABIC_BEH),
    std::make_pair(0xfe94, ARABIC_TEH_MARBUTA),
    std::make_pair(0xfe98, ARABIC_TEH),
    std::make_pair(0xfe9c, ARABIC_THEH),
    std::make_pair(0xfea0, ARABIC_JEEM),
    std::make_pair(0xfea4, ARABIC_HAH),
    std::make_pair(0xfea8, ARABIC_KHAH),
    std::make_pair(0xfeac, ARABIC_THAL),
    std::make_pair(0xfeb0, ARABIC_ZAIN),
    std::make_pair(0xfeb4, ARABIC_SEEN),
    std::make_pair(0xfeb8, ARABIC_SHEEN),
    std::make_pair(0xfebc, ARABIC_SAD),
    std::make_pair(0xfec0, ARABIC_DAD),
    std::make_pair(0xfec4, ARABIC_TAH),
    std::make_pair(0xfec8, ARABIC_ZAH),
    std::make_pair(0xfecc, ARABIC_AIN),
    std::make_pair(0xfed0, ARABIC_GHAIN),
    std::make_pair(0xfed4, ARABIC_FEH),
    std::make_pair(0xfed8, ARABIC_QAF),
    std::make_pair(0xfedc, ARABIC_KAF),
    std::make_pair(0xfee0, ARABIC_LAM),
    std::make_pair(0xfee4, ARABIC_MEEM),
    std::make_pair(0xfee8, ARABIC_NOON),
    std::make_pair(0xfeec, ARABIC_HEH),
    std::make_pair(0xfef0, ARABIC_ALEF_MAKSURA),
    std::make_pair(0xfef4, ARABIC_YEH),
    std::make_pair(0xfe81, ARABIC_ALEF_MADDA),
    std::make_pair(0xfe85, ARABIC_WAW_HAMZA),
    std::make_pair(0xfe89, ARABIC_YEH_HAMZA),
    std::make_pair(0xfe8d, ARABIC_ALEF),
    std::make_pair(0xfe91, ARABIC_BEH),
    std::make_pair(0xfe95, ARABIC_TEH),
    std::make_pair(0xfe99, ARABIC_THEH),
    std::make_pair(0xfe9d, ARABIC_JEEM),
    std::make_pair(0xfea1, ARABIC_HAH),
    std::make_pair(0xfea5, ARABIC_KHAH),
    std::make_pair(0xfea9, ARABIC_DAL),
    std::make_pair(0xfead, ARABIC_REH),
    std::make_pair(0xfeb1, ARABIC_SEEN),
    std::make_pair(0xfeb5, ARABIC_SHEEN),
    std::make_pair(0xfeb9, ARABIC_SAD),
    std::make_pair(0xfebd, ARABIC_DAD),
    std::make_pair(0xfec1, ARABIC_TAH),
    std::make_pair(0xfec5, ARABIC_ZAH),
    std::make_pair(0xfec9, ARABIC_AIN),
    std::make_pair(0xfecd, ARABIC_GHAIN),
    std::make_pair(0xfed1, ARABIC_FEH),
    std::make_pair(0xfed5, ARABIC_QAF),
    std::make_pair(0xfed9, ARABIC_KAF),
    std::make_pair(0xfedd, ARABIC_LAM),
    std::make_pair(0xfee1, ARABIC_MEEM),
    std::make_pair(0xfee5, ARABIC_NOON),
    std::make_pair(0xfee9, ARABIC_HEH),
    std::make_pair(0xfeed, ARABIC_WAW),
    std::make_pair(0xfef1, ARABIC_YEH),
    std::make_pair(0xfe82, ARABIC_ALEF_MADDA),
    std::make_pair(0xfe86, ARABIC_WAW_HAMZA),
    std::make_pair(0xfe8a, ARABIC_YEH_HAMZA),
    std::make_pair(0xfe8e, ARABIC_ALEF),
    std::make_pair(0xfe92, ARABIC_BEH),
    std::make_pair(0xfe96, ARABIC_TEH),
    std::make_pair(0xfe9a, ARABIC_THEH),
    std::make_pair(0xfe9e, ARABIC_JEEM),
    std::make_pair(0xfea2, ARABIC_HAH),
    std::make_pair(0xfea6, ARABIC_KHAH),
    std::make_pair(0xfeaa, ARABIC_DAL),
    std::make_pair(0xfeae, ARABIC_REH),
    std::make_pair(0xfeb2, ARABIC_SEEN),
    std::make_pair(0xfeb6, ARABIC_SHEEN),
    std::make_pair(0xfeba, ARABIC_SAD),
    std::make_pair(0xfebe, ARABIC_DAD),
    std::make_pair(0xfec2, ARABIC_TAH),
    std::make_pair(0xfec6, ARABIC_ZAH),
    std::make_pair(0xfeca, ARABIC_AIN),
    std::make_pair(0xfece, ARABIC_GHAIN),
    std::make_pair(0xfed2, ARABIC_FEH),
    std::make_pair(0xfed6, ARABIC_QAF),
    std::make_pair(0xfeda, ARABIC_KAF),
    std::make_pair(0xfede, ARABIC_LAM),
    std::make_pair(0xfee2, ARABIC_MEEM),
    std::make_pair(0xfee6, ARABIC_NOON),
    std::make_pair(0xfeea, ARABIC_HEH),
    std::make_pair(0xfeee, ARABIC_WAW),
    std::make_pair(0xfef2, ARABIC_YEH)
};

std::map<unsigned, unsigned> ARABIC_SHAPING_MAP(shaping_map_data, shaping_map_data + sizeof shaping_map_data / sizeof shaping_map_data[0]);


// Buckwalter Romanization Mapping
std::pair<unsigned, unsigned> buckwalter_to_unicode_map_data[] = {
    std::make_pair('\'',  ARABIC_HAMZA),
    std::make_pair('|',  ARABIC_ALEF_MADDA),
    std::make_pair('>',  ARABIC_ALEF_HAMZA_ABOVE),
    std::make_pair('&',  ARABIC_WAW_HAMZA),
    std::make_pair('<',  ARABIC_ALEF_HAMZA_BELOW),
    std::make_pair('}',  ARABIC_YEH_HAMZA),
    std::make_pair('A',  ARABIC_ALEF),
    std::make_pair('b',  ARABIC_BEH),
    std::make_pair('p',  ARABIC_TEH_MARBUTA),
    std::make_pair('t',  ARABIC_TEH),
    std::make_pair('v',  ARABIC_THEH),
    std::make_pair('j',  ARABIC_JEEM),
    std::make_pair('H',  ARABIC_HAH),
    std::make_pair('x',  ARABIC_KHAH),
    std::make_pair('d',  ARABIC_DAL),
    std::make_pair('*',  ARABIC_THAL),
    std::make_pair('r',  ARABIC_REH),
    std::make_pair('z',  ARABIC_ZAIN),
    std::make_pair('s',  ARABIC_SEEN),
    std::make_pair('$',  ARABIC_SHEEN),
    std::make_pair('S',  ARABIC_SAD),
    std::make_pair('D',  ARABIC_DAD),
    std::make_pair('T',  ARABIC_TAH),
    std::make_pair('Z',  ARABIC_ZAH),
    std::make_pair('E',  ARABIC_AIN),
    std::make_pair('g',  ARABIC_GHAIN),
    std::make_pair('_',  ARABIC_TATWEEL),
    std::make_pair('f',  ARABIC_FEH),
    std::make_pair('q',  ARABIC_QAF),
    std::make_pair('k',  ARABIC_KAF),
    std::make_pair('l',  ARABIC_LAM),
    std::make_pair('m',  ARABIC_MEEM),
    std::make_pair('n',  ARABIC_NOON),
    std::make_pair('h',  ARABIC_HEH),
    std::make_pair('w',  ARABIC_WAW),
    std::make_pair('Y',  ARABIC_ALEF_MAKSURA),
    std::make_pair('y',  ARABIC_YEH),
    std::make_pair('F',  ARABIC_FATHATAN),
    std::make_pair('N',  ARABIC_DAMMATAN),
    std::make_pair('K',  ARABIC_KASRATAN),
    std::make_pair('a',  ARABIC_FATHA),
    std::make_pair('u',  ARABIC_DAMMA),
    std::make_pair('i',  ARABIC_KASRA),
    std::make_pair('~',  ARABIC_SHADDA),
    std::make_pair('o',  ARABIC_SUKUN),
    std::make_pair('`',  ARABIC_MINI_ALEF),
    std::make_pair('{',  ARABIC_ALEF_WASLA),
};

std::map<unsigned, unsigned> BUCKWALTER_TO_ARABIC(buckwalter_to_unicode_map_data, buckwalter_to_unicode_map_data + sizeof buckwalter_to_unicode_map_data / sizeof buckwalter_to_unicode_map_data[0]);

// ISO233-2 romanisation letter mapping
std::pair<unsigned, unsigned> iso233_to_unicode_map_data[] = {
    std::make_pair( 0x02CC,  ARABIC_HAMZA), // ˌ
    std::make_pair('|',  ARABIC_ALEF_MADDA),
    std::make_pair( 0x02C8,  ARABIC_ALEF_HAMZA_ABOVE), // ˈ
    std::make_pair('<',  ARABIC_ALEF_HAMZA_BELOW),
    std::make_pair( 0x02BE,  ARABIC_ALEF), // ʾ
    std::make_pair('b',  ARABIC_BEH),
    std::make_pair( 0x1E97,  ARABIC_TEH_MARBUTA), // ẗ
    std::make_pair('t',  ARABIC_TEH),
    std::make_pair( 0x1E6F,  ARABIC_THEH), // ṯ
    std::make_pair( 0x01E7,  ARABIC_JEEM), // ǧ
    std::make_pair( 0x1E25,  ARABIC_HAH), // ḥ
    std::make_pair( 0x1E96,  ARABIC_KHAH), // ẖ
    std::make_pair('d',  ARABIC_DAL),
    std::make_pair( 0x1E0F,  ARABIC_THAL), // ḏ
    std::make_pair('r',  ARABIC_REH),
    std::make_pair('z',  ARABIC_ZAIN),
    std::make_pair('s',  ARABIC_SEEN),
    std::make_pair( 0x0161,  ARABIC_SHEEN), // š
    std::make_pair( 0x1E63,  ARABIC_SAD), // ṣ
    std::make_pair( 0x1E0D,  ARABIC_DAD), // ḍ
    std::make_pair( 0x1E6D,  ARABIC_TAH), // ṭ
    std::make_pair( 0x1E93,  ARABIC_ZAH), // ẓ
    std::make_pair( 0x02BF,  ARABIC_AIN), // ʿ
    std::make_pair( 0x0121,  ARABIC_GHAIN), // ġ
    std::make_pair('_',  ARABIC_TATWEEL),
    std::make_pair('f',  ARABIC_FEH),
    std::make_pair('q',  ARABIC_QAF),
    std::make_pair('k',  ARABIC_KAF),
    std::make_pair('l',  ARABIC_LAM),
    std::make_pair('m',  ARABIC_MEEM),
    std::make_pair('n',  ARABIC_NOON),
    std::make_pair('h',  ARABIC_HEH),
    std::make_pair('w',  ARABIC_WAW),
    std::make_pair( 0x1EF3,  ARABIC_ALEF_MAKSURA), // ỳ
    std::make_pair('y',  ARABIC_YEH),
    std::make_pair( 0x00E1,  ARABIC_FATHATAN), // á
    std::make_pair( 0x00FA,  ARABIC_DAMMATAN), // ú
    std::make_pair( 0x00ED,  ARABIC_KASRATAN), // í
    std::make_pair('a',  ARABIC_FATHA),
    std::make_pair('u',  ARABIC_DAMMA),
    std::make_pair('i',  ARABIC_KASRA),
    std::make_pair('~',  ARABIC_SHADDA),
    std::make_pair( 0x00B0,  ARABIC_SUKUN), // °
    std::make_pair('`',  ARABIC_MINI_ALEF),
    std::make_pair('{',  ARABIC_ALEF_WASLA),
    // extended here
    std::make_pair('^',  ARABIC_MADDA_ABOVE),
    std::make_pair('#',  ARABIC_HAMZA_ABOVE)
};

std::map<unsigned, unsigned> ISO233_TO_ARABIC(iso233_to_unicode_map_data, iso233_to_unicode_map_data + sizeof iso233_to_unicode_map_data / sizeof buckwalter_to_unicode_map_data[0]);

#endif // XAPIAN_INCLUDED_ARABICNORMALIZERCONSTANTS_H
