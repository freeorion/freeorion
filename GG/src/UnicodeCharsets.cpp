/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/UnicodeCharsets.h>
#include <GG/utf8/checked.h>

#include <map>


using namespace GG;

namespace {
    const std::size_t BLOCK_SIZE = 16;
}

UnicodeCharset::UnicodeCharset() :
    m_first_char(0),
    m_last_char(0)
{}

UnicodeCharset::UnicodeCharset(std::string script_name, std::uint32_t first_char, std::uint32_t last_char) :
    m_script_name(script_name),
    m_first_char(first_char),
    m_last_char(last_char + 1)
{
    assert(script_name != "");
    assert(m_first_char % BLOCK_SIZE == 0);
    assert(m_last_char % BLOCK_SIZE == 0);
    assert(m_first_char < m_last_char);
}

bool GG::operator==(const UnicodeCharset& lhs, const UnicodeCharset& rhs)
{
    return lhs.m_script_name == rhs.m_script_name &&
        lhs.m_first_char == rhs.m_first_char &&
        lhs.m_last_char == rhs.m_last_char;
}

bool GG::operator<(const UnicodeCharset& lhs, const UnicodeCharset& rhs)
{ return lhs.m_first_char < rhs.m_first_char; }

const std::vector<UnicodeCharset>& GG::AllUnicodeCharsets()
{
    static const std::vector<UnicodeCharset> ALL_UNICODE_CHARSETS{
        UnicodeCharset("Basic Latin", 0x0000, 0x007F),
        UnicodeCharset("Latin-1 Supplement", 0x0080, 0x00FF),
        UnicodeCharset("Latin Extended-A", 0x0100, 0x017F),
        UnicodeCharset("Latin Extended-B", 0x0180, 0x024F),
        UnicodeCharset("IPA Extensions", 0x0250, 0x02AF),
        UnicodeCharset("Spacing Modifier Letters", 0x02B0, 0x02FF),
        UnicodeCharset("Combining Diacritical Marks", 0x0300, 0x036F),
        UnicodeCharset("Greek and Coptic", 0x0370, 0x03FF),
        UnicodeCharset("Cyrillic", 0x0400, 0x04FF),
        UnicodeCharset("Cyrillic Supplement", 0x0500, 0x052F),
        UnicodeCharset("Armenian", 0x0530, 0x058F),
        UnicodeCharset("Hebrew", 0x0590, 0x05FF),
        UnicodeCharset("Arabic", 0x0600, 0x06FF),
        UnicodeCharset("Syriac", 0x0700, 0x074F),
        UnicodeCharset("Arabic Supplement", 0x0750, 0x077F),
        UnicodeCharset("Thaana", 0x0780, 0x07BF),
        UnicodeCharset("N'Ko, 0x(Mandenkan)", 0x07C0, 0x07FF),
        UnicodeCharset("Devanagari", 0x0900, 0x097F),
        UnicodeCharset("Bengali", 0x0980, 0x09FF),
        UnicodeCharset("Gurmukhi", 0x0A00, 0x0A7F),
        UnicodeCharset("Gujarati", 0x0A80, 0x0AFF),
        UnicodeCharset("Oriya", 0x0B00, 0x0B7F),
        UnicodeCharset("Tamil", 0x0B80, 0x0BFF),
        UnicodeCharset("Telugu", 0x0C00, 0x0C7F),
        UnicodeCharset("Kannada", 0x0C80, 0x0CFF),
        UnicodeCharset("Malayalam", 0x0D00, 0x0D7F),
        UnicodeCharset("Sinhala", 0x0D80, 0x0DFF),
        UnicodeCharset("Thai", 0x0E00, 0x0E7F),
        UnicodeCharset("Lao", 0x0E80, 0x0EFF),
        UnicodeCharset("Tibetan", 0x0F00, 0x0FFF),
        UnicodeCharset("Burmese (Myanmar)", 0x1000, 0x109F),
        UnicodeCharset("Georgian", 0x10A0, 0x10FF),
        UnicodeCharset("Hangul Jamo", 0x1100, 0x11FF),
        UnicodeCharset("Ethiopic", 0x1200, 0x137F),
        UnicodeCharset("Ethiopic Supplement", 0x1380, 0x139F),
        UnicodeCharset("Cherokee", 0x13A0, 0x13FF),
        UnicodeCharset("Unified Canadian Aboriginal Syllabics", 0x1400, 0x167F),
        UnicodeCharset("Ogham", 0x1680, 0x169F),
        UnicodeCharset("Runic", 0x16A0, 0x16FF),
        UnicodeCharset("Tagalog", 0x1700, 0x171F),
        UnicodeCharset("Hanunóo", 0x1720, 0x173F),
        UnicodeCharset("Buhid", 0x1740, 0x175F),
        UnicodeCharset("Tagbanwa", 0x1760, 0x177F),
        UnicodeCharset("Khmer", 0x1780, 0x17FF),
        UnicodeCharset("Mongolian", 0x1800, 0x18AF),
        UnicodeCharset("Limbu", 0x1900, 0x194F),
        UnicodeCharset("Tai Le", 0x1950, 0x197F),
        UnicodeCharset("New Tai Lue", 0x1980, 0x19DF),
        UnicodeCharset("Khmer Symbols", 0x19E0, 0x19FF),
        UnicodeCharset("Buginese", 0x1A00, 0x1A1F),
        UnicodeCharset("Balinese", 0x1B00, 0x1B7F),
        UnicodeCharset("Lepcha (Rong)", 0x1C00, 0x1C4F),
        UnicodeCharset("Ol Chiki (Santali / Ol Cemet’)", 0x1C50, 0x1C7F),
        UnicodeCharset("Phonetic Extensions", 0x1D00, 0x1D7F),
        UnicodeCharset("Phonetic Extensions Supplement", 0x1D80, 0x1DBF),
        UnicodeCharset("Combining Diacritical Marks Supplement", 0x1DC0, 0x1DFF),
        UnicodeCharset("Latin Extended Additional", 0x1E00, 0x1EFF),
        UnicodeCharset("Greek Extended", 0x1F00, 0x1FFF),
        UnicodeCharset("General Punctuation", 0x2000, 0x206F),
        UnicodeCharset("Superscripts and Subscripts", 0x2070, 0x209F),
        UnicodeCharset("Currency Symbols", 0x20A0, 0x20CF),
        UnicodeCharset("Combining Diacritical Marks for Symbols", 0x20D0, 0x20FF),
        UnicodeCharset("Letterlike Symbols", 0x2100, 0x214F),
        UnicodeCharset("Number Forms", 0x2150, 0x218F),
        UnicodeCharset("Arrows", 0x2190, 0x21FF),
        UnicodeCharset("Mathematical Operators", 0x2200, 0x22FF),
        UnicodeCharset("Miscellaneous Technical", 0x2300, 0x23FF),
        UnicodeCharset("Control Pictures", 0x2400, 0x243F),
        UnicodeCharset("Optical Character Recognition", 0x2440, 0x245F),
        UnicodeCharset("Enclosed Alphanumerics", 0x2460, 0x24FF),
        UnicodeCharset("Box Drawing", 0x2500, 0x257F),
        UnicodeCharset("Block Elements", 0x2580, 0x259F),
        UnicodeCharset("Geometric Shapes", 0x25A0, 0x25FF),
        UnicodeCharset("Miscellaneous Symbols", 0x2600, 0x26FF),
        UnicodeCharset("Dingbats", 0x2700, 0x27BF),
        UnicodeCharset("Miscellaneous Mathematical Symbols-A", 0x27C0, 0x27EF),
        UnicodeCharset("Supplemental Arrows-A", 0x27F0, 0x27FF),
        UnicodeCharset("Braille Patterns", 0x2800, 0x28FF),
        UnicodeCharset("Supplemental Arrows-B", 0x2900, 0x297F),
        UnicodeCharset("Miscellaneous Mathematical Symbols-B", 0x2980, 0x29FF),
        UnicodeCharset("Supplemental Mathematical Operators", 0x2A00, 0x2AFF),
        UnicodeCharset("Miscellaneous Symbols and Arrows", 0x2B00, 0x2BFF),
        UnicodeCharset("Glagolitic", 0x2C00, 0x2C5F),
        UnicodeCharset("Latin Extended-C", 0x2C60, 0x2C7F),
        UnicodeCharset("Coptic", 0x2C80, 0x2CFF),
        UnicodeCharset("Georgian Supplement", 0x2D00, 0x2D2F),
        UnicodeCharset("Tifinagh", 0x2D30, 0x2D7F),
        UnicodeCharset("Ethiopic Extended", 0x2D80, 0x2DDF),
        UnicodeCharset("Cyrillic Extended-A", 0x2DE0, 0x2DFF),
        UnicodeCharset("Supplemental Punctuation", 0x2E00, 0x2E7F),
        UnicodeCharset("CJK Radicals Supplement", 0x2E80, 0x2EFF),
        UnicodeCharset("Kangxi Radicals", 0x2F00, 0x2FDF),
        UnicodeCharset("Ideographic Description Characters", 0x2FF0, 0x2FFF),
        UnicodeCharset("CJK Symbols and Punctuation", 0x3000, 0x303F),
        UnicodeCharset("Hiragana", 0x3040, 0x309F),
        UnicodeCharset("Katakana", 0x30A0, 0x30FF),
        UnicodeCharset("Bopomofo", 0x3100, 0x312F),
        UnicodeCharset("Hangul Compatibility Jamo", 0x3130, 0x318F),
        UnicodeCharset("Kanbun", 0x3190, 0x319F),
        UnicodeCharset("Bopomofo Extended", 0x31A0, 0x31BF),
        UnicodeCharset("CJK Strokes", 0x31C0, 0x31EF),
        UnicodeCharset("Katakana Phonetic Extensions", 0x31F0, 0x31FF),
        UnicodeCharset("Enclosed CJK Letters and Months", 0x3200, 0x32FF),
        UnicodeCharset("CJK Compatibility", 0x3300, 0x33FF),
        UnicodeCharset("CJK Unified Ideographs Extension A", 0x3400, 0x4DBF),
        UnicodeCharset("Yijing Hexagram Symbols", 0x4DC0, 0x4DFF),
        UnicodeCharset("CJK Unified Ideographs", 0x4E00, 0x9FFF),
        UnicodeCharset("Yi Syllables", 0xA000, 0xA48F),
        UnicodeCharset("Yi Radicals", 0xA490, 0xA4CF),
        UnicodeCharset("Vai", 0xA500, 0xA63F),
        UnicodeCharset("Cyrillic Extended-B", 0xA640, 0xA69F),
        UnicodeCharset("Modifier Tone Letters", 0xA700, 0xA71F),
        UnicodeCharset("Latin Extended-D", 0xA720, 0xA7FF),
        UnicodeCharset("Syloti Nagri", 0xA800, 0xA82F),
        UnicodeCharset("Phags-pa", 0xA840, 0xA87F),
        UnicodeCharset("Saurashtra", 0xA880, 0xA8DF),
        UnicodeCharset("Kayah Li", 0xA900, 0xA92F),
        UnicodeCharset("Rejang", 0xA930, 0xA95F),
        UnicodeCharset("Cham", 0xAA00, 0xAA5F),
        UnicodeCharset("Hangul Syllables", 0xAC00, 0xD7AF),
        UnicodeCharset("High Surrogates", 0xD800, 0xDB7F),
        UnicodeCharset("High Private Use Surrogates", 0xDB80, 0xDBFF),
        UnicodeCharset("Low Surrogates", 0xDC00, 0xDFFF),
        UnicodeCharset("Private Use Area", 0xE000, 0xF8FF),
        UnicodeCharset("CJK Compatibility Ideographs", 0xF900, 0xFAFF),
        UnicodeCharset("Alphabetic Presentation Forms", 0xFB00, 0xFB4F),
        UnicodeCharset("Arabic Presentation Forms-A", 0xFB50, 0xFDFF),
        UnicodeCharset("Variation Selectors", 0xFE00, 0xFE0F),
        UnicodeCharset("Vertical Forms", 0xFE10, 0xFE1F),
        UnicodeCharset("Combining Half Marks", 0xFE20, 0xFE2F),
        UnicodeCharset("CJK Compatibility Forms", 0xFE30, 0xFE4F),
        UnicodeCharset("Small Form Variants", 0xFE50, 0xFE6F),
        UnicodeCharset("Arabic Presentation Forms-B", 0xFE70, 0xFEFF),
        UnicodeCharset("Halfwidth and Fullwidth Forms", 0xFF00, 0xFFEF),
        UnicodeCharset("Specials", 0xFFF0, 0xFFFF)
    };

    return ALL_UNICODE_CHARSETS;
}

