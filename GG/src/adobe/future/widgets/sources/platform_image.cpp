/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_image.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/memory.hpp>

#include <string>
#include <cassert>

#include <GG/GUI.h>
#include <GG/StaticGraphic.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

class ImageFilter :
    public GG::Wnd
{
public:
    ImageFilter(image_t& image) : m_image(image) {}

    virtual bool EventFilter(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::LButtonDown) {
                m_image.last_point_m = event.Point();
                retval = true;
            } else if (event.Type() == GG::WndEvent::LDrag) {
                GG::Pt cur_point(event.Point());
                double x(Value(cur_point.x));
                double y(Value(cur_point.y));

                if (m_image.last_point_m != cur_point && m_image.callback_m) {
                    double delta_x(Value(m_image.last_point_m.x - cur_point.x));
                    double delta_y(Value(m_image.last_point_m.y - cur_point.y));

                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"),
                                                                       any_regular_t(delta_x)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"),
                                                                       any_regular_t(delta_y)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"),
                                                                       any_regular_t(true)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("x"),
                                                                       any_regular_t(x)));
                    m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("y"),
                                                                       any_regular_t(y)));

                    m_image.callback_m(m_image.metadata_m);
                }

                m_image.last_point_m = cur_point;

                retval = true;
            } else if (event.Type() == GG::WndEvent::LButtonUp ||
                       event.Type() == GG::WndEvent::LClick) {
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"),
                                                                   any_regular_t(0)));
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"),
                                                                   any_regular_t(0)));
                m_image.metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"),
                                                                   any_regular_t(false)));
                if (m_image.callback_m)
                    m_image.callback_m(m_image.metadata_m);

                retval = true;
            }
            return retval;
        }

    image_t& m_image;
};

} // implementation

} // adobe

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

const GG::X fixed_width(250);
const GG::Y fixed_height(Value(fixed_width));

/****************************************************************************************************/

GG::X get_width(adobe::image_t& image)
{ return image.image_m ? image.image_m->DefaultWidth() : fixed_width; }

/****************************************************************************************************/

GG::Y get_height(adobe::image_t& image)
{ return image.image_m ? image.image_m->DefaultHeight() : fixed_height; }

/****************************************************************************************************/

void reset_image(adobe::image_t& image, const adobe::image_t::view_model_type& view)
{
    delete image.window_m;

    if (view) {
        image.window_m =
            adobe::implementation::Factory().NewStaticGraphic(
                GG::X0, GG::Y0, view->DefaultWidth(), view->DefaultHeight(), view,
                GG::GRAPHIC_NONE, GG::INTERACTIVE
            );
        image.filter_m.reset(new adobe::implementation::ImageFilter(image));
        image.window_m->InstallEventFilter(image.filter_m.get());
    }
}

/****************************************************************************************************/

} // namespace


namespace adobe {

/****************************************************************************************************/

image_t::image_t(const view_model_type& image) :
    window_m(0),
    image_m(image)
{
    metadata_m.insert(dictionary_t::value_type(static_name_t("delta_x"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("delta_y"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("dragging"), any_regular_t(false)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("x"), any_regular_t(0)));
    metadata_m.insert(dictionary_t::value_type(static_name_t("y"), any_regular_t(0)));
}

/****************************************************************************************************/

void image_t::display(const view_model_type& value)
{
    image_m = value;
    reset_image(*this, image_m);
}

/****************************************************************************************************/

void image_t::monitor(const setter_proc_type& proc)
{ callback_m = proc; }

/****************************************************************************************************/

void place(image_t& value, const place_data_t& place_data)
{
    implementation::set_control_bounds(value.window_m, place_data);

    if (value.callback_m)
    {
        dictionary_t old_metadata(value.metadata_m);

        double width(Value(std::min(fixed_width, get_width(value))));
        double height(Value(std::min(fixed_height, get_height(value))));

        value.metadata_m.insert(dictionary_t::value_type(static_name_t("width"), any_regular_t(width)));
        value.metadata_m.insert(dictionary_t::value_type(static_name_t("height"), any_regular_t(height)));

        if (old_metadata != value.metadata_m)
            value.callback_m(value.metadata_m);
    }
}

/****************************************************************************************************/

void measure(image_t& value, extents_t& result)
{
    // TODO: figure out why a set monitor implies a fixed size
    if (value.callback_m)
        result.horizontal().length_m = Value(fixed_width);
    else
        result.horizontal().length_m = Value(get_width(value));
}

/****************************************************************************************************/

void measure_vertical(image_t& value, extents_t& result,
                      const place_data_t& placed_horizontal)
{
    if (value.callback_m) {
        result.vertical().length_m = Value(fixed_height);
    } else {
        // TODO: handle graphics flags for which non-aspect-ratio-preserving stretches are ok

        double aspect_ratio =
            Value(get_height(value)) / static_cast<double>(Value(get_width(value)));

        result.vertical().length_m =
            static_cast<long>(placed_horizontal.horizontal().length_m * aspect_ratio);
    }
}

/****************************************************************************************************/

void enable(image_t& value, bool make_enabled)
{
    if (value.window_m)
        value.window_m->Disable(!make_enabled);
}

/*************************************************************************************************/

template <>
platform_display_type insert<image_t>(display_t&             display,
                                      platform_display_type& parent,
                                      image_t&               element)
{
    assert(!element.window_m);
    reset_image(element, element.image_m);
    return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
