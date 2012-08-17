/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/future/widgets/headers/platform_metrics.hpp>
#include <GG/adobe/name.hpp>

#include <boost/cstdint.hpp>

#include <vector>

#include <GG/GUI.h>
#include <GG/Menu.h>
#include <GG/MultiEdit.h>
#include <GG/StyleFactory.h>
#include <GG/dialogs/FileDlg.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

boost::filesystem::path to_path(const std::string& path_string)
{ return boost::filesystem::path(path_string); }

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

GG::StyleFactory& Factory()
{ return *GG::GUI::GetGUI()->GetStyleFactory(); }

boost::shared_ptr<GG::Font> DefaultFont()
{ return Factory().DefaultFont(); }

GG::X CharWidth()
{
    static GG::X retval = DefaultFont()->TextExtent("0").x;
    return retval;
}

GG::Y CharHeight()
{ return DefaultFont()->Lineskip(); }

GG::Y StandardHeight()
{ return CharHeight() * 3 / 2; }

GG::Pt NonClientSize(GG::Wnd& w)
{ return w.Size() - w.ClientSize(); }

/****************************************************************************************************/

bool pick_file(boost::filesystem::path& path)
{
    boost::shared_ptr<GG::StyleFactory> style(GG::GUI::GetGUI()->GetStyleFactory());
    assert(style->DefaultFont());

    std::auto_ptr<GG::FileDlg> dlg(
        style->NewFileDlg("", "", false, false,
                          style->DefaultFont(), GG::CLR_GRAY, GG::CLR_GRAY));
    dlg->Run();
    assert(dlg->Result().size() <= 1u);

    bool result(!dlg->Result().empty());

    if (result)
        path = to_path(*dlg->Result().begin());

    return result;
}

/****************************************************************************************************/

bool pick_save_path(boost::filesystem::path& path)
{
    boost::shared_ptr<GG::StyleFactory> style(GG::GUI::GetGUI()->GetStyleFactory());
    assert(style->DefaultFont());

    std::auto_ptr<GG::FileDlg> dlg(
        style->NewFileDlg("", "", true, false,
                          style->DefaultFont(), GG::CLR_GRAY, GG::CLR_GRAY));
    dlg->SelectDirectories(true);
    dlg->Run();
    assert(dlg->Result().size() <= 1u);

    bool result(!dlg->Result().empty());

    if (result)
        path = to_path(*dlg->Result().begin());

    return result;
}

/****************************************************************************************************/

void set_control_alt_text(GG::Wnd* control, const std::string& alt_text)
{ control->SetBrowseText(alt_text); }

/****************************************************************************************************/

void get_control_bounds(GG::Wnd* control, GG::Rect& bounds)
{
    assert(control);
    bounds = GG::Rect(control->UpperLeft(), control->LowerRight());
}

/****************************************************************************************************/

void set_control_bounds(GG::Wnd* control, const place_data_t& place_data)
{
    if (control) {
        GG::Pt ul(GG::X(left(place_data)), GG::Y(top(place_data)));
        control->SizeMove(ul,
                          ul + GG::Pt(GG::X(width(place_data)),
                                      GG::Y(height(place_data))));
    }
}

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::Edit*>(GG::Edit*& x)
{ return x->Text(); }

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::MultiEdit*>(GG::MultiEdit*& x)
{ return x->Text(); }

/****************************************************************************************************/

template <>
const std::string& get_field_text<GG::TextControl*>(GG::TextControl*& x)
{ return x->Text(); }

/****************************************************************************************************/

bool is_focused(GG::Wnd* w)
{ return GG::GUI::GetGUI()->FocusWnd() == w; }

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void set_control_visible(GG::Wnd* control, bool make_visible)
{
    if (make_visible)
        control->Show();
    else
        control->Hide();
}

/****************************************************************************************************/

bool context_menu(const GG::Pt& pt,
                  const name_t* first,
                  const name_t* last,
                  name_t& result)
{
    boost::shared_ptr<GG::StyleFactory> style(GG::GUI::GetGUI()->GetStyleFactory());
    assert(style->DefaultFont());

    int id = 1;
    GG::MenuItem items;
    for (const name_t* it = first; it != last; ++it, ++id) {
        items.next_level.push_back(
            GG::MenuItem(static_cast<std::string>(it->c_str()), id, false, false));
    }

    GG::PopupMenu popup(pt.x, pt.y, style->DefaultFont(), items);
    int result_id = popup.Run();

    if (result_id)
        result = *(first + result_id - 1);

    return result_id;
}

/****************************************************************************************************/

modifiers_t convert_modifiers(GG::Flags<GG::ModKey> mods)
{
    modifiers_t result(modifiers_none_s);

#define MAP_GG_TO_ADOBE_MOD(gg_mod, adobe_mod) if (mods & gg_mod) result |= adobe_mod

    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LSHIFT, modifiers_left_shift_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RSHIFT, modifiers_right_shift_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LALT, modifiers_left_option_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RALT, modifiers_right_option_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_LCTRL, modifiers_left_control_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_RCTRL, modifiers_right_control_s);
    MAP_GG_TO_ADOBE_MOD(GG::MOD_KEY_CAPS, modifiers_caps_lock_s);

#undef MAP_GG_TO_ADOBE_MOD

    return result;
}

/****************************************************************************************************/

modifiers_t modifier_state()
{ return convert_modifiers(GG::GUI::GetGUI()->ModKeys()); }

/****************************************************************************************************/

platform_display_type get_top_level_window(platform_display_type thing)
{ return thing->RootParent(); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
