//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Menu.h
//!
//! Contains the MenuItem class, which represents menu data and the PopupMenu
//! class, which is used to provide immediate context menus.

#ifndef _GG_Menu_h_
#define _GG_Menu_h_


#include <functional>
#include <GG/ClrConstants.h>
#include <GG/Control.h>


namespace GG {

class Font;
class TextControl;

/** \brief Serves as a single menu entry in a GG::PopupMenu.

    May include a submenu.

    MenuItems have a selected_on_close callback which is called from
    PopUpMenus if this MenuItem is selected when the menu closes.
*/
struct GG_API MenuItem
{
    MenuItem() = default;

    explicit MenuItem(bool separator) :
        disabled(true),
        separator(true)
    {}

    template <class S>
    MenuItem(S&& str, bool disable, bool check,
             std::function<void()> selected_on_close_callback = std::function<void()>()) :
        label(std::forward<S>(str)),
        disabled(disable),
        checked(check),
        m_selected_on_close_callback{std::move(selected_on_close_callback)}
    {}

    std::string           label;            ///< text shown for this menu item
    bool                  disabled = false; ///< set to true when this menu item is disabled
    bool                  checked = false;  ///< set to true when this menu item can be toggled, and is currently on
    bool                  separator = false;///< set to true to render this menu item as a separator bar, rather than showing its text
    std::vector<MenuItem> next_level;       ///< submenu off of this menu item; may be emtpy

    /** A callback to be called if this menu item is selected on close.*/
    std::function<void()> m_selected_on_close_callback;
};


/** \brief A modal pop-up menu.

    PopupMenu gives calling code the ability to create a pop-up menu
    (usually in response to a right mouse click).  MenuItems are added with
    AddMenuItem(menuitem, callback).  When PopupMenu is Run(), on
    completion the callback of the selected MenuItem will be called.  */
class GG_API PopupMenu : public Wnd
{
public:
    /** Ctor.  Parameter \a m should contain the desired menu in its
        next_level member. */
    PopupMenu(X x, Y y, std::shared_ptr<Font> font,
              Clr text_color = CLR_WHITE, Clr border_color = CLR_BLACK,
              Clr interior_color = CLR_SHADOW, Clr hilite_color = CLR_GRAY);

    Pt ClientUpperLeft() const noexcept override { return m_origin; }

    Clr BorderColor() const noexcept { return m_border_color; }         ///< returns the color used to render the border of the control
    Clr InteriorColor() const noexcept { return m_int_color; }          ///< returns the color used to render the interior of the control
    Clr TextColor() const noexcept { return m_text_color; }             ///< returns the color used to render menu item text
    Clr HiliteColor() const noexcept { return m_hilite_color; }         ///< returns the color used to indicate a hilited menu item

    /** Add \p menu_item to the end of the popup menu and store its callback.*/
    void AddMenuItem(MenuItem&& menu_item);
    void AddMenuItem(std::string str, bool disable, bool check,
                     std::function<void()> selected_on_close_callback = std::function<void()>());

    void Render() override;
    void LButtonUp(Pt pt, Flags<ModKey> mod_keys) override;
    void LClick(Pt pt, Flags<ModKey> mod_keys) override;
    void LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys) override;
    void RButtonUp(Pt pt, Flags<ModKey> mod_keys) override;
    void RClick(Pt pt, Flags<ModKey> mod_keys) override;
    void MouseHere(Pt pt, Flags<ModKey> mod_keys) override;

    bool Run() override;

    void SetBorderColor(Clr clr);       ///< sets the color used to render the border of the control
    void SetInteriorColor(Clr clr);     ///< sets the color used to render the interior of the control
    void SetTextColor(Clr clr);         ///< sets the color used to render menu item text
    void SetHiliteColor(Clr clr);       ///< sets the color used to indicate a hilited menu item

    static constexpr std::size_t INVALID_CARET = std::numeric_limits<std::size_t>::max();

protected:
    /** Returns the font used to render text in the control. */
    const std::shared_ptr<Font>& GetFont() const;

    const MenuItem&                 MenuData() const;     ///< returns a const reference to the MenuItem that contains all the menu contents
    const std::vector<Rect>&        OpenLevels() const;   ///< returns the bounding rectangles for each open submenu, used to detect clicks in them
    const std::vector<std::size_t>& Caret() const;        ///< returns the stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push
    const MenuItem*                 ItemSelected() const; ///< returns the menu item selected (0 if none)

private:
    /** The font used to render the text in the control. */
    std::shared_ptr<Font> m_font;

    Clr               m_border_color;   ///< the color of the menu's border
    Clr               m_int_color;      ///< color painted into the client area of the control
    Clr               m_text_color;     ///< color used to paint text in control
    Clr               m_hilite_color;   ///< color behind selected items

    MenuItem          m_menu_data;      ///< this is not just a single menu item; the next_level element represents the entire menu

    std::vector<Rect>           m_open_levels;      ///< bounding rectangles for each open submenu, used to detect clicks in them
    std::vector<std::size_t>    m_caret;            ///< stack representing the caret's location's path (eg 0th subitem of 1st subitem of item 3) back() is the most recent push

    const Pt          m_origin;                     ///< the upper left hand corner of the control's visible area
    MenuItem*         m_item_selected = nullptr;    ///< the menu item selected (0 if none)
};

}


#endif
