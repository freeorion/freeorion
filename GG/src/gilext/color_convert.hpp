/*
    Copyright 2008 T. Zachary Laine
 
    Use, modification and distribution are subject to the Boost Software License,
    Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).

    See http://opensource.adobe.com/gil for most recent version including documentation.
*/
/*************************************************************************************************/

#ifndef GILEXT_COLOR_CONVERT_HPP
#define GILEXT_COLOR_CONVERT_HPP

#include <boost/gil/color_convert.hpp>
#include "gray_alpha.hpp"

namespace boost { namespace gil {

/// \ingroup ColorConvert
/// \brief Gray Alpha to RGBA
template <>
struct default_color_converter_impl<gray_alpha_t,rgba_t> {
    template <typename P1, typename P2>
    void operator()(const P1& src, P2& dst) const {
        get_color(dst,red_t())  =
            channel_convert<typename color_element_type<P2, red_t  >::type>(get_color(src,gray_color_t()));
        get_color(dst,green_t())=
            channel_convert<typename color_element_type<P2, green_t>::type>(get_color(src,gray_color_t()));
        get_color(dst,blue_t()) =
            channel_convert<typename color_element_type<P2, blue_t >::type>(get_color(src,gray_color_t()));
        get_color(dst,alpha_t()) =
            channel_convert<typename color_element_type<P2, alpha_t>::type>(get_color(src,alpha_t()));
    }
};

/// \ingroup ColorConvert
/// \brief RGBA to Gray Alpha
template <>
struct default_color_converter_impl<rgba_t,gray_alpha_t> {
    template <typename P1, typename P2>
    void operator()(const P1& src, P2& dst) const {
        get_color(dst,gray_color_t()) = 
            detail::rgb_to_luminance<typename color_element_type<P2,gray_color_t>::type>(
                get_color(src,red_t()), get_color(src,green_t()), get_color(src,blue_t())
            );
         get_color(dst,alpha_t()) =
            channel_convert<typename color_element_type<P2, alpha_t>::type>(get_color(src,alpha_t()));
    }
};

} }  // namespace boost::gil

#endif
