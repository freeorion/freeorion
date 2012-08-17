/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

// This is the implementation of the class which looks up widget metrics on
// Windows systems. It has two implementations, one uses UxTheme to look up
// widget metrics, and the other uses constant values (and is used on systems
// where UxTheme is unavailable or Visual Styles are disabled).

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>

#include <sstream>
#include <stdexcept>
#include <cassert>
#include <cstring>

#include <boost/static_assert.hpp>

#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace metrics {

/****************************************************************************************************/

extents_t measure_text(const std::string& text, const boost::shared_ptr<GG::Font>& font)
{
    assert(font);

    extents_t retval;
    GG::Pt size = font->TextExtent(text);
    retval.width() = Value(size.x);
    retval.height() = Value(size.y);
    return retval;
}

/****************************************************************************************************/

extents_t measure(GG::Wnd* window)
{
    assert(window);

    extents_t retval;
    GG::Pt min_size = window->MinUsableSize();
    retval.width() = Value(min_size.x);
    retval.height() = Value(min_size.y);

    if (GG::TextControl* text_control = dynamic_cast<GG::TextControl*>(window)) {
        retval.slice_m[extents_slices_t::vertical].guide_set_m.push_back(
            measure_baseline(text_control));
    }

    return retval;
}

/****************************************************************************************************/

long measure_baseline(GG::TextControl* text_control)
{
    assert(text_control);

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    GG::Y baseline_offset =
        text_control->TextUpperLeft().y - text_control->UpperLeft().y +
        style->DefaultFont()->Ascent();

    return Value(baseline_offset);
}

} // namespace metrics

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
