//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Clr.h
//!
//! Contains the utility class Clr, which represents colors in GG.

#ifndef _GG_Clr_h_
#define _GG_Clr_h_

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <GG/Export.h>


namespace GG {

/** \brief A simple 32-bit structure that can act as a packed 32-bit unsigned
    integer representation of a RGBA color, a vector of the four unsigned
    bytes that compose an RGBA color, or the individual unsigned bytes "a",
    "r", "g", and "b".

    You should not use literals to initialize Color objects; depending on the
    endian-ness of the machine, 0x00FFFFFF would be transparent white
    (little-endian) or opaque yellow (big-endian).*/
struct Clr
{
    [[nodiscard]] constexpr Clr() = default;

    /** ctor that constructs a Clr from four ints that represent the color channels */
    [[nodiscard]] constexpr Clr(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255u) noexcept :
        r(r_), g(g_), b(b_), a(a_)
    {}

    /** ctor that constructs a Clr from std::array that represents the color channels */
    [[nodiscard]] constexpr Clr(std::array<uint8_t, 4> clr) noexcept :
        Clr(clr[0], clr[1], clr[2], clr[3])
    {}

    /** ctor that constructs a Clr from a string that represents the color
        channels in the format 'RRGGBB', 'RRGGBBAA' where each channel value
        ranges from 00 to FF. When the alpha component is left out, the alpha
        value FF is assumed. When characters out of the range 0-9 and A-F are
        passed, results are undefined.
    */
    [[nodiscard]] constexpr Clr(std::string_view hex_colour)
    {
        const auto sz = hex_colour.size();
        r = (sz >= 2u) ? HexCharsToUInt8(hex_colour.substr(0, 2)) : 0u;
        g = (sz >= 4u) ? HexCharsToUInt8(hex_colour.substr(2, 2)) : 0u;
        b = (sz >= 6u) ? HexCharsToUInt8(hex_colour.substr(4, 2)) : 0u;
        a = (sz >= 8u) ? HexCharsToUInt8(hex_colour.substr(6, 2)) : 255u;
    }

    [[nodiscard]] explicit constexpr operator uint32_t() const noexcept
    { return (uint32_t{r} << 24u) + (uint32_t{g} << 16u) + (uint32_t{b} << 8u) + uint32_t{a}; }

    [[nodiscard]] constexpr auto ToCharArray() const noexcept
    {
        //                                 "255"  ' '   0
        std::array<std::string::value_type, 4*3 + 3*1 + 1> buf{};
        auto it = buf.data();

        it = UInt8ToChars(it, r);
        *it++ = ' ';
        it = UInt8ToChars(it, g);
        *it++ = ' ';
        it = UInt8ToChars(it, b);
        *it++ = ' ';
        it = UInt8ToChars(it, a);

        return buf;
    }

    [[nodiscard]] explicit operator std::string() const
    {
        const auto data = ToCharArray();
        return std::string{data.data()};
    }

    [[nodiscard]] constexpr std::array<uint8_t, 4> RGBA() const noexcept
    { return {r, g, b, a}; }

    [[nodiscard]] constexpr std::array<float, 4> ToNormalizedRGBA() const noexcept
    { return {r/255.0f, g/255.0f, b/255.0f, a/255.0f}; }


#if defined(__cpp_impl_three_way_comparison)
    [[nodiscard]] constexpr auto operator<=>(const Clr&) const noexcept = default;
#else
    [[nodiscard]] constexpr bool operator==(const Clr& rhs) const noexcept
    { return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a; };
    [[nodiscard]] constexpr bool operator!=(const Clr& rhs) const noexcept
    { return r != rhs.r || g != rhs.g || b != rhs.b || a != rhs.a; };
#endif

    uint8_t r = 0;    ///< the red channel
    uint8_t g = 0;    ///< the green channel
    uint8_t b = 0;    ///< the blue channel
    uint8_t a = 0;    ///< the alpha channel

    [[nodiscard]] static constexpr auto ToHexChars(uint8_t bits) noexcept
    {
        using val_t = std::string_view::value_type;
        constexpr auto to_char = [](uint8_t nibble) -> val_t
        { return (nibble > 9) ? ('A' - 10 + nibble) : ('0' + nibble); };

        val_t high_char = to_char(bits >> 4);
        val_t low_char = to_char(bits & 0xF);
        return std::array<val_t, 2>{high_char, low_char};
    };

    [[nodiscard]] constexpr std::array<std::string_view::value_type, 8> Hex() const noexcept
    {
        const auto rhex = ToHexChars(r);
        const auto ghex = ToHexChars(g);
        const auto bhex = ToHexChars(b);
        const auto ahex = ToHexChars(a);
        return {rhex[0], rhex[1], ghex[0], ghex[1], bhex[0], bhex[1], ahex[0], ahex[1]};
    }

