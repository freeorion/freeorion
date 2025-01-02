//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 ...
//!  ...
//!  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//!  SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef _GG_Enum_h_
#define _GG_Enum_h_

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <limits>
#include <stdexcept>
#include <array>
#include <type_traits>

namespace GG {

/** 
 * \brief A compile-time utility that maps from enum values to their string names
 *        and vice versa.
 * 
 * Use the \p GG_ENUM or \p GG_CLASS_ENUM macros to create the actual enumerations
 * and string-conversion functions.
 */
template <typename EnumType>
class EnumMap {
public:
    /// Construct from a comma-separated list of "Symbol = Value" pairs or "Symbol" alone.
    constexpr explicit EnumMap(const char* comma_separated_names)
    {
        Build(comma_separated_names);
    }

    /// Convert EnumType -> string
    [[nodiscard]] constexpr std::string_view operator[](EnumType value) const noexcept
    {
        for (std::size_t idx = 0; idx < m_size; ++idx) {
            if (m_values[idx] == value)
                return m_names[idx];
        }
        return "None";
    }

    /// Convert string -> EnumType
    [[nodiscard]] constexpr EnumType operator[](std::string_view name) const noexcept
    {
        return FromString(name);
    }

    /// Convert string -> EnumType, or return not_found_result if not found
    [[nodiscard]] constexpr EnumType FromString(std::string_view name,
                                                EnumType not_found_result = EnumType(0)) const noexcept
    {
        for (std::size_t idx = 0; idx < m_size; ++idx) {
            if (m_names[idx] == name)
                return m_values[idx];
        }
        return not_found_result;
    }

    [[nodiscard]] constexpr auto size() const noexcept { return m_size; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_size == 0; }

private:
    static constexpr std::size_t CAPACITY = std::numeric_limits<std::uint8_t>::max();

    /// Parse the comma-separated names and fill internal arrays.
    constexpr void Build(const char* comma_separated_names)
    {
        auto count = Count(comma_separated_names, ',');
        if (count > CAPACITY)
            throw std::invalid_argument("too many entries in enum map.");

        auto [num_entries, entries] = SplitApply(comma_separated_names, Trim, ',');
        for (std::size_t i = 0; i < num_entries; ++i)
            Insert(entries[i]);
    }

    /// Insert a single "SYMBOL = value" or "SYMBOL" entry
    constexpr void Insert(std::string_view entry)
    {
        auto [parts_count, trimmed_strs] = SplitApply<2>(entry, Trim, '=');

        const auto name = trimmed_strs[0];

        // If there's a second part after "=", parse it; otherwise auto-increment.
        EnumType value = [this, had_value=(parts_count >= 2), val_str=trimmed_strs[1]]()
        {
            if (had_value)
                return EnumType(ToInt(val_str));

            using val_t = std::underlying_type_t<EnumType>;
            val_t next_val = (m_size == 0) ? 0 : (static_cast<val_t>(m_values[m_size - 1]) + 1);
            return EnumType(next_val);
        }();

        if (m_size >= CAPACITY)
            throw std::runtime_error("Too many entries in EnumMap.");

        auto idx = m_size++;
        m_names[idx] = name;
        m_values[idx] = value;
    }

    //------------ Utilities: Splitting, Trimming, Counting --------------//

    [[nodiscard]] static constexpr std::pair<std::string_view, std::string_view>
    Split(std::string_view str, char delim)
    {
        auto idx = str.find(delim);
        if (idx == std::string_view::npos)
            return {str, ""};
        return {
            str.substr(0, idx),
            str.substr(idx + 1)
        };
    }

    [[nodiscard]] static constexpr std::size_t Count(std::string_view text, char delim)
    {
        std::size_t count = 1;
        for (char c : text)
            if (c == delim) count++;
        return count;
    }

    template <std::size_t RET_CAP = CAPACITY, typename F>
    [[nodiscard]] static constexpr std::pair<std::size_t, std::array<std::string_view, RET_CAP>>
    SplitApply(std::string_view raw_string, F&& fn, char delim)
    {
        std::array<std::string_view, RET_CAP> result{};
        std::size_t count = 0;

        while (count < RET_CAP) {
            auto [head, tail] = Split(raw_string, delim);
            if (head.empty()) break;
            result[count++] = fn(head);
            raw_string = tail;
        }
        return {count, result};
    }

    [[nodiscard]] static constexpr std::string_view Trim(std::string_view s)
    {
        constexpr std::string_view ws = " \t\n\r\f\v";
        auto start = s.find_first_not_of(ws);
        if (start == std::string_view::npos) return {};

        auto end = s.find_last_not_of(ws);
        return s.substr(start, end - start + 1);
    }

    //------------ Utilities: Parsing numeric values --------------//

    static constexpr bool IsDec(std::string_view txt) {
        // optional leading '-', plus digits
        if (txt.empty()) return false;
        if (txt[0] == '-' && txt.size() > 1)
            txt.remove_prefix(1);
        for (char c : txt)
            if (c < '0' || c > '9') return false;
        return true;
    }

