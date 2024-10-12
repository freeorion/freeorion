//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/StrongTypedef.h
//!
//! Contains macros used to create "strong typedefs", that is value types that
//! are not mutually interoperable with each other or with builtin types for
//! extra type safety.

#ifndef _GG_StrongTypedef_h_
#define _GG_StrongTypedef_h_


#include <cmath>
#include <cstdint>
#include <limits>

namespace CXRound {
    [[nodiscard]] constexpr int32_t round_to_int(float f) noexcept
    { return static_cast<int32_t>((f >= 0.0f) ? (f + 0.5f) : (f - 0.5f)); }
    [[nodiscard]] constexpr int32_t round_to_int(double d) noexcept
    { return static_cast<int32_t>((d >= 0.0) ? (d + 0.5) : (d - 0.5)); }
    [[nodiscard]] constexpr std::size_t abs_to_size(int32_t i) noexcept
    { return static_cast<std::size_t>(i >= 0 ? i : -i); }
}

#define POSITION_TYPEDEF(name)                                                                  \
enum class name : int32_t {};                                                                   \
inline constexpr name name ## 0 {0};                                                            \
inline constexpr name name ## 1 {1};                                                            \
                                                                                                \
constexpr name To ## name (float f) noexcept { return name{CXRound::round_to_int(f)}; }         \
constexpr name To ## name (double d) noexcept { return name{CXRound::round_to_int(d)}; }        \
                                                                                                \
constexpr int32_t Value(name x) noexcept { return static_cast<int32_t>(x); }                    \
                                                                                                \
constexpr name operator-(name x) noexcept { return name{-Value(x)}; }                           \
                                                                                                \
constexpr name operator+(name lhs, name rhs) noexcept { return name{Value(lhs) + Value(rhs)}; } \
constexpr name operator+(name x, int32_t i) noexcept { return name{Value(x) + i}; }             \
constexpr name operator+(int32_t i, name x) noexcept { return name{Value(x) + i}; }             \
constexpr name& operator+=(name& lhs, name rhs) noexcept { lhs = lhs + rhs; return lhs; }       \
constexpr name& operator+=(name& lhs, int32_t rhs) noexcept { lhs = lhs + rhs; return lhs; }    \
constexpr float operator+(name x, float f) noexcept { return Value(x) + f; }                    \
constexpr float operator+(float f, name x) noexcept { return x + f; }                           \
constexpr name& operator+=(name&, float) noexcept = delete;                                     \
constexpr double operator+(name x, double d) noexcept { return Value(x) + d; }                  \
constexpr double operator+(double d, name x) noexcept { return x + d; }                         \
constexpr name& operator+=(name&, double) noexcept = delete;                                    \
                                                                                                \
constexpr name operator-(name lhs, name rhs) noexcept { return name{Value(lhs) - Value(rhs)}; } \
constexpr name operator-(name x, int32_t i) noexcept { return name{Value(x) - i}; }             \
constexpr int32_t operator-(int32_t i, name x) noexcept { return i - Value(x); }                \
constexpr name& operator-=(name& lhs, name rhs) noexcept { lhs = lhs - rhs; return lhs; }       \
constexpr name& operator-=(name& lhs, int32_t rhs) noexcept { lhs = lhs - rhs; return lhs; }    \
constexpr float operator-(name x, float f) noexcept { return Value(x) - f; }                    \
constexpr float operator-(float f, name x) noexcept { return f - Value(x); }                    \
constexpr name& operator-=(name&, float) noexcept = delete;                                     \
constexpr double operator-(name x, double d) noexcept { return Value(x) - d; }                  \
constexpr double operator-(double d, name x) noexcept { return d - Value(x); }                  \
constexpr name& operator-=(name&, double) noexcept = delete;                                    \
                                                                                                \
