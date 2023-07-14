#ifndef _Range_Fallback_Include_
#define _Range_Fallback_Include_

#if __has_include(<ranges>) && (!defined(__clang_major__) || (__clang_major__ >= 16))
# include <ranges>
static constexpr auto range_keys = std::views::keys;
static constexpr auto range_values = std::views::values;
static constexpr auto range_transform = std::views::transform;
static constexpr auto range_filter = std::views::filter;
#else
# include <boost/range/adaptor/map.hpp>
# include <boost/range/adaptor/filtered.hpp>
# include <boost/range/adaptor/transformed.hpp>
static const auto& range_keys = boost::adaptors::map_keys;
static const auto& range_values = boost::adaptors::map_values;
static const auto& range_filter = boost::adaptors::filtered;
static const auto& range_transform = boost::adaptors::transformed;
#endif

#endif