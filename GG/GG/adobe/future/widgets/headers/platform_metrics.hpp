/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// This header defines functions which can return various widget metrics
// on Windows systems. When available, the UXTHEME library is used to
// discover metrics. When UXTHEME is not available some reasonable
// defaults (precalculated on a system without UXTHEME) are returned.

/****************************************************************************************************/

#ifndef ADOBE_METRICS_HPP
#define ADOBE_METRICS_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/extents.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <boost/shared_ptr.hpp>

#include <string>

namespace GG {
    class Font;
    class TextControl;
}


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/// The adobe::metrics suite of functions can fetch information
/// on ideal widget sizes. The design of this suite is to provide
/// simple metric and text information for a particular widget, and
/// require clients of the function suite to compose the metrics
/// information in a way that is meaningful for them.

/****************************************************************************************************/

namespace metrics {

/****************************************************************************************************/

//
/// Measure text and return the optimal extents.
//

extents_t measure_text(const std::string& text, const boost::shared_ptr<GG::Font>& font);

/****************************************************************************************************/

//
/// Measure the window and return the optimal extents.
//

extents_t measure(GG::Wnd* window);

/****************************************************************************************************/

//
/// Measure the baseline of the TextControl.
//

long measure_baseline(GG::TextControl* text_control);

/****************************************************************************************************/

} // namespace metrics

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
