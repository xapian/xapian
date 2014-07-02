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


struct character_map {
    unsigned key, value;
};

static const character_map ARABIC_SHAPING_MAP[] = {
    { 0xfe83, ARABIC_ALEF_HAMZA_ABOVE },
    { 0xfe87, ARABIC_ALEF_HAMZA_BELOW },
    { 0xfe8b, ARABIC_YEH_HAMZA },
    { 0xfe8f, ARABIC_BEH },
    { 0xfe93, ARABIC_TEH_MARBUTA },
    { 0xfe97, ARABIC_TEH },
    { 0xfe9b, ARABIC_THEH },
    { 0xfe9f, ARABIC_JEEM },
    { 0xfea3, ARABIC_HAH },
    { 0xfea7, ARABIC_KHAH },
    { 0xfeab, ARABIC_THAL },
    { 0xfeaf, ARABIC_ZAIN },
    { 0xfeb3, ARABIC_SEEN },
    { 0xfeb7, ARABIC_SHEEN },
    { 0xfebb, ARABIC_SAD },
    { 0xfebf, ARABIC_DAD },
    { 0xfec3, ARABIC_TAH },
    { 0xfec7, ARABIC_ZAH },
    { 0xfecb, ARABIC_AIN },
    { 0xfecf, ARABIC_GHAIN },
    { 0xfed3, ARABIC_FEH },
    { 0xfed7, ARABIC_QAF },
    { 0xfedb, ARABIC_KAF },
    { 0xfedf, ARABIC_LAM },
    { 0xfee3, ARABIC_MEEM },
    { 0xfee7, ARABIC_NOON },
    { 0xfeeb, ARABIC_HEH },
    { 0xfeef, ARABIC_ALEF_MAKSURA },
    { 0xfef3, ARABIC_YEH },
    { 0xfe80, ARABIC_HAMZA },
    { 0xfe84, ARABIC_ALEF_HAMZA_ABOVE },
    { 0xfe88, ARABIC_ALEF_HAMZA_BELOW },
    { 0xfe8c, ARABIC_YEH_HAMZA },
    { 0xfe90, ARABIC_BEH },
    { 0xfe94, ARABIC_TEH_MARBUTA },
    { 0xfe98, ARABIC_TEH },
    { 0xfe9c, ARABIC_THEH },
    { 0xfea0, ARABIC_JEEM },
    { 0xfea4, ARABIC_HAH },
    { 0xfea8, ARABIC_KHAH },
    { 0xfeac, ARABIC_THAL },
    { 0xfeb0, ARABIC_ZAIN },
    { 0xfeb4, ARABIC_SEEN },
    { 0xfeb8, ARABIC_SHEEN },
    { 0xfebc, ARABIC_SAD },
    { 0xfec0, ARABIC_DAD },
    { 0xfec4, ARABIC_TAH },
    { 0xfec8, ARABIC_ZAH },
    { 0xfecc, ARABIC_AIN },
    { 0xfed0, ARABIC_GHAIN },
    { 0xfed4, ARABIC_FEH },
    { 0xfed8, ARABIC_QAF },
    { 0xfedc, ARABIC_KAF },
    { 0xfee0, ARABIC_LAM },
    { 0xfee4, ARABIC_MEEM },
    { 0xfee8, ARABIC_NOON },
    { 0xfeec, ARABIC_HEH },
    { 0xfef0, ARABIC_ALEF_MAKSURA },
    { 0xfef4, ARABIC_YEH },
    { 0xfe81, ARABIC_ALEF_MADDA },
    { 0xfe85, ARABIC_WAW_HAMZA },
    { 0xfe89, ARABIC_YEH_HAMZA },
    { 0xfe8d, ARABIC_ALEF },
    { 0xfe91, ARABIC_BEH },
    { 0xfe95, ARABIC_TEH },
    { 0xfe99, ARABIC_THEH },
    { 0xfe9d, ARABIC_JEEM },
    { 0xfea1, ARABIC_HAH },
    { 0xfea5, ARABIC_KHAH },
    { 0xfea9, ARABIC_DAL },
    { 0xfead, ARABIC_REH },
    { 0xfeb1, ARABIC_SEEN },
    { 0xfeb5, ARABIC_SHEEN },
    { 0xfeb9, ARABIC_SAD },
    { 0xfebd, ARABIC_DAD },
    { 0xfec1, ARABIC_TAH },
    { 0xfec5, ARABIC_ZAH },
    { 0xfec9, ARABIC_AIN },
    { 0xfecd, ARABIC_GHAIN },
    { 0xfed1, ARABIC_FEH },
    { 0xfed5, ARABIC_QAF },
    { 0xfed9, ARABIC_KAF },
    { 0xfedd, ARABIC_LAM },
    { 0xfee1, ARABIC_MEEM },
    { 0xfee5, ARABIC_NOON },
    { 0xfee9, ARABIC_HEH },
    { 0xfeed, ARABIC_WAW },
    { 0xfef1, ARABIC_YEH },
    { 0xfe82, ARABIC_ALEF_MADDA },
    { 0xfe86, ARABIC_WAW_HAMZA },
    { 0xfe8a, ARABIC_YEH_HAMZA },
    { 0xfe8e, ARABIC_ALEF },
    { 0xfe92, ARABIC_BEH },
    { 0xfe96, ARABIC_TEH },
    { 0xfe9a, ARABIC_THEH },
    { 0xfe9e, ARABIC_JEEM },
    { 0xfea2, ARABIC_HAH },
    { 0xfea6, ARABIC_KHAH },
    { 0xfeaa, ARABIC_DAL },
    { 0xfeae, ARABIC_REH },
    { 0xfeb2, ARABIC_SEEN },
    { 0xfeb6, ARABIC_SHEEN },
    { 0xfeba, ARABIC_SAD },
    { 0xfebe, ARABIC_DAD },
    { 0xfec2, ARABIC_TAH },
    { 0xfec6, ARABIC_ZAH },
    { 0xfeca, ARABIC_AIN },
    { 0xfece, ARABIC_GHAIN },
    { 0xfed2, ARABIC_FEH },
    { 0xfed6, ARABIC_QAF },
    { 0xfeda, ARABIC_KAF },
    { 0xfede, ARABIC_LAM },
    { 0xfee2, ARABIC_MEEM },
    { 0xfee6, ARABIC_NOON },
    { 0xfeea, ARABIC_HEH },
    { 0xfeee, ARABIC_WAW },
    { 0xfef2, ARABIC_YEH }
};

