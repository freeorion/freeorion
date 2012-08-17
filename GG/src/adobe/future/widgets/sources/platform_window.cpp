/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_window.hpp>

#include <GG/adobe/future/widgets/headers/display.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>
#include <GG/adobe/future/widgets/headers/platform_label.hpp>
#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/future/widgets/headers/platform_widget_utils.hpp>
#include <GG/adobe/keyboard.hpp>

#include <GG/DrawUtil.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>
#include <GG/TextControl.h>
#include <GG/Wnd.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

// TODO: Put this into StyleFactory.
class Window :
    public GG::Wnd
{
public:
    static const unsigned int BEVEL = 2;

    Window(adobe::window_t& imp) :
        Wnd(GG::X0, GG::Y0, GG::X1, GG::Y1, imp.flags_m),
        m_imp(imp),
        m_title(0)
        {
            if (!m_imp.name_m.empty()) {
                m_title = adobe::implementation::Factory().NewTextControl(
                    BEVEL_OFFSET.x, BEVEL_OFFSET.y - adobe::implementation::CharHeight(),
                    m_imp.name_m, adobe::implementation::DefaultFont()
                );
                AttachChild(m_title);
            }
        }

    virtual GG::Pt ClientUpperLeft() const
        { return UpperLeft() + BEVEL_OFFSET + GG::Pt(GG::X0, m_title ? m_title->Height() : GG::Y0); }

    virtual GG::Pt ClientLowerRight() const
        { return LowerRight() - BEVEL_OFFSET; }

    virtual GG::WndRegion WindowRegion(const GG::Pt& pt) const
        {
            enum {LEFT = 0, MIDDLE = 1, RIGHT = 2};
            enum {TOP = 0, BOTTOM = 2};

            // window regions look like this:
            // 0111112
            // 3444445   // 4 is client area, 0,2,6,8 are corners
            // 3444445
            // 6777778

            int x_pos = MIDDLE;   // default & typical case is that the mouse is over the (non-border) client area
            int y_pos = MIDDLE;

            GG::Pt ul = UpperLeft() + BEVEL_OFFSET, lr = LowerRight() - BEVEL_OFFSET;

            if (pt.x < ul.x)
                x_pos = LEFT;
            else if (pt.x > lr.x)
                x_pos = RIGHT;

            if (pt.y < ul.y)
                y_pos = TOP;
            else if (pt.y > lr.y)
                y_pos = BOTTOM;

            return (Resizable() ? GG::WndRegion(x_pos + 3 * y_pos) : GG::WR_NONE);
        }

    virtual void SizeMove(const GG::Pt& ul, const GG::Pt& lr)
        {
            GG::Wnd::SizeMove(ul, lr);

            GG::Pt client_size = ClientSize();

            if (!m_imp.debounce_m && !m_imp.resize_proc_m.empty()) {
                m_imp.debounce_m = true;

                if (adobe::width(m_imp.place_data_m) != Value(client_size.x) ||
                    adobe::height(m_imp.place_data_m) != Value(client_size.y)) {
                    m_imp.resize_proc_m(Value(client_size.x), Value(client_size.y));

                    adobe::width(m_imp.place_data_m) = Value(client_size.x);
                    adobe::height(m_imp.place_data_m) = Value(client_size.y);
                }

                m_imp.debounce_m = false;
            }

            GG::Pt new_title_size((LowerRight() - UpperLeft()).x - BEVEL_OFFSET.x * 2, m_title->Height());
            m_title->Resize(new_title_size);
        }

    virtual void Render()
        { GG::BeveledRectangle(UpperLeft(), LowerRight(), GG::CLR_GRAY, GG::CLR_GRAY, true, BEVEL); }

    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point,
                          GG::Flags<GG::ModKey> mod_keys)
        {
            adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                              true,
                                              adobe::modifier_state(),
                                              adobe::any_regular_t(this));
        }

    virtual void KeyRelease(GG::Key key, boost::uint32_t key_code_point,
                            GG::Flags<GG::ModKey> mod_keys)
        {
            adobe::keyboard_t::get().dispatch(adobe::key_type(key, key_code_point),
                                              false,
                                              adobe::modifier_state(),
                                              adobe::any_regular_t(this));
        }