std::set<UnicodeCharset> GG::UnicodeCharsetsToRender(const std::string& str)
{
    std::set<UnicodeCharset> retval;
    auto it = str.begin();
    auto end_it = str.end();
    while (it != end_it) {
        if (auto charset = CharsetContaining(utf8::next(it, end_it)))
            retval.insert(*charset);
    }
    return retval;
}

const UnicodeCharset* GG::CharsetContaining(std::uint32_t c)
{
    static std::vector<const UnicodeCharset*> s_charset_blocks;
    if (s_charset_blocks.empty()) {
        s_charset_blocks.resize(AllUnicodeCharsets().back().m_last_char / BLOCK_SIZE);
        for (const UnicodeCharset& uchs : AllUnicodeCharsets()) {
            std::size_t first_block = uchs.m_first_char / BLOCK_SIZE;
            std::size_t last_block = uchs.m_last_char / BLOCK_SIZE;
            for (std::size_t j = first_block; j != last_block; ++j) {
                s_charset_blocks[j] = &uchs;
            }
        }
    }
    std::size_t block = c / BLOCK_SIZE;
    return block < s_charset_blocks.size() ? s_charset_blocks[block] : nullptr;
}

const UnicodeCharset* GG::CharsetWithName(const std::string& name)
{
    static std::map<std::string, const UnicodeCharset*> s_name_map;
    if (s_name_map.empty()) {
        for (const UnicodeCharset& uchs : AllUnicodeCharsets()) {
            s_name_map[uchs.m_script_name] = &uchs;
        }
    }
    auto it = s_name_map.find(name);
    return it == s_name_map.end() ? nullptr : it->second;
}
