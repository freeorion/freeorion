/* GG is a GUI for SDL and OpenGL.

   Copyright (C) 2015 Mitten-O

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

   Zach Laine
   whatwasthataddress@gmail.com */

#ifndef TEXTBLOCK_H
#define TEXTBLOCK_H

#include <GG/TextControl.h>
#include <GG/RichText/BlockControl.h>

namespace GG
{

/**
 * @brief A text control embeddable in RichText.
*/
class TextBlock : public GG::BlockControl
{
public:
    //! Create a new TextBlock.
    TextBlock(X x, Y y, X w, const std::string& str, const std::shared_ptr<Font>& font, Clr color,
              Flags<TextFormat> format, Flags<WndFlag> flags);

    void CompleteConstruction() override;
    void Render() override
    {};

    //! Calculate the size based on the width it should take.
    Pt SetMaxWidth(X width) override;

private:
    std::shared_ptr<TextControl> m_text; //! The text control used to handle the text.
};

}

#endif // TEXTBLOCK_H
