//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2007 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Cursor.h>
#include <GG/Texture.h>


using namespace GG;

namespace {
    constexpr bool OUTLINE_CURSOR = false;
}

TextureCursor::TextureCursor(std::shared_ptr<Texture> texture, Pt hotspot) :
    m_texture(std::move(texture)),
    m_hotspot(hotspot)
{
    m_hotspot.x = std::max(X0, std::min(m_hotspot.x, m_texture->DefaultWidth() - 1));
    m_hotspot.y = std::max(Y0, std::min(m_hotspot.y, m_texture->DefaultHeight() - 1));
}

void TextureCursor::Render(Pt pt) const
{
    assert(m_texture);
    Pt ul = pt - m_hotspot;
    if constexpr (OUTLINE_CURSOR) {
        Pt lr = ul + Pt(m_texture->DefaultWidth(), m_texture->DefaultHeight());
        int verts[8] = {
            Value(lr.x), Value(ul.y),
            Value(ul.x), Value(ul.y),
            Value(ul.x), Value(lr.y),
            Value(lr.x), Value(lr.y)
        };
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_INT, 0, verts);

        glDisable(GL_TEXTURE_2D);
        glColor3ub(255, 0, 0);

        glDrawArrays(GL_LINE_LOOP, 0, 4);

        glEnable(GL_TEXTURE_2D);

        glPopClientAttrib();
    }
    glColor4ub(255, 255, 255, 255);
    m_texture->OrthoBlit(ul);
}