    static constexpr bool IsHex(std::string_view txt) {
        // "0x" prefix, then 1 or more hex digits
        if (txt.size() < 3) return false;
        if (txt[0] != '0' || txt[1] != 'x') return false;
        auto hex_part = txt.substr(2);
        for (char c : hex_part) {
            bool dec = (c >= '0' && c <= '9');
            bool af  = (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
            if (!dec && !af) return false;
        }
        return true;
    }

    static constexpr int ToInt(std::string_view txt) {
        if (IsHex(txt)) {
            // parse hex: skip "0x" or "0X"
            txt.remove_prefix(2);
            int value = 0;
            for (char c : txt) {
                value <<= 4;
                if (c >= '0' && c <= '9')      value += c - '0';
                else if (c >= 'A' && c <= 'F') value += c - 'A' + 10;
                else if (c >= 'a' && c <= 'f') value += c - 'a' + 10;
            }
            return value;
        } else if (IsDec(txt)) {
            bool neg = (txt[0] == '-');
            if (neg) txt.remove_prefix(1);
            int value = 0;
            for (char c : txt) {
                value = value * 10 + (c - '0');
            }
            return neg ? -value : value;
        }
        return 0; // fallback if not recognized
    }

private:
    std::size_t m_size = 0;
    std::array<std::string_view, CAPACITY> m_names{};
    std::array<EnumType, CAPACITY>         m_values{};
};

/** 
 * Macro for an enum declared inside a class. 
 * Defines an enum named \p EnumName, with underlying type \p UnderlyingType,
 * plus automatic string conversion via `EnumMap`. 
 */
#define GG_CLASS_ENUM(EnumName, UnderlyingType, ...)            \
    enum class EnumName : UnderlyingType {                      \
        __VA_ARGS__                                             \
    };                                                          \
    static inline const ::GG::EnumMap<EnumName>& GetEnumMap() { \
        static const ::GG::EnumMap<EnumName> s_map{#__VA_ARGS__};\
        return s_map;                                           \
    }                                                           \
    friend inline std::istream& operator>>(std::istream& is, EnumName& e) { \
        std::string name;                                       \
        is >> name;                                             \
        e = GetEnumMap()[name];                                 \
        return is;                                              \
    }                                                           \
    friend inline std::ostream& operator<<(std::ostream& os, EnumName e) { \
        return os << GetEnumMap()[e];                           \
    }

/**
 * Macro for an enum declared in free (non-class) scope. 
 * Similar to GG_CLASS_ENUM but suitable for top-level or namespace scope.
 */
#define GG_ENUM(EnumName, UnderlyingType, ...)                                             \
    enum class EnumName : UnderlyingType {                                                 \
        __VA_ARGS__                                                                        \
    };                                                                                     \
    template <> constexpr ::GG::EnumMap<EnumName> CGetEnumMap<EnumName>() {                \
        return ::GG::EnumMap<EnumName>(#__VA_ARGS__);                                      \
    }                                                                                      \
    template <> inline const ::GG::EnumMap<EnumName>& GetEnumMap<EnumName>() {             \
        static const auto s_map = CGetEnumMap<EnumName>();                                 \
        return s_map;                                                                      \
    }                                                                                      \
    inline std::istream& operator>>(std::istream& is, EnumName& e) {                       \
        std::string name;                                                                  \
        is >> name;                                                                        \
        e = GetEnumMap<EnumName>()[name];                                                  \
        return is;                                                                         \
    }                                                                                      \
    inline std::ostream& operator<<(std::ostream& os, EnumName e) {                        \
        return os << GetEnumMap<EnumName>()[e];                                            \
    }                                                                                      \
    [[nodiscard]] constexpr std::string_view to_string(EnumName e) {                       \
        return CGetEnumMap<EnumName>()[e];                                                 \
    }                                                                                      \
    [[nodiscard]] constexpr EnumName EnumName##FromString(std::string_view sv,             \
                                                          EnumName not_found=EnumName(0)) {\
        return CGetEnumMap<EnumName>().FromString(sv, not_found);                          \
    }


//------------------------------------------------------------------------------
// Example usage: simplifying GG::X and GG::Y to be enum classes
// Instead of a more complicated class, we define them as enumerations with 
// potential discrete values. If you had an old dynamic approach, update it as needed.
//------------------------------------------------------------------------------

GG_ENUM(X, int,
    X_0 = 0,
    X_1,
    X_2,
    X_3,
    X_MAX // can add as many as you want
)

GG_ENUM(Y, int,
    Y_0 = 0,
    Y_1,
    Y_2,
    Y_MAX
)

// Optionally, you can define helper functions or operators for X and Y if
// you previously relied on arithmetic in the old classes. For example:
//
// inline int to_pixel_count(X x) { return static_cast<int>(x); }
// inline int to_pixel_count(Y y) { return static_cast<int>(y); }
//
// ...and so on, depending on your usage.

} // namespace GG

#endif // _GG_Enum_h_
