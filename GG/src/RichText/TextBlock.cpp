//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <limits.h>
#include <GG/RichText/RichText.h>
#include "TextBlock.h"


using namespace GG;

TextBlock::TextBlock(X x, Y y, X w, std::string str,
                     std::shared_ptr<Font> font,
                     Clr color, Flags<TextFormat> format,
                     Flags<WndFlag> flags) :
    BlockControl(x, y, w, flags)
{
    SetName("TextBlock: " + str.substr(0, 16));

    // Construct the text control. Activate full text wrapping features,
    // and make it stick to the top.
    // With these setting the height is largely ignored, so we set it to one.
    m_text = Wnd::Create<TextControl>(GG::X0, GG::Y0, w, Y1, std::move(str), std::move(font), color,
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
    std::shared_ptr<BlockControl> CreateFromTag(const RichText::TAG_PARAMS&,
                                                std::string content,
                                                std::shared_ptr<Font> font,
                                                Clr color,
                                                Flags<TextFormat> format) const override
    {
        return Wnd::Create<TextBlock>(X0, Y0, X1, std::move(content), std::move(font),
                                      color, format, NO_WND_FLAGS);
    }
};

namespace {
    // Register text block as the default plaintext handler.
    const auto dummy = RichText::RegisterDefaultBlock(RichText::PLAINTEXT_TAG,
                                                      std::make_shared<TextBlockFactory>());
}
