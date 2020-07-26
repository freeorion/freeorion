//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2015 Mitten-O
//!  Copyright (C) 2016-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

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


#endif
