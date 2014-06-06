
// Specific punctuation marks
#define ARABIC_COMMA "\u060C"
#define ARABIC_SEMICOLON "\u061B"
#define ARABIC_QUESTION "\u061F"

// Hamza forms
#define ARABIC_HAMZA "\u0621"
#define ARABIC_ALEF_MADDA "\u0622"
#define ARABIC_ALEF_HAMZA_ABOVE "\u0623"
#define ARABIC_WAW_HAMZA "\u0624"
#define ARABIC_ALEF_HAMZA_BELOW "\u0625"
#define ARABIC_YEH_HAMZA "\u0626"

// Letters
#define ARABIC_ALEF "\u0627"
#define ARABIC_BEH "\u0628"
#define ARABIC_TEH_MARBUTA "\u0629"
#define ARABIC_TEH "\u062a"
#define ARABIC_THEH "\u062b"
#define ARABIC_JEEM "\u062c"
#define ARABIC_HAH "\u062d"
#define ARABIC_KHAH "\u062e"
#define ARABIC_DAL "\u062f"
#define ARABIC_THAL "\u0630"
#define ARABIC_REH "\u0631"
#define ARABIC_ZAIN "\u0632"
#define ARABIC_SEEN "\u0633"
#define ARABIC_SHEEN "\u0634"
#define ARABIC_SAD "\u0635"
#define ARABIC_DAD "\u0636"
#define ARABIC_TAH "\u0637"
#define ARABIC_ZAH "\u0638"
#define ARABIC_AIN "\u0639"
#define ARABIC_GHAIN "\u063a"
#define ARABIC_TATWEEL "\u0640"
#define ARABIC_FEH "\u0641"
#define ARABIC_QAF "\u0642"
#define ARABIC_KAF "\u0643"
#define ARABIC_LAM "\u0644"
#define ARABIC_MEEM "\u0645"
#define ARABIC_NOON "\u0646"
#define ARABIC_HEH "\u0647"
#define ARABIC_WAW "\u0648"
#define ARABIC_ALEF_MAKSURA "\u0649"
#define ARABIC_YEH "\u064a"
#define ARABIC_MADDA_ABOVE "\u0653"
#define ARABIC_HAMZA_ABOVE "\u0654"
#define ARABIC_HAMZA_BELOW "\u0655"

// Hindu–Arabic numerals
#define ARABIC_ZERO "\u0660"
#define ARABIC_ONE "\u0661"
#define ARABIC_TWO "\u0662"
#define ARABIC_THREE "\u0663"
#define ARABIC_FOUR "\u0664"
#define ARABIC_FIVE "\u0665"
#define ARABIC_SIX "\u0666"
#define ARABIC_SEVEN "\u0667"
#define ARABIC_EIGHT "\u0668"
#define ARABIC_NINE "\u0669"
#define ARABIC_PERCENT "\u066a"
#define ARABIC_DECIMAL "\u066b"
#define ARABIC_THOUSANDS "\u066c"

#define ARABIC_STAR "\u066d"
#define ARABIC_MINI_ALEF "\u0670"
#define ARABIC_ALEF_WASLA "\u0671"
#define ARABIC_FULL_STOP "\u06d4"
#define ARABIC_BYTE_ORDER_MARK "\ufeff"

// Diacritics
#define ARABIC_FATHATAN "\u064b"
#define ARABIC_DAMMATAN "\u064c"
#define ARABIC_KASRATAN "\u064d"
#define ARABIC_FATHA "\u064e"
#define ARABIC_DAMMA "\u064f"
#define ARABIC_KASRA "\u0650"
#define ARABIC_SHADDA "\u0651"
#define ARABIC_SUKUN "\u0652"

// Small Letters
#define ARABIC_SMALL_ALEF u"\u0670"
#define ARABIC_SMALL_WAW u"\u06E5"
#define ARABIC_SMALL_YEH u"\u06E6"

// Ligatures
#define ARABIC_LAM_ALEF "\ufefb"
#define ARABIC_LAM_ALEF_HAMZA_ABOVE "\ufef7"
#define ARABIC_LAM_ALEF_HAMZA_BELOW "\ufef9"
#define ARABIC_LAM_ALEF_MADDA_ABOVE "\ufef5"
#define ARABIC_simple_LAM_ALEF "\u0644\u0627"
#define ARABIC_simple_LAM_ALEF_HAMZA_ABOVE "\u0644\u0623"
#define ARABIC_simple_LAM_ALEF_HAMZA_BELOW "\u0644\u0625"
#define ARABIC_simple_LAM_ALEF_MADDA_ABOVE "\u0644\u0622"

