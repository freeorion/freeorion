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

/** \file RichText.h \brief Contains the RichText class that allows you
 to intersperse text with images.*/

#ifndef RICHTEXT_H
#define RICHTEXT_H

#include <GG/ClrConstants.h>
#include <GG/TextControl.h>

#include <GG/RichText/BlockControl.h>

namespace GG
{
class RichTextPrivate;

/** \brief A control for showing text and images.
 */
class GG_API RichText: public Control
{
public:
    typedef std::map<std::string, std::string> TAG_PARAMS;

    //! An interface for object that create block controls from tags.
    class IBlockControlFactory
    {
    public:
        //! Creates a control from the tag (with unparsed parameters) and the content between the tags.
        //! You own the returned control.
        virtual std::shared_ptr<BlockControl> CreateFromTag(const std::string& tag,
                                                            const TAG_PARAMS& params,
                                                            const std::string& content,
                                                            const std::shared_ptr<Font>& font,
                                                            const Clr& color,
                                                            Flags<TextFormat> format) = 0;
    };

    //! The type of the object where we store control factories of tags.
    typedef std::map<std::string, std::shared_ptr<IBlockControlFactory>> BLOCK_FACTORY_MAP;

    //! The special tag that is used to represent plaintext.
    // Allows you to register a custom control for displaying plaintext.
    static const std::string PLAINTEXT_TAG;

    /** \name Structors */ ///@{
    RichText(X x, Y y, X w, Y h, const std::string& str, const std::shared_ptr<Font>& font,
             Clr color = CLR_BLACK, Flags<TextFormat> format = FORMAT_NONE,
             Flags<WndFlag> flags = NO_WND_FLAGS);

    void CompleteConstruction() override;

    ~RichText();
    //@}

    /** Set the text content. */
    virtual void SetText(const std::string& str);

    /**
     * @brief Set the whitespace around the content.
     *
     * @param pixels The number of pixels to reserve for empty space around the content.
     * @return void
     */
    void SetPadding(int pixels);

    void Render() override;

    void SizeMove(const Pt& ul, const Pt& lr) override;

    //! Use this to customize the handling of tags in the text on a per-object basis.
    void SetBlockFactoryMap(const std::shared_ptr<BLOCK_FACTORY_MAP>& block_factory_map);

    //! Registers a factory in the default block factory map.
    static int RegisterDefaultBlock(const std::string& tag, std::shared_ptr<IBlockControlFactory>&& factory);

    //! Access the default block factory map.
    static std::shared_ptr<RichText::BLOCK_FACTORY_MAP>& DefaultBlockFactoryMap();

private:
    friend class RichTextPrivate;
    std::unique_ptr<RichTextPrivate> const m_self;
};

}

#endif // RICHTEXT_H
