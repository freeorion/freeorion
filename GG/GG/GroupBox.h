//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/GroupBox.h
//!
//! Contains the GroupBox class, a simple, noninteractive box with an optional
//! label.

#ifndef _GG_GroupBox_h_
#define _GG_GroupBox_h_


#include <GG/ClrConstants.h>
#include <GG/Wnd.h>


namespace GG {

class Font;
class TextControl;

/** \brief This is a noninteractive box with an optional label. */
class GG_API GroupBox : public Wnd
{
public:
    /** Height is determined from the font and point size used. */
    GroupBox(X x, Y y, X w, Y h, const std::string& label, const std::shared_ptr<Font>& font, Clr color,
             Clr text_color = CLR_BLACK, Clr interior = CLR_ZERO, Flags<WndFlag> flags = NO_WND_FLAGS);
    void CompleteConstruction() override;

    Pt ClientUpperLeft() const override;
    Pt ClientLowerRight() const override;

    void Render() override;

    /** Sets the color of the group box. */
    void SetColor(Clr c);

    /** Sets the color of the label's text. */
    void SetTextColor(Clr c);

    /** Sets the interior color of the box. */
    void SetInteriorColor(Clr c);

    /** Setting this to true causes ClientUpperLeft() and ClientLowerRight()
        to return UpperLeft() and LowerRight(), respectively.  This is a
        horrible hack that is designed to allow the Eve layout engine to set
        this box's children at their desired locations without knowing
        anything about how GG Wnd client areas work. */
    void SetClientCornersEqualToBoxCorners(bool b);

    virtual void SetText(const std::string& str);

protected:
    GroupBox();

    /** The thickness with which to render the frame. */
    static const int FRAME_THICK;

    /** The number of pixels to leave between the client area and the
        frame. */
    static const int PIXEL_MARGIN;

private:
    Clr                             m_color;      ///< Color of box frame
    Clr                             m_text_color; ///< Color of label text
    Clr                             m_int_color;  ///< Color of background inside box
    std::shared_ptr<Font>           m_font;
    std::shared_ptr<TextControl>    m_label;
    bool                            m_set_client_corners_equal_to_box_corners = false;
};

}


#endif