// Buckwalter Romanization Mapping
static const character_map BUCKWALTER_TO_ARABIC[] = {
        { '\'', ARABIC_HAMZA },
         { '|', ARABIC_ALEF_MADDA },
         { '>', ARABIC_ALEF_HAMZA_ABOVE },
         { '&', ARABIC_WAW_HAMZA },
         { '<', ARABIC_ALEF_HAMZA_BELOW },
         { '}', ARABIC_YEH_HAMZA },
         { 'A', ARABIC_ALEF },
         { 'b', ARABIC_BEH },
         { 'p', ARABIC_TEH_MARBUTA },
         { 't', ARABIC_TEH },
         { 'v', ARABIC_THEH },
         { 'j', ARABIC_JEEM },
         { 'H', ARABIC_HAH },
         { 'x', ARABIC_KHAH },
         { 'd', ARABIC_DAL },
         { '*', ARABIC_THAL },
         { 'r', ARABIC_REH },
         { 'z', ARABIC_ZAIN },
         { 's', ARABIC_SEEN },
         { '$', ARABIC_SHEEN },
         { 'S', ARABIC_SAD },
         { 'D', ARABIC_DAD },
         { 'T', ARABIC_TAH },
         { 'Z', ARABIC_ZAH },
         { 'E', ARABIC_AIN },
         { 'g', ARABIC_GHAIN },
         { '_', ARABIC_TATWEEL },
         { 'f', ARABIC_FEH },
         { 'q', ARABIC_QAF },
         { 'k', ARABIC_KAF },
         { 'l', ARABIC_LAM },
         { 'm', ARABIC_MEEM },
         { 'n', ARABIC_NOON },
         { 'h', ARABIC_HEH },
         { 'w', ARABIC_WAW },
         { 'Y', ARABIC_ALEF_MAKSURA },
         { 'y', ARABIC_YEH },
         { 'F', ARABIC_FATHATAN },
         { 'N', ARABIC_DAMMATAN },
         { 'K', ARABIC_KASRATAN },
         { 'a', ARABIC_FATHA },
         { 'u', ARABIC_DAMMA },
         { 'i', ARABIC_KASRA },
         { '~', ARABIC_SHADDA },
         { 'o', ARABIC_SUKUN },
         { '`', ARABIC_MINI_ALEF },
         { '{', ARABIC_ALEF_WASLA }
};