// groups
#define ARABIC_LETTERS ARABIC_ALEF ARABIC_BEH ARABIC_TEH ARABIC_TEH_MARBUTA   \
					   ARABIC_THEH ARABIC_JEEM ARABIC_HAH ARABIC_KHAH ARABIC_DAL \
					   ARABIC_THAL ARABIC_REH ARABIC_ZAIN ARABIC_SEEN ARABIC_SHEEN \
					   ARABIC_SAD ARABIC_DAD ARABIC_TAH	ARABIC_ZAH ARABIC_AIN \
					   ARABIC_GHAIN  ARABIC_FEH ARABIC_QAF ARABIC_KAF ARABIC_LAM \
					   ARABIC_MEEM ARABIC_NOON ARABIC_HEH ARABIC_WAW ARABIC_YEH \
					   ARABIC_HAMZA ARABIC_ALEF_MADDA ARABIC_ALEF_HAMZA_ABOVE \
					   ARABIC_WAW_HAMZA ARABIC_ALEF_HAMZA_BELOW ARABIC_YEH_HAMZA
#define ARABIC_TASHKEEL ARABIC_FATHATAN ARABIC_DAMMATAN ARABIC_KASRATAN \
						ARABIC_FATHA ARABIC_DAMMA  ARABIC_KASRA ARABIC_SUKUN \
						ARABIC_SHADDA
#define ARABIC_LIGUATURES ARABIC_LAM_ALEF ARABIC_LAM_ALEF_HAMZA_ABOVE \
				   ARABIC_LAM_ALEF_HAMZA_BELOW  ARABIC_LAM_ALEF_MADDA_ABOVE
#define ARABIC_HAMZAT ARABIC_HAMZA ARABIC_WAW_HAMZA ARABIC_YEH_HAMZA ARABIC_HAMZA_ABOVE \
               ARABIC_HAMZA_BELOW ARABIC_ALEF_HAMZA_BELOW ARABIC_ALEF_HAMZA_ABOVE
#define ARABIC_ALEFAT ARABIC_ALEF ARABIC_ALEF_MADDA ARABIC_ALEF_HAMZA_ABOVE \
			   ARABIC_ALEF_HAMZA_BELOW ARABIC_ALEF_WASLA ARABIC_ALEF_MAKSURA \
			   ARABIC_SMALL_ALEF
#define ARABIC_YAHLIKE ARABIC_YEH ARABIC_YEH_HAMZA ARABIC_ALEF_MAKSURA ARABIC_SMALL_YEH
#define ARABIC_TEHLIKE TEH, TEH_MARBUTA
#define ARABIC_WAWLIKE ARABIC_WAW ARABIC_WAW_HAMZA ARABIC_SMALL_WAW

// orders
const char* ARABIC_ALPHABETIC_ORDER[29][6] = {
										/*0 */	  {ARABIC_HAMZA,
													ARABIC_ALEF_MADDA,
													ARABIC_ALEF_HAMZA_ABOVE,
													ARABIC_WAW_HAMZA ,
													ARABIC_ALEF_HAMZA_BELOW,
													ARABIC_YEH_HAMZA },
										/*1 */	  {ARABIC_ALEF, ARABIC_ALEF_MAKSURA, "","","",""},
										/*2 */	  {ARABIC_BEH, "", "","","",""},
										/*3 */    {ARABIC_TEH, ARABIC_TEH_MARBUTA, "","","",""},
										/*4 */	  {ARABIC_THEH, "", "","","",""},
										/*5 */	  {ARABIC_JEEM , "", "","","",""},
										/*6 */	  {ARABIC_HAH , "", "","","",""},
										/*7 */	  {ARABIC_KHAH, "", "","","",""},
										/*8 */	  {ARABIC_DAL, "", "","","",""},
										/*9 */	  {ARABIC_THAL, "", "","","",""},
										/*10 */	  {ARABIC_REH, "", "","","",""},
										/*11 */	  {ARABIC_ZAIN, "", "","","",""},
										/*12 */	  {ARABIC_SEEN, "", "","","",""},
										/*13 */	  {ARABIC_SHEEN, "", "","","",""},
										/*14 */	  {ARABIC_SAD, "", "","","",""},
										/*15 */	  {ARABIC_DAD, "", "","","",""},
										/*16 */	  {ARABIC_TAH, "", "","","",""},
										/*17 */	  {ARABIC_ZAH, "", "","","",""},
										/*18 */	  {ARABIC_AIN, "", "","","",""},
										/*19 */	  {ARABIC_GHAIN, "", "","","",""},
										/*20 */	  {ARABIC_FEH, "", "","","",""},
										/*21 */	  {ARABIC_QAF, "", "","","",""},
										/*22 */	  {ARABIC_KAF, "", "","","",""},
										/*23 */	  {ARABIC_LAM, "", "","","",""},
										/*24 */	  {ARABIC_MEEM, "", "","","",""},
										/*25 */	  {ARABIC_NOON, "", "","","",""},
										/*26 */	  {ARABIC_HEH, "", "","","",""},
										/*27 */	  {ARABIC_WAW, "", "","","",""},
										/*28 */	  {ARABIC_YEH, "", "","","",""},
											 };
