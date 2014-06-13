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
    std::make_pair(0xfe83,  0x0623),
    std::make_pair(0xfe87, 0x0625),
    std::make_pair(0xfe8b, 0x0626),
    std::make_pair(0xfe8f, 0x0628),
    std::make_pair(0xfe93, 0x0629),
    std::make_pair(0xfe97, 0x062a),
    std::make_pair(0xfe9b, 0x062b),
    std::make_pair(0xfe9f, 0x062c),
    std::make_pair(0xfea3, 0x062d),
    std::make_pair(0xfea7, 0x062e),
    std::make_pair(0xfeab, 0x0630),
    std::make_pair(0xfeaf, 0x0632),
    std::make_pair(0xfeb3, 0x0633),
    std::make_pair(0xfeb7, 0x0634),
    std::make_pair(0xfebb, 0x0635),
    std::make_pair(0xfebf, 0x0636),
    std::make_pair(0xfec3, 0x0637),
    std::make_pair(0xfec7, 0x0638),
    std::make_pair(0xfecb, 0x0639),
    std::make_pair(0xfecf, 0x063a),
    std::make_pair(0xfed3, 0x0641),
    std::make_pair(0xfed7, 0x0642),
    std::make_pair(0xfedb, 0x0643),
    std::make_pair(0xfedf, 0x0644),
    std::make_pair(0xfee3, 0x0645),
    std::make_pair(0xfee7, 0x0646),
    std::make_pair(0xfeeb, 0x0647),
    std::make_pair(0xfeef, 0x0649),
    std::make_pair(0xfef3, 0x064a),
    std::make_pair(0xfe80, 0x0621),
    std::make_pair(0xfe84, 0x0623),
    std::make_pair(0xfe88, 0x0625),
    std::make_pair(0xfe8c, 0x0626),
    std::make_pair(0xfe90, 0x0628),
    std::make_pair(0xfe94, 0x0629),
    std::make_pair(0xfe98, 0x062a),
    std::make_pair(0xfe9c, 0x062b),
    std::make_pair(0xfea0, 0x062c),
    std::make_pair(0xfea4, 0x062d),
    std::make_pair(0xfea8, 0x062e),
    std::make_pair(0xfeac, 0x0630),
    std::make_pair(0xfeb0, 0x0632),
    std::make_pair(0xfeb4, 0x0633),
    std::make_pair(0xfeb8, 0x0634),
    std::make_pair(0xfebc, 0x0635),
    std::make_pair(0xfec0, 0x0636),
    std::make_pair(0xfec4, 0x0637),
    std::make_pair(0xfec8, 0x0638),
    std::make_pair(0xfecc, 0x0639),
    std::make_pair(0xfed0, 0x063a),
    std::make_pair(0xfed4, 0x0641),
    std::make_pair(0xfed8, 0x0642),
    std::make_pair(0xfedc, 0x0643),
    std::make_pair(0xfee0, 0x0644),
    std::make_pair(0xfee4, 0x0645),
    std::make_pair(0xfee8, 0x0646),
    std::make_pair(0xfeec, 0x0647),
    std::make_pair(0xfef0, 0x0649),
    std::make_pair(0xfef4, 0x064a),
    std::make_pair(0xfe81, 0x0622),
    std::make_pair(0xfe85, 0x0624),
    std::make_pair(0xfe89, 0x0626),
    std::make_pair(0xfe8d, 0x0627),
    std::make_pair(0xfe91, 0x0628),
    std::make_pair(0xfe95, 0x062a),
    std::make_pair(0xfe99, 0x062b),
    std::make_pair(0xfe9d, 0x062c),
    std::make_pair(0xfea1, 0x062d),
    std::make_pair(0xfea5, 0x062e),
    std::make_pair(0xfea9, 0x062f),
    std::make_pair(0xfead, 0x0631),
    std::make_pair(0xfeb1, 0x0633),
    std::make_pair(0xfeb5, 0x0634),
    std::make_pair(0xfeb9, 0x0635),
    std::make_pair(0xfebd, 0x0636),
    std::make_pair(0xfec1, 0x0637),
    std::make_pair(0xfec5, 0x0638),
    std::make_pair(0xfec9, 0x0639),
    std::make_pair(0xfecd, 0x063a),
    std::make_pair(0xfed1, 0x0641),
    std::make_pair(0xfed5, 0x0642),
    std::make_pair(0xfed9, 0x0643),
    std::make_pair(0xfedd, 0x0644),
    std::make_pair(0xfee1, 0x0645),
    std::make_pair(0xfee5, 0x0646),
    std::make_pair(0xfee9, 0x0647),
    std::make_pair(0xfeed, 0x0648),
    std::make_pair(0xfef1, 0x064a),
    std::make_pair(0xfe82, 0x0622),
    std::make_pair(0xfe86, 0x0624),
    std::make_pair(0xfe8a, 0x0626),
    std::make_pair(0xfe8e, 0x0627),
    std::make_pair(0xfe92, 0x0628),
    std::make_pair(0xfe96, 0x062a),
    std::make_pair(0xfe9a, 0x062b),
    std::make_pair(0xfe9e, 0x062c),
    std::make_pair(0xfea2, 0x062d),
    std::make_pair(0xfea6, 0x062e),
    std::make_pair(0xfeaa, 0x062f),
    std::make_pair(0xfeae, 0x0631),
    std::make_pair(0xfeb2, 0x0633),
    std::make_pair(0xfeb6, 0x0634),
    std::make_pair(0xfeba, 0x0635),
    std::make_pair(0xfebe, 0x0636),
    std::make_pair(0xfec2, 0x0637),
    std::make_pair(0xfec6, 0x0638),
    std::make_pair(0xfeca, 0x0639),
    std::make_pair(0xfece, 0x063a),
    std::make_pair(0xfed2, 0x0641),
    std::make_pair(0xfed6, 0x0642),
    std::make_pair(0xfeda, 0x0643),
    std::make_pair(0xfede, 0x0644),
    std::make_pair(0xfee2, 0x0645),
    std::make_pair(0xfee6, 0x0646),
    std::make_pair(0xfeea, 0x0647),
    std::make_pair(0xfeee, 0x0648),
    std::make_pair(0xfef2, 0x064a)
};

