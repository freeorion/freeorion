/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_edit_text.hpp>

#include <GG/adobe/controller_concept.hpp>
#include <GG/adobe/placeable_concept.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>

#include <GG/GUI.h>
#include <GG/MultiEdit.h>
#include <GG/StyleFactory.h>


/*************************************************************************************************/

namespace {

/*************************************************************************************************/

const int gap = 4;

} // namespace

/*************************************************************************************************/

namespace adobe {

namespace implementation {

/*************************************************************************************************/

class EditTextFilter :
    public GG::Wnd
{
public:
    EditTextFilter(edit_text_t& edit_text) : m_edit_text(edit_text) {}

    virtual bool EventFilter(GG::Wnd*, const GG::WndEvent& event)
        {
            bool retval = false;
            if (event.Type() == GG::WndEvent::MouseWheel) {
                bool squelch;
                if (event.WheelMove() < 0)
                    m_edit_text.pre_edit_proc_m(std::string(30, 1), squelch);
                else if (0 < event.WheelMove())
                    m_edit_text.pre_edit_proc_m(std::string(31, 1), squelch);
                retval = true;
            } else if (event.Type() == GG::WndEvent::KeyPress) {
                bool nontext =
                    event.GetKey() == GG::GGK_HOME ||
                    event.GetKey() == GG::GGK_LEFT ||
                    event.GetKey() == GG::GGK_RIGHT ||
                    event.GetKey() == GG::GGK_END ||
                    event.GetKey() == GG::GGK_BACKSPACE ||
                    event.GetKey() == GG::GGK_DELETE ||
                    event.GetKey() == GG::GGK_RETURN ||
                    event.GetKey() == GG::GGK_KP_ENTER;

                if (nontext)
                    return false;

                bool squelch =
                    m_edit_text.rows_m == 1 &&
                    0 < m_edit_text.max_cols_m &&
                    static_cast<std::size_t>(m_edit_text.max_cols_m) <
                    m_edit_text.control_m->Length() + 1;

                if (squelch)
                    return true;

                std::string translated_code_point;
                GG::GetTranslatedCodePoint(event.GetKey(), event.KeyCodePoint(),
                                           event.ModKeys(), translated_code_point);

                std::string new_value = m_edit_text.control_m->Text() + translated_code_point;

                if (m_edit_text.pre_edit_proc_m)
                    m_edit_text.pre_edit_proc_m(new_value, squelch);

                if (squelch) {
                    retval = true;
                } else {
                    *m_edit_text.control_m += translated_code_point;
                    m_edit_text.value_m = new_value;
                    m_edit_text.post_edit_proc_m(new_value);
                    retval = true;
                }
            }
            return retval;
        }

    edit_text_t& m_edit_text;
};

}

/*************************************************************************************************/

void edit_text_t::display(const model_type& value) // values that come in from Adam
{
    if (value != value_m)
        set_field_text(value_m = value);
}

/****************************************************************************************************/

edit_text_t::edit_text_t(const edit_text_ctor_block_t& block) : 
    name_m(block.name_m, block.alt_text_m, 0, GG::FORMAT_NONE, block.theme_m),
    alt_text_m(block.alt_text_m),
    field_text_m(),
    using_label_m(!block.name_m.empty()),
    rows_m(block.num_lines_m),
    cols_m(block.min_characters_m),
    max_cols_m(block.max_characters_m),
    scrollable_m(block.scrollable_m),
    password_m(block.password_m)
{ /* TODO: Address password_m == true. */ }

/****************************************************************************************************/

extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int rows);

void edit_text_t::measure(extents_t& result)
{
    assert(control_m);
    //
    // The calculate_edit_bounds function can figure out the size this edit box
    // should be, based on the number of rows and columns.
    //
    result = calculate_edit_bounds(control_m, cols_m, original_height_m);
    //
    // If we have a label then we need to make extra space
    // for it.
    //
    if (!using_label_m)
        return;
    const boost::shared_ptr<GG::Font>& font = implementation::DefaultFont();
    extents_t label_bounds(measure_text(name_m.name_m, font));
    label_bounds.vertical().guide_set_m.push_back(Value(font->Ascent()));
    //
    // Make sure that the height can accomodate both the label
    // and the edit widget.
    //
    align_slices(result.vertical(), label_bounds.vertical());
    //
    // We put the label on the left side of the edit box, and
    // place a point of interest at the end of the label, so
    // that colon alignment can be performed.
    //

    result.width() += gap + label_bounds.width();
    result.horizontal().guide_set_m.push_back(label_bounds.width());
}

