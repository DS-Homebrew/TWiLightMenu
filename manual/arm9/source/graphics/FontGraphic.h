#pragma once

#include <nds.h>
#include <string>
#include <string_view>
#include <vector>

enum class Alignment {
	left,
	center,
	right,
};

class FontGraphic {
private:
	constexpr static char16_t arabicPresentationForms[][3] = {
		// Initial, Medial, Final
		{u'آ', u'ﺂ', u'ﺂ'}, // Alef with madda above
		{u'أ', u'ﺄ', u'ﺄ'}, // Alef with hamza above
		{u'ؤ', u'ﺆ', u'ﺆ'}, // Waw with hamza above
		{u'إ', u'ﺈ', u'ﺈ'}, // Alef with hamza below
		{u'ﺋ', u'ﺌ', u'ﺊ'}, // Yeh with hamza above
		{u'ا', u'ﺎ', u'ﺎ'}, // Alef
		{u'ﺑ', u'ﺒ', u'ﺐ'}, // Beh
		{u'ة', u'ﺔ', u'ﺔ'}, // Teh marbuta
		{u'ﺗ', u'ﺘ', u'ﺖ'}, // Teh
		{u'ﺛ', u'ﺜ', u'ﺚ'}, // Theh
		{u'ﺟ', u'ﺠ', u'ﺞ'}, // Jeem
		{u'ﺣ', u'ﺤ', u'ﺢ'}, // Hah
		{u'ﺧ', u'ﺨ', u'ﺦ'}, // Khah
		{u'د', u'ﺪ', u'ﺪ'}, // Dal
		{u'ذ', u'ﺬ', u'ﺬ'}, // Thal
		{u'ر', u'ﺮ', u'ﺮ'}, // Reh
		{u'ز', u'ﺰ', u'ﺰ'}, // Zain
		{u'ﺳ', u'ﺴ', u'ﺲ'}, // Seen
		{u'ﺷ', u'ﺸ', u'ﺶ'}, // Sheen
		{u'ﺻ', u'ﺼ', u'ﺺ'}, // Sad
		{u'ﺿ', u'ﻀ', u'ﺾ'}, // Dad
		{u'ﻃ', u'ﻄ', u'ﻂ'}, // Tah
		{u'ﻇ', u'ﻈ', u'ﻆ'}, // Zah
		{u'ﻋ', u'ﻌ', u'ﻊ'}, // Ain
		{u'ﻏ', u'ﻐ', u'ﻎ'}, // Ghain
		{u'ػ', u'ػ', u'ػ'}, // Keheh with two dots above
		{u'ؼ', u'ؼ', u'ؼ'}, // Keheh with three dots below
		{u'ؽ', u'ؽ', u'ؽ'}, // Farsi yeh with inverted v
		{u'ؾ', u'ؾ', u'ؾ'}, // Farsi yeh with two dots above
		{u'ؿ', u'ؿ', u'ؿ'}, // Farsi yeh with three docs above
		{u'ـ', u'ـ', u'ـ'}, // Tatweel
		{u'ﻓ', u'ﻔ', u'ﻒ'}, // Feh
		{u'ﻗ', u'ﻘ', u'ﻖ'}, // Qaf
		{u'ﻛ', u'ﻜ', u'ﻚ'}, // Kaf
		{u'ﻟ', u'ﻠ', u'ﻞ'}, // Lam
		{u'ﻣ', u'ﻤ', u'ﻢ'}, // Meem
		{u'ﻧ', u'ﻨ', u'ﻦ'}, // Noon
		{u'ﻫ', u'ﻬ', u'ﻪ'}, // Heh
		{u'و', u'ﻮ', u'ﻮ'}, // Waw
		{u'ﯨ', u'ﯩ', u'ﻰ'}, // Alef maksura
		{u'ﻳ', u'ﻴ', u'ﻲ'}, // Yeh
	};

	static bool isArabic(char16_t c);
	static bool isStrongRTL(char16_t c);
	static bool isWeak(char16_t c);
	static bool isNumber(char16_t c);

	static char16_t arabicForm(char16_t current, char16_t prev, char16_t next);

	static u8 *lastUsedLoc;

	bool useExpansionPak = false;
	u8 tileWidth = 0, tileHeight = 0;
	u16 tileSize = 0;
	int tileAmount = 0;
	u16 questionMark = 0;
	u8 *fontTiles = nullptr;
	u8 *fontWidths = nullptr;
	u16 *fontMap = nullptr;

	u16 getCharIndex(char16_t c);

public:
	static u8 textBuf[2][256 * 192];

	static std::u16string utf8to16(std::string_view text);

	FontGraphic(const std::vector<std::string> &paths, const bool useExpansionPak);

	~FontGraphic(void);

	u8 height(void) { return tileHeight; }

	int calcWidth(std::string_view text) { return calcWidth(utf8to16(text)); }
	int calcWidth(std::u16string_view text);

	void print(int x, int y, bool top, int value, Alignment align, bool rtl = false) { print(x, y, top, std::to_string(value), align, rtl); }
	void print(int x, int y, bool top, std::string_view text, Alignment align, bool rtl = false) { print(x, y, top, utf8to16(text), align, rtl); }
	void print(int x, int y, bool top, std::u16string_view text, Alignment align, bool rtl = false);
};
