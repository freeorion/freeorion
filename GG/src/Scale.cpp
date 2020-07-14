/* GG is a GUI for OpenGL.
   Copyright (C) 2020 Robert Ham <rah@settrans.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
*/

#include <GG/Base.h>
#include <GG/Scale.h>

using namespace GG;

static std::shared_ptr<Font> s_font;

void GG::SetScaleFont(std::shared_ptr<Font> font) {
    s_font = font;
}

static std::shared_ptr<Font> EnsureFont(std::shared_ptr<Font> font) {
    if (font) {
        return font;
    }

    assert(s_font);
    return s_font;
}

static bool NeedScaling(std::shared_ptr<Font> font) {
    std::shared_ptr<Font> ensured(EnsureFont(font));
    return ensured->PointSize() != 16;
}

static X_d ScaleFactorX(std::shared_ptr<Font> font) {
    /* Scale based on font's space width which scales linearly:

       pts 16, space width 4
       pts 24, space width 6
       pts 32, space width 8
       pts 40, space width 10

       The default font point size ("pts") is 16 and we scale based on
       that.
    */
    std::shared_ptr<Font> ensured(EnsureFont(font));
    return ensured->SpaceWidth() / static_cast<double>(4);
}

static Y_d ScaleFactorY(std::shared_ptr<Font> font) {
    /* Scale based on font's height which scales roughly linearly:

       pts 16, height 20
       pts 24, height 30
       pts 32, height 39
       pts 40, height 49
    */
    std::shared_ptr<Font> ensured(EnsureFont(font));
    return ensured->Height() / static_cast<double>(20);
}

static double DoScale(X_d x, std::shared_ptr<Font> font) {
    return Value(x * ScaleFactorX(font));
}

static double DoScale(Y_d y, std::shared_ptr<Font> font) {
    return Value(y * ScaleFactorY(font));
}

X::value_type GG::Scale(X x, std::shared_ptr<Font> font) {
    if (!NeedScaling(font)) {
        return Value(x);
    }

    return static_cast<X::value_type>(DoScale(X_d(Value(x)), font));
}

Y::value_type GG::Scale(Y y, std::shared_ptr<Font> font) {
    if (!NeedScaling(font)) {
        return Value(y);
    }

    return static_cast<Y::value_type>(DoScale(Y_d(Value(y)), font));
}

void GG::PushScale(std::shared_ptr<Font> font) {
    if (!NeedScaling(font)) {
        return;
    }

    glPushMatrix();
    glScalef(Value(ScaleFactorX(font)),
             Value(ScaleFactorY(font)),
             1.0f);
}

void GG::PopScale(std::shared_ptr<Font> font) {
    if (!NeedScaling(font)) {
        return;
    }

    glPopMatrix();
}