// ISO233-2 romanisation letter mapping
static const character_map ISO233_TO_ARABIC[] = {
    { 0x02CC, ARABIC_HAMZA }, // ˌ
    { '|', ARABIC_ALEF_MADDA },
    { 0x02C8, ARABIC_ALEF_HAMZA_ABOVE }, // ˈ
    { '<', ARABIC_ALEF_HAMZA_BELOW },
    { 0x02BE, ARABIC_ALEF }, // ʾ
    { 'b', ARABIC_BEH },
    { 0x1E97, ARABIC_TEH_MARBUTA }, // ẗ
    { 't', ARABIC_TEH },
    { 0x1E6F, ARABIC_THEH }, // ṯ
    { 0x01E7, ARABIC_JEEM }, // ǧ
    { 0x1E25, ARABIC_HAH }, // ḥ
    { 0x1E96, ARABIC_KHAH }, // ẖ
    { 'd', ARABIC_DAL },
    { 0x1E0F, ARABIC_THAL }, // ḏ
    { 'r', ARABIC_REH },
    { 'z', ARABIC_ZAIN },
    { 's', ARABIC_SEEN },
    { 0x0161, ARABIC_SHEEN }, // š
    { 0x1E63, ARABIC_SAD }, // ṣ
    { 0x1E0D, ARABIC_DAD }, // ḍ
    { 0x1E6D, ARABIC_TAH }, // ṭ
    { 0x1E93, ARABIC_ZAH }, // ẓ
    { 0x02BF, ARABIC_AIN }, // ʿ
    { 0x0121, ARABIC_GHAIN }, // ġ
    { '_', ARABIC_TATWEEL },
    { 'f', ARABIC_FEH },
    { 'q', ARABIC_QAF },
    { 'k', ARABIC_KAF },
    { 'l', ARABIC_LAM },
    { 'm', ARABIC_MEEM },
    { 'n', ARABIC_NOON },
    { 'h', ARABIC_HEH },
    { 'w', ARABIC_WAW },
    { 0x1EF3, ARABIC_ALEF_MAKSURA }, // ỳ
    { 'y', ARABIC_YEH },
    { 0x00E1, ARABIC_FATHATAN }, // á
    { 0x00FA, ARABIC_DAMMATAN }, // ú
    { 0x00ED, ARABIC_KASRATAN }, // í
    { 'a', ARABIC_FATHA },
    { 'u', ARABIC_DAMMA },
    { 'i', ARABIC_KASRA },
    { '~', ARABIC_SHADDA },
    { 0x00B0, ARABIC_SUKUN }, // °
    { '`', ARABIC_MINI_ALEF },
    { '{', ARABIC_ALEF_WASLA },
    // extended here
    { '^', ARABIC_MADDA_ABOVE },
    { '#', ARABIC_HAMZA_ABOVE }
};

#endif // XAPIAN_INCLUDED_ARABICNORMALIZERCONSTANTS_H