/****************************************************************************************************/

void edit_text_t::place(const place_data_t& place_data)
{
    using adobe::place;

    assert(control_m);

    place_data_t local_place_data(place_data);

    if (using_label_m)
    {
        //
        // The vertical offset of the label is the geometry's
        // baseline - the label's baseline.
        //
        assert(place_data.vertical().guide_set_m.empty() == false);

        place_data_t label_place_data;
        label_place_data.horizontal().position_m = left(local_place_data);
        label_place_data.vertical().position_m = top(local_place_data);

        //
        // The width of the label is the first horizontal
        // point of interest.
        //
        assert(place_data.horizontal().guide_set_m.empty() == false);
        width(label_place_data) = place_data.horizontal().guide_set_m[0];
        height(label_place_data) = height(place_data);;

        //
        // Set the label dimensions.
        //
        place(get_label(), label_place_data);

        //
        // Now we need to adjust the position of the popup
        // widget.
        //
        long width = gap + adobe::width(label_place_data);
        local_place_data.horizontal().position_m += width;
        adobe::width(local_place_data) -= width;
    }

    implementation::set_control_bounds(control_m, local_place_data);
}

/****************************************************************************************************/

label_t& edit_text_t::get_label()
{ return name_m; }

/****************************************************************************************************/

void edit_text_t::enable(bool active)
{
    assert(control_m);

    control_m->Disable(!active);

    if (using_label_m) {
        using adobe::enable;
        enable(get_label(), active);
    }
}

/****************************************************************************************************/

void edit_text_t::set_theme(theme_t theme)
{ theme_m = theme; }


/****************************************************************************************************/

void edit_text_t::set_field_text(const std::string& text)
{
    assert(control_m);
    control_m->SetText(text);
}

/****************************************************************************************************/

void edit_text_t::signal_pre_edit(edit_text_pre_edit_proc_t proc)
{
    assert(control_m);

    if (!pre_edit_proc_m)
        pre_edit_proc_m = proc;
}

/****************************************************************************************************/

void edit_text_t::monitor(setter_type proc)
{
    if (!post_edit_proc_m)
        post_edit_proc_m = proc;
}

/****************************************************************************************************/
/// This function is used by the edit widget, as well as the popup widget
/// (which contains an edit widget).
///
/// \param  control the edit control to obtain the best bounds for.
/// \param  cols    the number of columns (or characters across) the edit control should contain
/// \param  rows    the number of rows of text to contain.
///
/// \return the best bounds for the edit control, including baseline.
//
extents_t calculate_edit_bounds(GG::Edit* edit, int cols, int original_height)
{
    extents_t result;

    assert(edit);
    assert(0 < cols);
    assert(0 < original_height);

    GG::Pt non_client_size = implementation::NonClientSize(*edit);

    result.width() = Value(cols * implementation::CharWidth() + non_client_size.x);
    result.height() = original_height;
    GG::Y baseline =
        (static_cast<int>(result.height()) - implementation::CharHeight()) / 2 +
        implementation::DefaultFont()->Ascent();
    result.vertical().guide_set_m.push_back(Value(baseline));

    return result;
}

/****************************************************************************************************/

template <>
platform_display_type insert<edit_text_t>(display_t&             display,
                                          platform_display_type& parent,
                                          edit_text_t&           element)
{
    if (element.using_label_m)
        insert(display, parent, element.get_label());

    assert(0 < element.rows_m);
    if (element.rows_m <= 1) {
        element.control_m =
            implementation::Factory().NewEdit(GG::X0, GG::Y0, GG::X1, "",
                                              implementation::DefaultFont(), GG::CLR_GRAY);
    } else {
        GG::Y height =
            implementation::CharHeight() * static_cast<int>(element.rows_m) +
            static_cast<int>(2 * GG::MultiEdit::BORDER_THICK);
        GG::Flags<GG::MultiEditStyle> style = GG::MULTI_LINEWRAP;
        if (!element.scrollable_m)
            style |= GG::MULTI_NO_SCROLL;
        element.control_m =
            implementation::Factory().NewMultiEdit(GG::X0, GG::Y0, GG::X1, height, "",
                                                   implementation::DefaultFont(), GG::CLR_GRAY, style);
    }

    element.original_height_m = Value(element.control_m->Height());

    element.filter_m.reset(new implementation::EditTextFilter(element));
    element.control_m->InstallEventFilter(element.filter_m.get());

    if (!element.alt_text_m.empty())
        implementation::set_control_alt_text(element.control_m, element.alt_text_m);

   return display.insert(parent, get_display(element));
}

/****************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