private:
    adobe::window_t& m_imp;
    GG::TextControl* m_title;

    static const int FRAME_WIDTH;
    static const GG::Pt BEVEL_OFFSET;
};

const int Window::FRAME_WIDTH = 2;
const GG::Pt Window::BEVEL_OFFSET(GG::X(Window::FRAME_WIDTH), GG::Y(Window::FRAME_WIDTH));

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

window_t::window_t(const std::string&     name,
                   GG::Flags<GG::WndFlag> flags,
                   theme_t                theme) :
    window_m(0),
    flags_m(flags),
    name_m(name),
    theme_m(theme),
    debounce_m(false),
    placed_once_m(false)
{ }

/****************************************************************************************************/

window_t::~window_t()
{ delete window_m; }

/****************************************************************************************************/

void window_t::measure(extents_t& result)
{
    assert(window_m);

    if (name_m.empty())
    {
        result.height() = 15;
        result.width() = 15;

        return;
    }

    // REVISIT (fbrereto) : A lot of static metrics values added here

    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    result = metrics::measure_text(name_m, style->DefaultFont());

    result.width() = static_cast<long>(result.width() * 1.5);
}

/****************************************************************************************************/

void window_t::place(const place_data_t& place_data)
{
    assert(window_m);

    if (placed_once_m)
    {
        set_size(point_2d_t(width(place_data), height(place_data)));
    }
    else
    {
        placed_once_m = true;

        place_data_m = place_data;

        GG::Pt ul(GG::X(left(place_data)), GG::Y(top(place_data)));
        GG::Pt size(
            GG::Pt(GG::X(width(place_data)), GG::Y(height(place_data))) +
            implementation::NonClientSize(*window_m)
        );

        window_m->SetMinSize(size);

        window_m->SizeMove(ul, ul + size);
    }
}

/****************************************************************************************************/

void window_t::set_size(const point_2d_t& size)
{
    assert(window_m);

    if (debounce_m)
        return;

    debounce_m = true;

    width(place_data_m) = size.x_m;
    height(place_data_m) = size.y_m;

    window_m->Resize(
        GG::Pt(GG::X(width(place_data_m)), GG::Y(height(place_data_m))) +
        implementation::NonClientSize(*window_m)
    );

    debounce_m = false;
}

/****************************************************************************************************/

void window_t::reposition()
{
    assert(window_m);

    const GG::X width(window_m->Width());
    const GG::Y height(window_m->Height());

    GG::X app_width(GG::GUI::GetGUI()->AppWidth());
    GG::Y app_height(GG::GUI::GetGUI()->AppHeight());
    
    GG::X left(std::max(GG::X(10), (app_width - width) / 2));
    GG::Y top(std::max(GG::Y(10), (app_height - height) / 2));

    window_m->MoveTo(GG::Pt(left, top));
}

/****************************************************************************************************/

void window_t::set_visible(bool make_visible)
{
    assert(window_m);

    if (!window_m->Visible())
        reposition();

    set_control_visible(window_m, make_visible);
}

/****************************************************************************************************/

void window_t::monitor_resize(const window_resize_proc_t& proc)
{ resize_proc_m = proc; }

/****************************************************************************************************/

any_regular_t window_t::underlying_handler()
{ return any_regular_t(window_m); }

/****************************************************************************************************/

bool window_t::handle_key(key_type key, bool pressed, modifiers_t modifiers)
{ return false; }

/****************************************************************************************************/

template <>
platform_display_type insert<window_t>(display_t&             display,
                                       platform_display_type& parent,
                                       window_t&              element)
{
    assert(!element.window_m);
    assert(!parent);

    element.window_m = new Window(element);

    return display.insert(parent, element.window_m);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
