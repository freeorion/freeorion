//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/BrowseInfoWnd.h
//!
//! Contains the BrowseInfoWnd class, and its subclass TextBoxBrowseInfoWnd.
//! These classes display info on another window.

#ifndef _GG_BrowseInfoWnd_h_
#define _GG_BrowseInfoWnd_h_


#include <GG/Font.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/Wnd.h>


namespace GG {

class Cursor;
class Font;
class TextControl;

/** \brief The abstract base class for all browse-info display windows.

    Each frame that a BrowseInfoWnd is displayed, its Update() method is
    called.  The Wnd* parameter passed in this call is the window about which
    the BrowseInfoWnd is displaying info (the target Wnd); the BrowseInfoWnd
    can collect whatever information it requires from the target Wnd before it
    is rendered.  Note that a BrowseInfoWnd should never be INTERACTIVE. */
class GG_API BrowseInfoWnd : public Wnd
{
public:
    /** Returns true iff \a wnd's browse mode \a mode contains browse info
        that is usable by this BrowseInfoWnd.  This method is used by GUI to
        determine whether a Wnd w has suitable browse info available; if not,
        w's ancestors may instead be asked for browse info recursively. */
    virtual bool WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const = 0;

    /** Collects data from \a target that is needed by Render().  Note that
        the one datum that is always available for any Wnd is the text to
        display for \a mode, accessible through Wnd::BrowseInfoText() (though
        this may be the empty string).  Other data that are provided by a Wnd
        subclass can be recovered by casting \a target to its actual subclass
        type. */
    void Update(std::size_t mode, const Wnd* target);

    /** Sets the current cursor position to the one given. */
    void SetCursorPosition(Pt cursor_pos) noexcept { m_cursor_pos = cursor_pos; }

    /** If set by the user, this function is used to determine the position of
        this BrowseInfoWnd.  It takes the current cursor position, the
        currently-set cursor, this BrowseInfoWnd, and the Wnd that is
        displaying this BrowseInfoWnd as parameters, and returns the desired
        upper-left corner of this BrowseInfoWnd. */
    mutable std::function<Pt (Pt, const Cursor&, const BrowseInfoWnd&, const Wnd&)> m_position_wnd_fn;

protected:
    BrowseInfoWnd(X x, Y y, X w, Y h) :
        Wnd(x, y, w, h)
    {}

private:
    Pt m_cursor_pos;

    virtual void UpdateImpl(std::size_t mode, const Wnd* target) {}
};


/** \brief A subclass of BrowseInfoWnd that displays text in a box, optionally
    with a border.

    The typical use case is for TextBoxBrowseInfoWnd to get the appropriate
    text for the current mode by calling BrowseInfoText() on its target Wnd.
    It may also be used to display static text, by setting
    SetTextFromTarget(false) and setting the desired text with SetText(). */
class GG_API TextBoxBrowseInfoWnd : public BrowseInfoWnd
{
public:
    TextBoxBrowseInfoWnd(X w, const std::shared_ptr<Font>& font, Clr color, Clr border_color, Clr text_color,
                         Flags<TextFormat> format = FORMAT_LEFT | FORMAT_WORDBREAK,
                         unsigned int border_width = 2, unsigned int text_margin = 4);
    void CompleteConstruction() override;

    [[nodiscard]] bool                         WndHasBrowseInfo(const Wnd* wnd, std::size_t mode) const override;

    [[nodiscard]] bool                         TextFromTarget() const noexcept { return m_text_from_target; } ///< true iff the text to display will be read from the target wnd
    [[nodiscard]] const std::string&           Text() const noexcept; ///< text currently set for display

    /** Returns the Font used to display text. */
    [[nodiscard]] const auto&                  GetFont() const noexcept { return m_font; }

    [[nodiscard]] Clr                          Color() const noexcept { return m_color; }               ///< color used to render the text box
    [[nodiscard]] Clr                          BorderColor() const noexcept { return m_border_color; }  ///< color used to render the text box border
    [[nodiscard]] Clr                          TextColor() const noexcept;      ///< returns the color used to render the text
    [[nodiscard]] Flags<TextFormat>            GetTextFormat() const noexcept;  ///< returns the text format used to render the text
    [[nodiscard]] unsigned int                 BorderWidth() const noexcept { return m_border_width; }  ///< width of the text box border
    [[nodiscard]] unsigned int                 TextMargin() const noexcept;     ///< returns the margin to leave between the text and the text box

    void SetText(std::string str);
    void Render() override;
    void SizeMove(Pt ul, Pt lr) override;

    void SetTextFromTarget(bool b);                    ///< sets the text display mode to static (\a b == true) or dynamic (read from the target Wnd, \a b == false)

    /** Sets the Font used to display text. */
    void SetFont(std::shared_ptr<Font> font);

    void SetColor(Clr color);                          ///< sets the color used to render the text box
    void SetBorderColor(Clr border_color);             ///< sets the color used to render the text box border
    void SetTextColor(Clr text_color);                 ///< sets the color used to render the text
    void SetTextFormat(Flags<TextFormat> format);      ///< sets the text format used to render the text
    void SetBorderWidth(unsigned int border_width);    ///< sets the width of the text box border
    void SetTextMargin(unsigned int text_margin);      ///< sets the margin to leave between the text and the text box

private:
    virtual void InitBuffer();
    void UpdateImpl(std::size_t mode, const Wnd* target) override;

    GG::GL2DVertexBuffer            m_buffer;
    bool                            m_text_from_target;
    std::shared_ptr<Font>           m_font;
    Clr                             m_color;
    Clr                             m_border_color;
    unsigned int                    m_border_width;
    X                               m_preferred_width;
    std::shared_ptr<TextControl>    m_text_control;
    unsigned int                    m_text_margin;
};

}


#endif
