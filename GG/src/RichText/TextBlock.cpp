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

#include "TextBlock.h"

#include <GG/RichText/RichText.h>

#include <limits.h>

namespace GG {

    TextBlock::TextBlock(X x, Y y, X w, const std::string& str,
                         const std::shared_ptr<Font>& font,
                         Clr color, Flags<TextFormat> format,
                         Flags<WndFlag> flags) :
        BlockControl(x, y, w, flags)
    {
        // Construct the text control. Activate full text wrapping features,
        // and make it stick to the top.
        // With these setting the height is largely ignored, so we set it to one.
        m_text = Wnd::Create<TextControl>(GG::X0, GG::Y0, w, Y1, str, font, color,
                                          format | FORMAT_WORDBREAK | FORMAT_LINEWRAP | FORMAT_TOP, flags);
    }

    void TextBlock::CompleteConstruction()
    {
        BlockControl::CompleteConstruction();
        AttachChild(m_text);
    }

    Pt TextBlock::SetMaxWidth(X width)
    {
        // Reflow the text to the given width. Height is ignored.
        m_text->Resize(GG::Pt(width, Y0));

        // Use the size the text requires.
        Pt text_size = m_text->MinUsableSize();
        Resize(text_size);
        return text_size;
    }

    // A factory for creating text blocks from tags.
    class TextBlockFactory: public RichText::IBlockControlFactory {
    public:
        //! Create a Text block from a plain text tag.
        std::shared_ptr<BlockControl> CreateFromTag(const std::string& tag,
                                                    const RichText::TAG_PARAMS& params,
                                                    const std::string& content,
                                                    const std::shared_ptr<Font>& font,
                                                    const Clr& color,
                                                    Flags<TextFormat> format) override
        {
            return Wnd::Create<TextBlock>(X0, Y0, X1, content, font, color, format, NO_WND_FLAGS);
        }
    };

    // Register text block as the default plaintext handler.
    static int dummy =
        RichText::RegisterDefaultBlock(RichText::PLAINTEXT_TAG, std::make_shared<TextBlockFactory>());
}
