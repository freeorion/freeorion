#ifndef _Range_Fallback_Include_
#define _Range_Fallback_Include_

#if __has_include(<ranges>) && (!defined(__clang_major__) || (__clang_major__ >= 16)) \
    && (!defined(__GNUC__) || (__GNUC__ >= 11) || (__GNUC__ == 10 && __GNUC_MINOR__ >= 4))
# define USING_STD_RANGES 1
#else
# define USING_STD_RANGES 0
#endif

#if (USING_STD_RANGES)
# include <ranges>
inline constexpr auto& range_keys = std::views::keys;
inline constexpr auto& range_values = std::views::values;
inline constexpr auto& range_transform = std::views::transform;
inline constexpr auto& range_filter = std::views::filter;
//inline constexpr auto& range_join = std::views::join; // no equivalent in boost ranges
inline constexpr auto& range_reverse = std::views::reverse;
inline constexpr auto& range_find_if = std::ranges::find_if;
inline constexpr auto& range_any_of = std::ranges::any_of;
inline constexpr auto& range_copy = std::ranges::copy;
inline constexpr auto& range_copy_if = std::ranges::copy_if;
inline constexpr auto& range_max_element = std::ranges::max_element;
inline constexpr auto& range_min_element = std::ranges::min_element;
inline constexpr auto& range_equal = std::ranges::equal_range;
inline constexpr auto& range_end = std::ranges::end;
#else
# include <boost/range/adaptor/map.hpp>
# include <boost/range/adaptor/filtered.hpp>
# include <boost/range/adaptor/transformed.hpp>
# include <boost/range/adaptor/reversed.hpp>
# include <boost/range/algorithm.hpp>
# include <boost/range/end.hpp>
# include <boost/algorithm/cxx11/any_of.hpp>
# include <boost/algorithm/cxx11/copy_if.hpp>
inline const auto& range_keys = boost::adaptors::map_keys;
inline const auto& range_values = boost::adaptors::map_values;
inline const auto& range_filter = boost::adaptors::filtered;
inline const auto& range_transform = boost::adaptors::transformed;
inline const auto& range_reverse = boost::adaptors::reversed;
template <typename... Args>
inline auto range_find_if(Args... args) { return boost::range::find_if(std::forward<Args>(args)...); }
template <typename... Args>
inline const auto range_any_of(Args... args) { return boost::algorithm::any_of(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_copy(Args... args) { return boost::range::copy(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_copy_if(Args... args) { return boost::algorithm::copy_if(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_max_element(Args... args) { return boost::range::max_element(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_min_element(Args... args) { return boost::range::min_element(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_equal(Args... args) { return boost::range::equal_range(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_end(Args... args) { return boost::end(std::forward<Args>(args)...); }

#endif

#endif
