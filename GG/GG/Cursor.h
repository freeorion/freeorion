//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2007 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Cursor.h
//!
//! Contains Cursor class, which encapsulates the rendering of the input
//! cursor.

#ifndef _GG_Cursor_h_
#define _GG_Cursor_h_


#include <memory>
#include <GG/PtRect.h>


namespace GG {

class Texture;

/** \brief Cursor is the base class for GUI-renderable cursors.

    A Cursor can be set in the GUI and will be rendered if GUI's
    RenderCursor() member returns true.  Note that it may be necessary to
    disable the underlying platform's cursor . */
class GG_API Cursor
{
public:
#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201907L)
    constexpr Cursor() = default;
    constexpr virtual ~Cursor() = default;
#else
    Cursor() = default;
    virtual ~Cursor() = default;
#endif

    /** Renders the cursor at the specified location.  Subclasses should take
        care to ensure that the cursor's "hotspot" is rendered at \a pt. */
    virtual void Render(Pt pt) const {};
};

/** \brief TextureCursor is a very simple subclass of Cursor.

    It renders a texture such that the point within the texture that
    represents the hotspot of the cursor is rendered at the click-point of the
    cursor. */
class GG_API TextureCursor final : public Cursor
{
public:
    /** Ctor.  \a texture is the texture to render and \a hotspot is the
        offset within \a texture where the click-point is located.  \a hotspot
        is clamped to \a texture's valid area. */
    TextureCursor(std::shared_ptr<Texture> texture, Pt hotspot = Pt());

    /** Returns the texture used to render this TextureCursor. */
    [[nodiscard]] const auto& GetTexture() const noexcept { return m_texture; }

    /** Returns the position within Texture() of the cursor hotspot. */
    [[nodiscard]] Pt Hotspot() const noexcept { return m_hotspot; }

    void Render(Pt pt) const override;

private:
    std::shared_ptr<Texture> m_texture;
    Pt                       m_hotspot;
};

}


#endif
