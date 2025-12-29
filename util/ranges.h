#ifndef _Range_Fallback_Include_
#define _Range_Fallback_Include_

#if __has_include(<ranges>) && (!defined(__clang_major__) || (__clang_major__ >= 16)) \
    && (!defined(__GNUC__) || (__GNUC__ >= 11) || (__GNUC__ == 10 && __GNUC_MINOR__ >= 4))
# define USING_STD_RANGES 1
#else
# define USING_STD_RANGES 0
#endif

#if (USING_STD_RANGES)
# include <algorithm>
# include <ranges>
inline constexpr auto& range_keys = std::views::keys;
inline constexpr auto& range_values = std::views::values;
inline constexpr auto& range_transform = std::views::transform;
inline constexpr auto& range_filter = std::views::filter;
//inline constexpr auto& range_join = std::views::join; // no equivalent in boost ranges
inline constexpr auto& range_reverse = std::views::reverse;
inline constexpr auto& range_drop = std::views::drop;
inline constexpr auto& range_find_if = std::ranges::find_if;
inline constexpr auto& range_find = std::ranges::find;
inline constexpr auto& range_empty = std::ranges::empty;
inline constexpr auto& range_any_of = std::ranges::any_of;
inline constexpr auto& range_all_of = std::ranges::all_of;
inline constexpr auto& range_none_of = std::ranges::none_of;
inline constexpr auto& range_copy = std::ranges::copy;
inline constexpr auto& range_copy_if = std::ranges::copy_if;
inline constexpr auto& range_max_element = std::ranges::max_element;
inline constexpr auto& range_min_element = std::ranges::min_element;
inline constexpr auto& range_equal = std::ranges::equal_range;
inline constexpr auto& range_begin = std::ranges::begin;
inline constexpr auto& range_end = std::ranges::end;
inline constexpr auto& range_count_if = std::ranges::count_if;
inline constexpr auto& range_count = std::ranges::count;
inline constexpr auto& range_distance = std::ranges::distance;
#else
# include <span>
# include <boost/range/adaptors.hpp>
# include <boost/range/algorithm.hpp>
# include <boost/range/begin.hpp>
# include <boost/range/end.hpp>
# include <boost/range/empty.hpp>
# include <boost/range/algorithm/find.hpp>
# include <boost/algorithm/cxx11/any_of.hpp>
# include <boost/algorithm/cxx11/all_of.hpp>
# include <boost/algorithm/cxx11/none_of.hpp>
# include <boost/algorithm/cxx11/copy_if.hpp>

# if !defined(__GNUC__)
inline const auto& range_filter = boost::adaptors::filtered;
inline const auto& range_transform = boost::adaptors::transformed;
# else
template <typename Pred>
struct range_filter {
    [[nodiscard]] constexpr explicit range_filter(const Pred& pred_) : pred(pred_) {};
    const Pred& pred;
    using is_range_filter_tag = void;
};

template <typename T>
concept is_range_filter = std::is_same_v<typename T::is_range_filter_tag, void>;

template <is_range_filter RangeFilterT>
inline auto operator|(auto&& r, const RangeFilterT& filter)
{ return std::forward<decltype(r)>(r) | boost::adaptors::filtered(filter.pred); }

template <is_range_filter RangeFilterT>
inline auto operator|(const auto& r, const RangeFilterT& filter)
{ return r | boost::adaptors::filtered(filter.pred); }

template<typename T, std::size_t N, is_range_filter RangeFilterT>
inline auto operator|(std::span<T, N> r, const RangeFilterT& filter) {
    auto begin_it = r.begin();
    const auto end_it = r.end();
    using it_t = std::decay_t<decltype(begin_it)>;
    return boost::iterator_range<it_t>(begin_it, end_it) | boost::adaptors::filtered(filter.pred);
}


template <typename UnaryFunc>
struct range_transform {
    [[nodiscard]] constexpr explicit range_transform(const UnaryFunc& func_) : func(func_) {};
    const UnaryFunc& func;
    using is_range_transform_tag = void;
};

template <typename T>
concept is_range_transform = std::is_same_v<typename T::is_range_transform_tag, void>;

template <is_range_transform RangeTransformT>
inline auto operator|(auto&& r, const RangeTransformT& tform)
{ return std::forward<decltype(r)>(r) | boost::adaptors::transformed(tform.func); }

template <is_range_transform RangeTransformT>
inline auto operator|(const auto& r, const RangeTransformT& tform)
{ return r | boost::adaptors::transformed(tform.func); }

template<typename T, std::size_t N, is_range_transform RangeTransformT>
inline auto operator|(std::span<T, N> r, const RangeTransformT& tform) {
    auto begin_it = r.begin();
    const auto end_it = r.end();
    using it_t = std::decay_t<decltype(begin_it)>;
    return boost::iterator_range<it_t>(begin_it, end_it) | boost::adaptors::transformed(tform.func);
}


# endif


inline const auto& range_reverse = boost::adaptors::reversed;

static constexpr struct range_keys_t {} range_keys{};
auto operator|(auto&& r, const range_keys_t&)
{ return boost::adaptors::keys(std::forward<decltype(r)>(r)); }

static constexpr struct range_values_t {} range_values{};
auto operator|(auto&& r, const range_values_t&)
{ return boost::adaptors::values(std::forward<decltype(r)>(r)); }