    [[nodiscard]] static constexpr uint8_t HexCharToUint8(std::string_view::value_type digit) noexcept
    { return (digit >= 'A' ? (digit - 'A' + 10) : (digit - '0')); }

    [[nodiscard]] static constexpr uint8_t HexCharsToUInt8(std::string_view chars) noexcept {
        if (chars.empty())
            return 0;
        const uint8_t val0 = HexCharToUint8(chars[0]);
        if (chars.size() == 1) [[unlikely]]
            return val0;
        const uint8_t val1 = HexCharToUint8(chars[1]);
        return 16*val0 + val1;
    };

    static constexpr std::string::value_type* UInt8ToChars(
        std::string::value_type* out_it, const uint8_t num) noexcept
    {
        const uint8_t hundreds = num / 100u;
        const uint8_t less_than_100 = num - hundreds*100u;
        const uint8_t tens = less_than_100 / 10u;
        const uint8_t ones = less_than_100 - tens*10u;
        if (hundreds > 0)
            *out_it++ = (hundreds + '0');
        if (tens > 0 || hundreds > 0)
            *out_it++ = (tens + '0');
        *out_it++ = (ones + '0');
        return out_it;
    };

    [[nodiscard]] static constexpr auto UInt8ToCharArray(uint8_t num) noexcept
    {
        static_assert(std::array<std::string::value_type, 4>{}.back() == 0);
        std::array<std::string::value_type, 4> buf{};
        UInt8ToChars(buf.data(), num);
        return buf;
    };
};

inline std::ostream& operator<<(std::ostream& os, Clr clr)
{
    os << "(" << +clr.r << ", " << +clr.g << ", " << +clr.b << ", " << +clr.a << ")";
    return os;
}

//! Returns the lightened version of color clr.  LightenClr leaves the alpha
//! channel unchanged, and multiplies the other channels by some factor.
constexpr Clr LightenClr(Clr clr, float factor = 2.0) noexcept
{
    return Clr(
        static_cast<uint8_t>(std::max(std::min(clr.r*factor, 255.0f), 0.0f)),
        static_cast<uint8_t>(std::max(std::min(clr.g*factor, 255.0f), 0.0f)),
        static_cast<uint8_t>(std::max(std::min(clr.b*factor, 255.0f), 0.0f)),
        clr.a);
}

//! Returns the darkened version of color clr.  DarkenClr leaves the alpha
//! channel unchanged, and divides the other channels by some factor.
constexpr Clr DarkenClr(const Clr clr, float factor = 2.0) noexcept
{
    return Clr(
        static_cast<uint8_t>(std::max(std::min(clr.r / factor, 255.0f), 0.0f)),
        static_cast<uint8_t>(std::max(std::min(clr.g / factor, 255.0f), 0.0f)),
        static_cast<uint8_t>(std::max(std::min(clr.b / factor, 255.0f), 0.0f)),
        clr.a);
}

constexpr Clr InvertClr(const Clr clr) noexcept
{ return Clr(255 - clr.r, 255 - clr.g, 255 - clr.b, clr.a); }

constexpr Clr BlendClr(Clr src, Clr dst, float factor = 0.5f) noexcept
{
    const auto ifactor = 1.0f - factor;
    return Clr(static_cast<uint8_t>(std::max(std::min(src.r*factor + dst.r*ifactor, 255.0f), 0.0f)),
               static_cast<uint8_t>(std::max(std::min(src.g*factor + dst.g*ifactor, 255.0f), 0.0f)),
               static_cast<uint8_t>(std::max(std::min(src.b*factor + dst.b*ifactor, 255.0f), 0.0f)),
               static_cast<uint8_t>(std::max(std::min(src.a*factor + dst.a*ifactor, 255.0f), 0.0f)));
}

/** Named ctor that constructs a Clr from four floats that represent the color
    channels (each must be >= 0.0 and <= 1.0). */
constexpr Clr FloatClr(float r, float g, float b, float a) noexcept
{
    return Clr(static_cast<uint8_t>(std::max(std::min(r, 1.0f) * 255, 0.0f)),
               static_cast<uint8_t>(std::max(std::min(g, 1.0f) * 255, 0.0f)),
               static_cast<uint8_t>(std::max(std::min(b, 1.0f) * 255, 0.0f)),
               static_cast<uint8_t>(std::max(std::min(a, 1.0f) * 255, 0.0f)));
}

}


//! Calls the appropriate version of glColor*() with @a clr.
GG_API void glColor(GG::Clr clr);


#endif
