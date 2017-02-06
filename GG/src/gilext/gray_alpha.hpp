/*
    Copyright 2008 T. Zachary Laine

    Use, modification and distribution are subject to the Boost Software License,
    Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt).

    See http://opensource.adobe.com/gil for most recent version including documentation.
*/

/*************************************************************************************************/

#ifndef GILEXT_GRAY_ALPHA_H
#define GILEXT_GRAY_ALPHA_H

////////////////////////////////////////////////////////////////////////////////////////
/// \file
/// \brief Support for gray alpha color space and variants
/// \author T. Zachary Laine
/// \date 2008 \n Last updated on November 6, 2008
////////////////////////////////////////////////////////////////////////////////////////

#include <cstddef>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/vector_c.hpp>
#include <boost/gil/gil_config.hpp>
#include <boost/gil/metafunctions.hpp>
#include <boost/gil/planar_pixel_iterator.hpp>

namespace boost { namespace gil {

/// \ingroup ColorSpaceModel
typedef mpl::vector2<gray_color_t,alpha_t> gray_alpha_t;

/// \ingroup LayoutModel
typedef layout<gray_alpha_t> gray_alpha_layout_t;

/// \ingroup ImageViewConstructors
/// \brief from raw gray alpha planar data
template <typename IC>
inline
typename type_from_x_iterator<planar_pixel_iterator<IC,gray_alpha_t> >::view_t
planar_gray_alpha_view(std::size_t width, std::size_t height,
                IC g, IC a,
                std::ptrdiff_t rowsize_in_bytes) {
    typedef typename type_from_x_iterator<planar_pixel_iterator<IC,gray_alpha_t> >::view_t RView;
    return RView(width, height,
                 typename RView::locator(planar_pixel_iterator<IC,gray_alpha_t>(g,a),
                                         rowsize_in_bytes));
}

} }  // namespace boost::gil

#endif