template <typename... Args>
inline auto range_find_if(Args&&... args) { return boost::range::find_if(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_find(Args&&... args) { return boost::range::find(std::forward<Args>(args)...); }

template <typename... Args>
inline const auto range_empty(Args&&... args) { return boost::empty(std::forward<Args>(args)...); }

template <typename... Args>
inline const auto range_any_of(Args&&... args) { return boost::algorithm::any_of(std::forward<Args>(args)...); }

# if defined(__GNUC__)
template <typename T, std::size_t E, typename... Args>
inline const auto range_any_of(std::span<T, E> s, Args&&... args)
{ return boost::algorithm::any_of(s.begin(), s.end(), std::forward<Args>(args)...); }
# endif

template <typename... Args>
inline const auto range_all_of(Args&&... args) { return boost::algorithm::all_of(std::forward<Args>(args)...); }
template <typename... Args>
inline const auto range_none_of(Args&&... args) { return boost::algorithm::none_of(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_copy(Args&&... args) { return boost::range::copy(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_copy_if(Args&&... args) { return boost::algorithm::copy_if(std::forward<Args>(args)...); }

# if defined(__GNUC__)
template <typename T, std::size_t E, typename... Args>
inline const auto range_copy_if(std::span<T, E> s, Args&&... args)
{ return boost::algorithm::copy_if(s.begin(), s.end(), std::forward<Args>(args)...); }
# endif

template <typename... Args>
inline auto range_max_element(Args&&... args) { return boost::range::max_element(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_min_element(Args&&... args) { return boost::range::min_element(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_equal(Args&&... args) { return boost::range::equal_range(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_begin(Args&&... args) { return boost::begin(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_end(Args&&... args) { return boost::end(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_count_if(Args&&... args) { return boost::count_if(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_count(Args&&... args) { return boost::count(std::forward<Args>(args)...); }
template <typename... Args>
inline auto range_distance(Args&&... args) { return boost::distance(std::forward<Args>(args)...); }

struct range_drop {
    const std::size_t drop = 1u;
    [[nodiscard]] constexpr explicit range_drop(std::size_t drop_) noexcept : drop(drop_) {}
};
inline auto constexpr operator|(auto&& r, range_drop drop) {
    auto begin_it = range_begin(r);
    const auto end_it = range_end(r);
    const auto sz = static_cast<std::size_t>(boost::distance(r));
    std::advance(begin_it, std::min(sz, drop.drop));
    using it_t = std::decay_t<decltype(begin_it)>;
    return boost::iterator_range<it_t>(begin_it, end_it);
}
#endif

#if defined(__cpp_lib_ranges_contains)
inline constexpr auto& range_contains = std::ranges::contains;
#else
template <typename Rng, typename ValT>
    requires (
        requires (Rng rng, ValT val) { rng.contains(val); } ||
        requires (Rng rng, ValT val) { rng.find(val) != rng.end(); } ||
        requires (Rng rng, ValT val) { *rng.begin() == val; std::find(std::begin(rng), std::end(rng), val) != std::end(rng); }
    )
inline constexpr auto range_contains(Rng&& rng, ValT&& val)
{
    if constexpr (requires { rng.contains(std::forward<ValT>(val)); }) {
        return rng.contains(std::forward<ValT>(val));

    } else if constexpr (requires { rng.find(std::forward<ValT>(val)); }) {
        return rng.find(std::forward<ValT>(val)) != rng.end();

    } else {
        return std::find(std::begin(rng), std::end(rng), std::forward<ValT>(val)) != std::end(rng);
    }
}
#endif

#if defined(__cpp_lib_ranges_to_container)
# include <ranges>
template <typename OutT, typename... Args>
inline constexpr OutT range_to(Args&&... args) { return std::ranges::to<OutT>(std::forward<Args>(args)...); }
#else
template <typename OutT, typename R>
inline constexpr OutT range_to(R&& r)
{
    if constexpr (std::is_rvalue_reference_v<R>)
        return {std::make_move_iterator(range_begin(r)), std::make_move_iterator(range_end(r))};
    else
        return {range_begin(r), range_end(r)};
}
#endif

template <typename OutT>
struct range_to_t {};

template <typename OutT>
inline consteval range_to_t<OutT> range_to() { return {}; } // makes  rng | range_to<T>()  work

template <typename OutT>
inline constexpr OutT operator|(auto&& r, range_to_t<OutT>)
{ return range_to<OutT>(std::forward<decltype(r)>(r)); }

constexpr struct range_to_vec_t {} range_to_vec{};

inline constexpr auto operator|(auto&& r, range_to_vec_t) {
    using ValT = std::remove_cvref_t<decltype(*range_begin(r))>;
    return range_to<std::vector<ValT>>(std::forward<decltype(r)>(r));
}

[[nodiscard]] inline constexpr bool FlexibleContains(const auto& container, const auto val) {
    if constexpr (requires { *std::begin(container) == val; }) {
        return range_contains(container, val);

    } else if constexpr (requires { std::begin(container)->first == val; }) {
        return range_contains(container | range_keys, val);

    } else {
        constexpr auto to_id = [](const auto& o) noexcept {
            if constexpr (requires { o->ID(); })
                return o->ID();
            else if constexpr ( requires { o.ID(); })
                return o.ID();
        };

        if constexpr (requires { *std::begin(container) == to_id(val); }) {
            return range_contains(container, to_id(val));

        } else if constexpr (requires { std::begin(container)->first == to_id(val); }) {
            return range_contains(container | range_keys, to_id(val));

        } else if constexpr (requires { to_id(*std::begin(container)) == val; }) {
            return range_contains(container | range_transform(to_id), val);

        } else if constexpr (requires { to_id(std::begin(container)->first) == val; }) {
            return range_contains(container | range_keys | range_transform(to_id), val);

        }
    }
}


#endif