std::map<unsigned, unsigned> ARABIC_SHAPING_MAP(shaping_map_data, shaping_map_data + sizeof shaping_map_data / sizeof shaping_map_data[0]);


// Buckwalter Romanization Mapping
std::pair<char, unsigned> buckwalter_to_unicode_map_data[] = {
    std::make_pair('\'',  0x0621), // hamza-on-the-line
    std::make_pair('|',  0x0622), // madda
    std::make_pair('>',  0x0623), // hamza-on-'alif
    std::make_pair('&',  0x0624), // hamza-on-waaw
    std::make_pair('<',  0x0625), // hamza-under-'alif
    std::make_pair('}',  0x0626), // hamza-on-yaa'
    std::make_pair('A',  0x0627), // bare 'alif
    std::make_pair('b',  0x0628), // baa'
    std::make_pair('p',  0x0629), // taa' marbuuTa
    std::make_pair('t',  0x062A), // taa'
    std::make_pair('v',  0x062B), // thaa'
    std::make_pair('j',  0x062C), // jiim
    std::make_pair('H',  0x062D), // Haa'
    std::make_pair('x',  0x062E), // khaa'
    std::make_pair('d',  0x062F), // daal
    std::make_pair('*',  0x0630), // dhaal
    std::make_pair('r',  0x0631), // raa'
    std::make_pair('z',  0x0632), // zaay
    std::make_pair('s',  0x0633), // siin
    std::make_pair('$',  0x0634), // shiin
    std::make_pair('S',  0x0635), // Saad
    std::make_pair('D',  0x0636), // Daad
    std::make_pair('T',  0x0637), // Taa'
    std::make_pair('Z',  0x0638), // Zaa' (DHaa')
    std::make_pair('E',  0x0639), // cayn
    std::make_pair('g',  0x063A), // ghayn
    std::make_pair('_',  0x0640), // taTwiil
    std::make_pair('f',  0x0641), // faa'
    std::make_pair('q',  0x0642), // qaaf
    std::make_pair('k',  0x0643), // kaaf
    std::make_pair('l',  0x0644), // laam
    std::make_pair('m',  0x0645), // miim
    std::make_pair('n',  0x0646), // nuun
    std::make_pair('h',  0x0647), // haa'
    std::make_pair('w',  0x0648), // waaw
    std::make_pair('Y',  0x0649), // 'alif maqSuura
    std::make_pair('y',  0x064A), // yaa'
    std::make_pair('F',  0x064B), // fatHatayn
    std::make_pair('N',  0x064C), // Dammatayn
    std::make_pair('K',  0x064D), // kasratayn
    std::make_pair('a',  0x064E), // fatHa
    std::make_pair('u',  0x064F), // Damma
    std::make_pair('i',  0x0650), // kasra
    std::make_pair('~',  0x0651), // shaddah
    std::make_pair('o',  0x0652), // sukuun
    std::make_pair('`',  0x0670), // dagger 'alif
    std::make_pair('{',  0x0671), // waSla
};

std::map<char, unsigned> BUCKWALTER_TO_ARABIC(buckwalter_to_unicode_map_data, buckwalter_to_unicode_map_data + sizeof buckwalter_to_unicode_map_data / sizeof buckwalter_to_unicode_map_data[0]);

#endif // XAPIAN_INCLUDED_ARABICNORMALIZERCONSTANTS_H