constexpr name operator*(name lhs, name rhs) noexcept { return name{Value(lhs) * Value(rhs)}; } \
constexpr name operator*(name x, int32_t i) noexcept { return name{Value(x) * i}; }             \
constexpr name operator*(int32_t i, name x) noexcept { return name{Value(x) * i}; }             \
constexpr name& operator*=(name& lhs, name rhs) noexcept { lhs = lhs * rhs; return lhs; }       \
constexpr name& operator*=(name& lhs, int32_t rhs) noexcept { lhs = lhs * rhs; return lhs; }    \
constexpr float operator*(name x, float f) noexcept { return Value(x) * f; }                    \
constexpr float operator*(float f, name x) noexcept { return x * f; }                           \
constexpr name& operator*=(name& x, float f) noexcept { x = To ## name (x * f); return x; }     \
constexpr double operator*(name x, double d) noexcept { return Value(x) * d; }                  \
constexpr double operator*(double d, name x) noexcept { return x * d; }                         \
constexpr name& operator*=(name& x, double d) noexcept { x = To ## name (x * d); return x; }    \
                                                                                                \
constexpr int32_t operator/(name lhs, name rhs) noexcept { return Value(lhs) / Value(rhs); }    \
constexpr name operator/(name x, int32_t i) noexcept { return name{Value(x) / i}; }             \
constexpr name& operator/=(name& lhs, int32_t rhs) noexcept { lhs = lhs / rhs; return lhs; }    \
constexpr float operator/(name x, float f) noexcept { return Value(x) / f; }                    \
constexpr float operator/(float, name) noexcept = delete;                                       \
constexpr name& operator/=(name& x, float f) noexcept { x = To ## name(x / f); return x; }      \
constexpr double operator/(name x, double d) noexcept { return Value(x) / d; }                  \
constexpr double operator/(double, name) noexcept = delete;                                     \
constexpr name& operator/=(name& x, double d) noexcept { x = To ## name(x / d); return x; }     \
                                                                                                \
constexpr name& operator++(name& x)     { x += name ## 1; return x; }                           \
constexpr name operator++(name& x, int32_t) { name rv = x; x += name ## 1; return rv; }         \
constexpr name& operator--(name& x)     { x -= name ## 1; return x; }                           \
constexpr name operator--(name& x, int32_t) { name rv = x; x -= name ## 1; return rv; }



#define SIZE_TYPEDEF(name, abbrevname)                                                          \
enum class name : std::size_t {};                                                               \
inline constexpr name abbrevname ## 0 {0};                                                      \
inline constexpr name abbrevname ## 1 {1};                                                      \
inline constexpr name INVALID_ ## abbrevname ## _SIZE {std::numeric_limits<std::size_t>::max()};\
                                                                                                \
constexpr std::size_t Value(name x) noexcept { return static_cast<std::size_t>(x); }            \
                                                                                                \
constexpr name operator+(name lhs, name rhs) noexcept { return name{Value(lhs) + Value(rhs)}; } \
constexpr name operator+(name x, std::size_t s) noexcept { return name{Value(x) + s}; }         \
constexpr name operator+(std::size_t s, name x) noexcept { return name{Value(x) + s}; }         \
constexpr name& operator+=(name& lhs, name rhs) noexcept { lhs = lhs + rhs; return lhs; }       \
constexpr name& operator+=(name& lhs, std::size_t rhs) noexcept { lhs = lhs + rhs; return lhs; }\
constexpr float operator+(name, float) noexcept = delete;                                       \
constexpr float operator+(float, name) noexcept = delete;                                       \
constexpr name& operator+=(name&, float) noexcept = delete;                                     \
constexpr double operator+(name, double) noexcept = delete;                                     \
constexpr double operator+(double, name) noexcept = delete;                                     \
constexpr name& operator+=(name&, double) noexcept = delete;                                    \
                                                                                                \
constexpr name operator-(name lhs, name rhs) noexcept { return name{Value(lhs) - Value(rhs)}; } \
constexpr name operator-(name x, std::size_t s) noexcept { return name{Value(x) - s}; }         \
constexpr std::size_t operator-(std::size_t s, name x) noexcept { return s - Value(x); }        \
constexpr name& operator-=(name& lhs, name rhs) noexcept { lhs = lhs - rhs; return lhs; }       \
constexpr name& operator-=(name& x, std::size_t s) noexcept { x = x - s; return x; }            \
constexpr float operator-(name, float) noexcept = delete;                                       \
constexpr float operator-(float, name) noexcept = delete;                                       \
constexpr name& operator-=(name&, float) noexcept = delete;                                     \
constexpr double operator-(name, double) noexcept = delete;                                     \
constexpr double operator-(double, name) noexcept = delete;                                     \
constexpr name& operator-=(name&, double) noexcept = delete;                                    \
                                                                                                \
constexpr name& operator++(name& x)     { x += abbrevname ## 1; return x; }                     \
constexpr name operator++(name& x, int32_t) { name rv = x; x += abbrevname ## 1; return rv; }   \
constexpr name& operator--(name& x)     { x -= abbrevname ## 1; return x; }                     \
constexpr name operator--(name& x, int32_t) { name rv = x; x -= abbrevname ## 1; return rv; }

#endif
